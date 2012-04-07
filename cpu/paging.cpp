/////////////////////////////////////////////////////////////////////////
// $Id: paging.cc,v 1.8 2001/10/03 13:10:37 bdenney Exp $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001  MandrakeSoft S.A.
//
//    MandrakeSoft S.A.
//    43, rue d'Aboukir
//    75002 Paris - France
//    http://www.linux-mandrake.com/
//    http://www.mandrakesoft.com/
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA



#if 0
  // - what should the reserved bits in the error code be ?
  // - move CR0.wp bit in lookup table to cache.  Then dump
  //   cache whenever it is changed.  This eliminates the
  //   extra calculation and shifting.
  // - change BX_READ and BX_WRITE to 0,1 ???
#endif



#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#define LOG_THIS bx_cpu.

#if BX_USE_CPU_SMF
#define this (BX_CPU(0))
#endif

// stub functions for non-support of paging
  void
bx_cpu_c::enable_paging(void)
{
  BX_PANIC(("enable_paging(): not implemented"));
}

  void
bx_cpu_c::disable_paging(void)
{
  BX_PANIC(("disable_paging() called"));
}

  void
bx_cpu_c::CR3_change(Bit32u value32)
{
  BX_INFO(("CR3_change(): flush TLB cache"));
  BX_INFO(("Page Directory Base %08x", (unsigned) value32));
}

  void process_memory_bp(Bit32u addr, unsigned len, unsigned rw, void *data);

  void
	  bx_cpu_c::access_linear(Bit32u laddress, unsigned length, unsigned pl,
	  unsigned rw, void *data)
  {
	  /* perhaps put this check before all code which calls this function,
	  * so we don't have to here
	  */
	  /* access_linear(laddr, 4, CPL==3, BX_READ, (void *) data); */

	  if (bx_cpu. cr0.pg == 0) 
	  {
		  __try
		  {
			  if (rw == BX_READ)
				  bx_cpu. mem->read_physical(this, laddress, length, data);
			  else
				  bx_cpu. mem->write_physical(this, laddress, length, data);

			  process_memory_bp(laddress, length, rw, data);

			  return;
		  } __except (EXCEPTION_EXECUTE_HANDLER) {
			  exception(BX_PF_EXCEPTION, 0x5678, 0);
		  }
	  }

	  BX_PANIC(("access_linear: paging not supported"));
  }
  void
bx_cpu_c::INVLPG(BxInstruction_t* i)
{}
