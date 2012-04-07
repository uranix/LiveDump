#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

#include "bochs.h"
#include "cpu/cpu.h"

#pragma comment (lib, "dbghelp")

#include <list>

std::pair<Bit32u,Bit64u> back_trace[0x10000];

void print_stack_trace() {
	Bit32u esp = bx_cpu.gen_reg[4].erx;
	for (int i=0; (i<64 && (((esp + i) & ~0xfff) == (esp & ~0xfff))); i+= 4)
		printf("[esp+%.2x] = 0x%.8x\n", i, *(Bit32u *)bx_mem.vector(esp+i));
}

void dump_regs() {
	printf(
		"eax = 0x%.8x\n"
		"ebx = 0x%.8x\n"
		"ecx = 0x%.8x\n"
		"edx = 0x%.8x\n"
		"esp = 0x%.8x\n"
		"ebp = 0x%.8x\n"
		"esi = 0x%.8x\n"
		"edi = 0x%.8x\n"
		"eip = 0x%.8x\n",
		
		bx_cpu.gen_reg[0].erx,
		bx_cpu.gen_reg[3].erx,
		bx_cpu.gen_reg[1].erx,
		bx_cpu.gen_reg[2].erx,
		bx_cpu.gen_reg[4].erx,
		bx_cpu.gen_reg[5].erx,
		bx_cpu.gen_reg[6].erx,
		bx_cpu.gen_reg[7].erx,
		bx_cpu.eip);
}

Bit32u ip;
Bit64u clk = 0;

void process_eip_bp(Bit32u eip) {
	back_trace[clk & 0xffff] = std::pair<Bit32u, Bit64u>(eip, clk);
	clk ++;
	ip = eip;
	if (eip == 0xbadadd)
		throw 0xbadadd;
}

void process_memory_bp(Bit32u addr, unsigned len, unsigned rw, void *data) {
}

extern const unsigned char cpack[];

int main()
{
	const char pFileName[] = "D:\\Tools\\procdump\\1_120315_161703.dmp"; 
	HANDLE hFile = CreateFileA( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Dump open failed\n");
		return 1;
	}
	HANDLE hMapFile = CreateFileMappingA( hFile, NULL, PAGE_READONLY, 0, 0, NULL ); 
	if( hMapFile == NULL ) 
	{
		printf("Error: CreateFileMapping failed. Error: %u \n", GetLastError() ); 
		return 2;  
	}
	LPVOID pMiniDump = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 ); 

	PMINIDUMP_DIRECTORY		pMiniDumpDir  = 0; 
	PMINIDUMP_MEMORY64_LIST	pMemory       = 0; 

	ULONG					StreamSize    = 0; 

	if( !MiniDumpReadDumpStream( pMiniDump, Memory64ListStream, &pMiniDumpDir, 
	                             (PVOID *)&pMemory, &StreamSize ) ) 
	{
		DWORD ErrCode = GetLastError(); 
		if( ErrCode != 0 ) // 0 -> no such stream in the dump 
			printf( "Error: MiniDumpReadDumpStream failed. Error: %u \n", ErrCode ); 
		else 
			printf( "Stream (id %u) not found in the minidump.\n", Memory64ListStream ); 
		return 4;
	}

	MEM_INFO *pMemInfo = 0;

	if( !MiniDumpReadDumpStream( pMiniDump, MemoryInfoListStream, &pMiniDumpDir, 
		(PVOID *)&pMemInfo, &StreamSize ) ) 
	{
		DWORD ErrCode = GetLastError(); 
		if( ErrCode != 0 ) // 0 -> no such stream in the dump 
			printf( "Error: MiniDumpReadDumpStream failed. Error: %u \n", ErrCode ); 
		else 
			printf( "Stream (id %u) not found in the minidump.\n", MemoryInfoListStream ); 
		return 5;
	}

	typedef struct {
		MINIDUMP_THREAD_INFO_LIST header;
		MINIDUMP_THREAD_INFO info[0];
	} THREAD_INFO, *PTHREAD_INFO;
	PTHREAD_INFO pThreadInfo;

	if( !MiniDumpReadDumpStream( pMiniDump, ThreadInfoListStream, &pMiniDumpDir, 
		(PVOID *)&pThreadInfo, &StreamSize ) ) 
	{
		DWORD ErrCode = GetLastError(); 
		if( ErrCode != 0 ) // 0 -> no such stream in the dump 
			printf( "Error: MiniDumpReadDumpStream failed. Error: %u \n", ErrCode ); 
		else 
			printf( "Stream (id %u) not found in the minidump.\n", MemoryInfoListStream ); 
		return 5;
	}

	PMINIDUMP_THREAD_LIST pThreads;

	if( !MiniDumpReadDumpStream( pMiniDump, ThreadListStream, &pMiniDumpDir, 
		(PVOID *)&pThreads, &StreamSize ) ) 
	{
		DWORD ErrCode = GetLastError(); 
		if( ErrCode != 0 ) // 0 -> no such stream in the dump 
			printf( "Error: MiniDumpReadDumpStream failed. Error: %u \n", ErrCode ); 
		else 
			printf( "Stream (id %u) not found in the minidump.\n", MemoryInfoListStream ); 
		return 5;
	}

	bx_cpu.reset(0);
	bx_cpu.init(BX_MEM(0));
	// have to initialize CS, SS, FS, DS
	// eip, esp
	// set protected mode
