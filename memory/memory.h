#pragma once

#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

#include "bochs.h"

#pragma warning (disable : 4200)
typedef struct { 
	MINIDUMP_MEMORY_INFO_LIST header;
	MINIDUMP_MEMORY_INFO info[0];
} MEM_INFO, *PMEM_INFO;

#if BX_SMP_PROCESSORS==1
extern BX_MEM_C    bx_mem;
#else
extern BX_MEM_C    *bx_mem_array[BX_ADDRESS_SPACES];
#endif  /* BX_SMP_PROCESSORS */

class bx_mem_c
{
private:
	void prot_flags(Bit32u fl, char *buf);
	Bit8u *tlb[(1 << 19)];
public:
	bx_mem_c();
	void init_mem(PMINIDUMP_MEMORY64_LIST pMemory, PMEM_INFO pMemInfo, LPVOID DumpBase);
	Bit32u len;
	Bit8u *vector(Bit32u virt);
	void read_physical(bx_cpu_c *cpu, Bit32u addr, unsigned len, void *data);
	void write_physical(bx_cpu_c *cpu, Bit32u addr, unsigned len, void *data);
	Bit32u alloc_pages(int num, DWORD prot);
	Bit32u alloc_page(DWORD prot);
};