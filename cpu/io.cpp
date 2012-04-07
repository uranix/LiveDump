/////////////////////////////////////////////////////////////////////////
// $Id: io.cc,v 1.5 2001/10/03 13:10:37 bdenney Exp $
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







#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#define LOG_THIS bx_cpu.





  void
bx_cpu_c::INSB_YbDX(BxInstruction_t *i)
{
  Bit8u value8=0;

  if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
    if ( !bx_cpu. allow_io(DX, 1) ) {
      exception(BX_GP_EXCEPTION, 0, 0);
      }
    }

  if (i->as_32) {
    // Write a zero to memory, to trigger any segment or page
    // faults before reading from IO port.
    write_virtual_byte(BX_SEG_REG_ES, EDI, &value8);

    BX_ASSERT(0=="IO")
;  //value8 = BX_INP(DX, 1);

    /* no seg override possible */
    write_virtual_byte(BX_SEG_REG_ES, EDI, &value8);

    if (bx_cpu. eflags.df) {
      EDI = EDI - 1;
      }
    else {
      EDI = EDI + 1;
      }
    }
  else {
    // Write a zero to memory, to trigger any segment or page
    // faults before reading from IO port.
    write_virtual_byte(BX_SEG_REG_ES, DI, &value8);

    BX_ASSERT(0=="IO");
	//value8 = BX_INP(DX, 1);

    /* no seg override possible */
    write_virtual_byte(BX_SEG_REG_ES, DI, &value8);

    if (bx_cpu. eflags.df) {
      DI = DI - 1;
      }
    else {
      DI = DI + 1;
      }
    }
}

  void
bx_cpu_c::INSW_YvDX(BxInstruction_t *i)
  // input word/doubleword from port to string
{
  Bit32u edi;
  unsigned int incr;

  if (i->as_32)
    edi = EDI;
  else
    edi = DI;

  if (i->os_32) {
    Bit32u value32=0;

    if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
      if ( !bx_cpu. allow_io(DX, 4) ) {
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      }

    // Write a zero to memory, to trigger any segment or page
    // faults before reading from IO port.
    write_virtual_dword(BX_SEG_REG_ES, edi, &value32);

    BX_ASSERT(0=="IO")
;  //value32 = BX_INP(DX, 4);

    /* no seg override allowed */
    write_virtual_dword(BX_SEG_REG_ES, edi, &value32);
    incr = 4;
    }
  else {
    Bit16u value16=0;

    if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
      if ( !bx_cpu. allow_io(DX, 2) ) {
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      }

    // Write a zero to memory, to trigger any segment or page
    // faults before reading from IO port.
    write_virtual_word(BX_SEG_REG_ES, edi, &value16);

    BX_ASSERT(0=="IO")
;  //value16 = BX_INP(DX, 2);

    /* no seg override allowed */
    write_virtual_word(BX_SEG_REG_ES, edi, &value16);
    incr = 2;
    }

  if (i->as_32) {
    if (bx_cpu. eflags.df)
      EDI = EDI - incr;
    else
      EDI = EDI + incr;
    }
  else {
    if (bx_cpu. eflags.df)
      DI = DI - incr;
    else
      DI = DI + incr;
    }
}

  void
bx_cpu_c::OUTSB_DXXb(BxInstruction_t *i)
{
  unsigned seg;
  Bit8u value8;
  Bit32u esi;

  if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
    if ( !bx_cpu. allow_io(DX, 1) ) {
      exception(BX_GP_EXCEPTION, 0, 0);
      }
    }

  if (!BX_NULL_SEG_REG(i->seg)) {
    seg = i->seg;
    }
  else {
    seg = BX_SEG_REG_DS;
    }

  if (i->as_32)
    esi = ESI;
  else
    esi = SI;

  read_virtual_byte(seg, esi, &value8);

  BX_ASSERT(0=="IO")
;  //BX_OUTP(DX, value8, 1);

  if (i->as_32) {
    if (bx_cpu. eflags.df)
      ESI -= 1;
    else
      ESI += 1;
    }
  else {
    if (bx_cpu. eflags.df)
      SI -= 1;
    else
      SI += 1;
    }
}

  void
