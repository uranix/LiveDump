#include "memory.h"

void bx_mem_c::prot_flags(Bit32u fl, char *buf)
	{
		char *r;
		switch (fl & 0xff)
		{
			case PAGE_EXECUTE: r = "--x"; break;
			case PAGE_EXECUTE_READ: r = "r-x"; break;
			case PAGE_EXECUTE_READWRITE: r = "rwx"; break;
			case PAGE_READONLY: r = "r--"; break;
			case PAGE_READWRITE: r = "rw-"; break;
			case PAGE_EXECUTE_WRITECOPY: r = "cwx"; break;
			case PAGE_NOACCESS: r = "---"; break;
			case PAGE_WRITECOPY: r = "cw-"; break;
			default: r = "***";
		}
		strcpy_s(buf, 8, r);
		if (fl & PAGE_GUARD)
			strcat_s(buf, 8, "G");
		if (fl & PAGE_NOCACHE)
			strcat_s(buf, 8, "C");
		if (fl & PAGE_WRITECOMBINE)
			strcat_s(buf, 8, "W");
	}

	bx_mem_c::bx_mem_c()
	{
		len = 0x80000000;
		memset(tlb, 0, sizeof(tlb));
	}
	void bx_mem_c::init_mem(PMINIDUMP_MEMORY64_LIST pMemory, PMEM_INFO pMemInfo, LPVOID DumpBase)
	{
		int memdata = 0;

		Bit8u *dump = ((Bit8u *)DumpBase)+pMemory->BaseRva;
		
		//printf("Memory info\n");
		for (int i=0;i<pMemInfo->header.NumberOfEntries;i++)
		{
			PMINIDUMP_MEMORY_INFO p = &pMemInfo->info[i];
			char oldprot[8];
			char newprot[8];
			prot_flags(p->Protect, newprot);
			prot_flags(p->AllocationProtect, oldprot);
			//printf("[%c] 0x%.8llx - 0x%.8llx @%-6s(%-6s)\n", p->State == MEM_COMMIT ? 'C' : p->State == MEM_FREE ? 'F' : 'R',
			//	p->BaseAddress, p->BaseAddress + p->RegionSize - 1, newprot, oldprot);
			if (p->State == MEM_COMMIT)
			{
				Bit32u sz = (Bit32u)(p->RegionSize >> 12);
				Bit8u *ptr = (Bit8u *)VirtualAlloc(0, (SIZE_T)p->RegionSize, MEM_COMMIT, PAGE_READWRITE);
				BX_ASSERT(ptr);
				if (!(p->Protect & PAGE_GUARD))
				{
					while (p->BaseAddress >= pMemory->MemoryRanges[memdata].StartOfMemoryRange + pMemory->MemoryRanges[memdata].DataSize)
					{
						dump += pMemory->MemoryRanges[memdata].DataSize;
						memdata++;
					}
					// p->BaseAddress < pMemory->MemoryRanges[memdata].StartOfMemoryRange + pMemory->MemoryRanges[memdata].DataSize
					// assuming p->BaseAddress >= pMemory->MemoryRanges[memdata].StartOfMemoryRange
					BX_ASSERT(p->BaseAddress >= pMemory->MemoryRanges[memdata].StartOfMemoryRange);
					memcpy(ptr, dump + pMemory->MemoryRanges[memdata].StartOfMemoryRange - p->BaseAddress, (size_t)p->RegionSize);
				}
				DWORD diems;
				VirtualProtect(ptr, (SIZE_T)p->RegionSize, p->Protect, &diems);
				Bit32u offs = (Bit32u)(p->BaseAddress >> 12);
				while (sz)
				{
					tlb[offs] = ptr;
					sz--;
					offs++;
					ptr += 0x1000;
				}
			}
		}
	}
	Bit8u *bx_mem_c::vector(Bit32u virt)
	{
		return (Bit8u *)((virt & 0xfff) | (Bit32u)tlb[virt >> 12]);
	}
	void bx_mem_c::read_physical(bx_cpu_c *cpu, Bit32u addr, unsigned len, void *data){
		Bit8u *src = vector(addr);
		memcpy(data, src, len);
	}
	void bx_mem_c::write_physical(bx_cpu_c *cpu, Bit32u addr, unsigned len, void *data){
		Bit8u *dst = vector(addr);
		memcpy(dst, data, len);
	}
	Bit32u bx_mem_c::alloc_pages(int num, DWORD prot)
	{
		int nfound = 0;
		// keep 0 page empty
		for (int i=1;i<(1<<19);i++)
		{
			if (tlb[i] == 0) 
				nfound ++;
			else
				nfound = 0;
			if (nfound == num)
			{
				Bit8u *ptr = (Bit8u *)VirtualAlloc(0, num << 12, MEM_COMMIT, prot);
				if (!ptr)
					return 0;
				for (int j=i-num+1;j <= i; j++, ptr+=0x1000 )
					tlb[j] = ptr;
				return (i-num+1) << 12;
			}
		}
		return 0;
	}
	Bit32u bx_mem_c::alloc_page(DWORD prot)
	{
		return alloc_pages(1, prot);
	}