#define SET_SEG(_SEG, _BASE, _LIM) do { \
	bx_cpu.sregs[_SEG].cache.u.segment.limit_scaled = _LIM; \
	bx_cpu.sregs[_SEG].cache.u.segment.g = 0; \
	bx_cpu.sregs[_SEG].cache.u.segment.d_b = 1; \
	bx_cpu.sregs[_SEG].cache.u.segment.base = _BASE; } while (0)

	SET_SEG(BX_SEG_REG_CS, 0, 0xffffffff);
	SET_SEG(BX_SEG_REG_DS, 0, 0xffffffff);
	SET_SEG(BX_SEG_REG_SS, 0, 0xffffffff);
	SET_SEG(BX_SEG_REG_ES, 0, 0xffffffff);
	SET_SEG(BX_SEG_REG_GS, 0, 0xffffffff);
	SET_SEG(BX_SEG_REG_FS, (Bit32u)pThreads->Threads[0].Teb, 0xffffffff);

	bx_cpu.cr0.pe = 1;

	bx_cpu.parse_selector(0x23, &bx_cpu.sregs[BX_SEG_REG_CS].selector);
	bx_cpu.parse_selector(0x2B, &bx_cpu.sregs[BX_SEG_REG_DS].selector);
	bx_cpu.parse_selector(0x2B, &bx_cpu.sregs[BX_SEG_REG_ES].selector);
	bx_cpu.parse_selector(0x2B, &bx_cpu.sregs[BX_SEG_REG_SS].selector);
	bx_cpu.parse_selector(0x2B, &bx_cpu.sregs[BX_SEG_REG_GS].selector);
	bx_cpu.parse_selector(0x53, &bx_cpu.sregs[BX_SEG_REG_FS].selector);

	bx_mem.init_mem(pMemory, pMemInfo, pMiniDump);

	/* Dump loaded */

	/* Pick a thread */

	int j = -1;

	PCONTEXT pctx;

	for (unsigned i = 0; i < pThreads->NumberOfThreads; i++) {
		pctx = (PCONTEXT)((Bit8u *)pMiniDump + pThreads->Threads[i].ThreadContext.Rva);

		printf("Eip[%.4x] = 0x%.8x\n", i, pctx->Eip);
		if (pctx->Eip == 0x12345678) {
			j = i;
			break;
		}
	}

	if (j == -1)
		throw "not found";

	bx_cpu.eip = pctx->Eip;

	bx_cpu.gen_reg[0].erx = pctx->Eax;
	bx_cpu.gen_reg[1].erx = pctx->Ecx;
	bx_cpu.gen_reg[2].erx = pctx->Edx;
	bx_cpu.gen_reg[3].erx = pctx->Ebx;
	bx_cpu.gen_reg[4].erx = pctx->Esp;
	bx_cpu.gen_reg[5].erx = pctx->Ebp;
	bx_cpu.gen_reg[6].erx = pctx->Esi;
	bx_cpu.gen_reg[7].erx = pctx->Edi;

	Bit8u *patch = bx_mem.vector(bx_cpu.eip);

	/* Change 0xeb 0xfe back to original code */
	patch[0] = 0x90; 
	patch[1] = 0x90; 

	try {
		bx_cpu.cpu_loop(1);
	} catch (int) {

	}

	return 0;
}