bx_cpu_c::OUTSW_DXXv(BxInstruction_t *i)
  // output word/doubleword string to port
{
  unsigned seg;
  Bit32u esi;
  unsigned int incr;

  if (!BX_NULL_SEG_REG(i->seg)) {
    seg = i->seg;
    }
  else {
    seg = BX_SEG_REG_DS;
    }

  if (i->as_32)
    esi = ESI;
  else
    esi = SI;

  if (i->os_32) {
    Bit32u value32;

    if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
      if ( !bx_cpu. allow_io(DX, 4) ) {
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      }

    read_virtual_dword(seg, esi, &value32);

    BX_ASSERT(0=="IO");  
	//BX_OUTP(DX, value32, 4);
    incr = 4;
    }
  else {
    Bit16u value16;

    if (bx_cpu. cr0.pe && (bx_cpu. eflags.vm || (CPL>IOPL))) {
      if ( !bx_cpu. allow_io(DX, 2) ) {
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      }

    read_virtual_word(seg, esi, &value16);

    BX_ASSERT(0=="IO");  
	//BX_OUTP(DX, value16, 2);
    incr = 2;
    }

  if (i->as_32) {
    if (bx_cpu. eflags.df)
      ESI = ESI - incr;
    else
      ESI = ESI + incr;
    }
  else {
    if (bx_cpu. eflags.df)
      SI = SI - incr;
    else
      SI = SI + incr;
    }
}


  void
bx_cpu_c::IN_ALIb(BxInstruction_t *i)
{
  Bit8u al, imm8;

  imm8 = i->Ib;

  al = bx_cpu. inp8(imm8);

  AL = al;
}

  void
bx_cpu_c::IN_eAXIb(BxInstruction_t *i)
{
  Bit8u imm8;


  imm8 = i->Ib;

#if BX_CPU_LEVEL > 2
  if (i->os_32) {
    Bit32u eax;

    eax = bx_cpu. inp32(imm8);
    EAX = eax;
    }
  else
#endif /* BX_CPU_LEVEL > 2 */
    {
    Bit16u ax;

    ax = bx_cpu. inp16(imm8);
    AX = ax;
    }
}

  void
bx_cpu_c::OUT_IbAL(BxInstruction_t *i)
{
  Bit8u al, imm8;

  imm8 = i->Ib;

  al = AL;

  bx_cpu. outp8(imm8, al);
}

  void
bx_cpu_c::OUT_IbeAX(BxInstruction_t *i)
{
  Bit8u imm8;

  imm8 = i->Ib;

#if BX_CPU_LEVEL > 2
  if (i->os_32) {
    bx_cpu. outp32(imm8, EAX);
    }
  else
#endif /* BX_CPU_LEVEL > 2 */
    {
    bx_cpu. outp16(imm8, AX);
    }
}

  void
bx_cpu_c::IN_ALDX(BxInstruction_t *i)
{
  Bit8u al;

  al = bx_cpu. inp8(DX);

  AL = al;
}

  void
bx_cpu_c::IN_eAXDX(BxInstruction_t *i)
{
#if BX_CPU_LEVEL > 2
  if (i->os_32) {
    Bit32u eax;

    eax = bx_cpu. inp32(DX);
    EAX = eax;
    }
  else
#endif /* BX_CPU_LEVEL > 2 */
    {
    Bit16u ax;

    ax = bx_cpu. inp16(DX);
    AX = ax;
    }
}

  void
bx_cpu_c::OUT_DXAL(BxInstruction_t *i)
{
  Bit16u dx;
  Bit8u al;

  dx = DX;
  al = AL;

  bx_cpu. outp8(dx, al);
}

  void
bx_cpu_c::OUT_DXeAX(BxInstruction_t *i)
{
  Bit16u dx;

  dx = DX;

#if BX_CPU_LEVEL > 2
  if (i->os_32) {
    bx_cpu. outp32(dx, EAX);
    }
  else
#endif /* BX_CPU_LEVEL > 2 */
    {
    bx_cpu. outp16(dx, AX);
    }
}
