/////////////////////////////////////////////////////////////////////////
// $Id: ctrl_xfer16.cc,v 1.6 2001/10/03 13:10:37 bdenney Exp $
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
bx_cpu_c::RETnear16_Iw(BxInstruction_t *i)
{
  Bit16u imm16;
  Bit32u temp_ESP;
  Bit16u return_IP;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_ret;
#endif

  if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b) /* 32bit stack */
    temp_ESP = ESP;
  else
    temp_ESP = SP;

  imm16 = i->Iw;

  invalidate_prefetch_q();


    if (protected_mode()) {
      if ( !can_pop(2) ) {
        BX_PANIC(("retnear_iw: can't pop IP"));
        /* ??? #SS(0) -or #GP(0) */
        }

      access_linear(bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.base + temp_ESP + 0,
        2, CPL==3, BX_READ, &return_IP);

      if ( return_IP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled ) {
        BX_PANIC(("retnear_iw: IP > limit"));
        }

      if ( !can_pop(2 + imm16) ) {
        BX_PANIC(("retnear_iw: can't release bytes from stack"));
        /* #GP(0) -or #SS(0) ??? */
        }

      bx_cpu. eip = return_IP;
      if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b) /* 32bit stack */
        ESP += 2 + imm16; /* ??? should it be 2*imm16 ? */
      else
        SP  += 2 + imm16;
      }
    else {
      pop_16(&return_IP);
      bx_cpu. eip = return_IP;
      if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b) /* 32bit stack */
        ESP += imm16; /* ??? should it be 2*imm16 ? */
      else
        SP  += imm16;
      }

  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_RET, bx_cpu. eip);
}

  void
bx_cpu_c::RETnear16(BxInstruction_t *i)
{
  Bit32u temp_ESP;
  Bit16u return_IP;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_ret;
#endif

  invalidate_prefetch_q();

  if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b) /* 32bit stack */
    temp_ESP = ESP;
  else
    temp_ESP = SP;


    if (protected_mode()) {
      if ( !can_pop(2) ) {
        BX_PANIC(("retnear: can't pop IP"));
        /* ??? #SS(0) -or #GP(0) */
        }

      access_linear(bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.base + temp_ESP + 0,
        2, CPL==3, BX_READ, &return_IP);

      if ( return_IP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled ) {
        BX_PANIC(("retnear: IP > limit"));
        }

      bx_cpu. eip = return_IP;
      if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b) /* 32bit stack */
        ESP += 2;
      else
        SP  += 2;
      }
    else {
      pop_16(&return_IP);
      bx_cpu. eip = return_IP;
      }

  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_RET, bx_cpu. eip);
}

  void
bx_cpu_c::RETfar16_Iw(BxInstruction_t *i)
{
  Bit16s imm16;
  Bit16u ip, cs_raw;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_ret;
#endif

  /* ??? is imm16, number of bytes/words depending on operandsize ? */

  imm16 = i->Iw;

  invalidate_prefetch_q();

#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    bx_cpu. return_protected(i, imm16);
    goto done;
    }
#endif


    pop_16(&ip);
    pop_16(&cs_raw);
    bx_cpu. eip = (Bit32u) ip;
    load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);
    if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b)
      ESP += imm16;
    else
      SP  += imm16;

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_RET,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}

  void
bx_cpu_c::RETfar16(BxInstruction_t *i)
{
  Bit16u ip, cs_raw;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_ret;
#endif

  invalidate_prefetch_q();

#if BX_CPU_LEVEL >= 2
  if ( protected_mode() ) {
    bx_cpu. return_protected(i, 0);
    goto done;
    }
#endif

    pop_16(&ip);
    pop_16(&cs_raw);
    bx_cpu. eip = (Bit32u) ip;
    load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_RET,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}



  void
bx_cpu_c::CALL_Aw(BxInstruction_t *i)
{
  Bit32u new_EIP;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_call;
#endif

  invalidate_prefetch_q();

  new_EIP = EIP + (Bit32s) i->Id;
  new_EIP &= 0x0000ffff;
#if BX_CPU_LEVEL >= 2
  if ( protected_mode() &&
       (new_EIP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled) ) {
    BX_PANIC(("call_av: new_IP > bx_cpu. sregs[BX_SEG_REG_CS].limit"));
    exception(BX_GP_EXCEPTION, 0, 0);
    }
#endif

  /* push 16 bit EA of next instruction */
  push_16(IP);

  bx_cpu. eip = new_EIP;

  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_CALL, bx_cpu. eip);
}

  void
bx_cpu_c::CALL16_Ap(BxInstruction_t *i)
{
  Bit16u cs_raw;
  Bit16u disp16;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_call;
#endif

  disp16 = i->Iw;
  cs_raw = i->Iw2;
  invalidate_prefetch_q();

#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    bx_cpu. call_protected(i, cs_raw, disp16);
    goto done;
    }
#endif
  push_16(bx_cpu. sregs[BX_SEG_REG_CS].selector.value);
  push_16((Bit16u) bx_cpu. eip);
  bx_cpu. eip = (Bit32u) disp16;
  load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_CALL,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}

  void
bx_cpu_c::CALL_Ew(BxInstruction_t *i)
{
  Bit32u temp_ESP;
  Bit16u op1_16;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_call;
#endif

  if (bx_cpu. sregs[BX_SEG_REG_SS].cache.u.segment.d_b)
    temp_ESP = ESP;
  else
    temp_ESP = SP;


    /* op1_16 is a register or memory reference */
    if (i->mod == 0xc0) {
      op1_16 = BX_READ_16BIT_REG(i->rm);
      }
    else {
      /* pointer, segment address pair */
      read_virtual_word(i->seg, i->rm_addr, &op1_16);
      }
    invalidate_prefetch_q();

#if BX_CPU_LEVEL >= 2
    if (protected_mode()) {
      if (op1_16 > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled) {
        BX_PANIC(("call_ev: IP out of CS limits!"));
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      if ( !can_push(&bx_cpu. sregs[BX_SEG_REG_SS].cache, temp_ESP, 2) ) {
        BX_PANIC(("call_ev: can't push IP"));
        }
      }
#endif

    push_16(IP);

    bx_cpu. eip = op1_16;

  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_CALL, bx_cpu. eip);
}

  void
bx_cpu_c::CALL16_Ep(BxInstruction_t *i)
{
  Bit16u cs_raw;
  Bit16u op1_16;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_call;
#endif

    /* op1_16 is a register or memory reference */
    if (i->mod == 0xc0) {
      BX_PANIC(("CALL_Ep: op1 is a register"));
      }

    /* pointer, segment address pair */
    read_virtual_word(i->seg, i->rm_addr, &op1_16);
    read_virtual_word(i->seg, i->rm_addr+2, &cs_raw);
    invalidate_prefetch_q();

    if ( protected_mode() ) {
      bx_cpu. call_protected(i, cs_raw, op1_16);
      goto done;
      }

    push_16(bx_cpu. sregs[BX_SEG_REG_CS].selector.value);
    push_16(IP);

    bx_cpu. eip = op1_16;
    load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_CALL,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}


  void
bx_cpu_c::JMP_Jw(BxInstruction_t *i)
{
  Bit32u new_EIP;


  invalidate_prefetch_q();

  new_EIP = EIP + (Bit32s) i->Id;
  new_EIP &= 0x0000ffff;

#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    if ( new_EIP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled ) {
      BX_PANIC(("jmp_jv: offset outside of CS limits"));
      exception(BX_GP_EXCEPTION, 0, 0);
      }
    }
#endif

  bx_cpu. eip = new_EIP;
  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_JMP, new_EIP);
}

  void
bx_cpu_c::JCC_Jw(BxInstruction_t *i)
{
  Boolean condition = 0;

  switch (i->b1 & 0x0f) {
    case 0x00: /* JO */ condition = get_OF(); break;
    case 0x01: /* JNO */ condition = !get_OF(); break;
    case 0x02: /* JB */ condition = get_CF(); break;
    case 0x03: /* JNB */ condition = !get_CF(); break;
    case 0x04: /* JZ */ condition = get_ZF(); break;
    case 0x05: /* JNZ */ condition = !get_ZF(); break;
    case 0x06: /* JBE */ condition = get_CF() || get_ZF(); break;
    case 0x07: /* JNBE */ condition = !get_CF() && !get_ZF(); break;
    case 0x08: /* JS */ condition = get_SF(); break;
    case 0x09: /* JNS */ condition = !get_SF(); break;
    case 0x0A: /* JP */ condition = get_PF(); break;
    case 0x0B: /* JNP */ condition = !get_PF(); break;
    case 0x0C: /* JL */ condition = get_SF() != get_OF(); break;
    case 0x0D: /* JNL */ condition = get_SF() == get_OF(); break;
    case 0x0E: /* JLE */ condition = get_ZF() || (get_SF() != get_OF());
      break;
    case 0x0F: /* JNLE */ condition = (get_SF() == get_OF()) &&
                            !get_ZF();
      break;
    }

  if (condition) {
    Bit32u new_EIP;

    new_EIP = EIP + (Bit32s) i->Id;
    new_EIP &= 0x0000ffff;
#if BX_CPU_LEVEL >= 2
    if (protected_mode()) {
      if ( new_EIP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled ) {
        BX_PANIC(("jo_routine: offset outside of CS limits"));
        exception(BX_GP_EXCEPTION, 0, 0);
        }
      }
#endif
    EIP = new_EIP;
    BX_INSTR_CNEAR_BRANCH_TAKEN(new_EIP);
    revalidate_prefetch_q();
    }
#if BX_INSTRUMENTATION
  else {
    BX_INSTR_CNEAR_BRANCH_NOT_TAKEN();
    }
#endif
}



  void
bx_cpu_c::JMP_Ew(BxInstruction_t *i)
{
  Bit32u new_EIP;
  Bit16u op1_16;


    /* op1_16 is a register or memory reference */
    if (i->mod == 0xc0) {
      op1_16 = BX_READ_16BIT_REG(i->rm);
      }
    else {
      /* pointer, segment address pair */
      read_virtual_word(i->seg, i->rm_addr, &op1_16);
      }

    invalidate_prefetch_q();
    new_EIP = op1_16;

#if BX_CPU_LEVEL >= 2
  if (protected_mode()) {
    if (new_EIP > bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled) {
      BX_PANIC(("jmp_ev: IP out of CS limits!"));
      exception(BX_GP_EXCEPTION, 0, 0);
      }
    }
#endif

  bx_cpu. eip = new_EIP;

  BX_INSTR_UCNEAR_BRANCH(BX_INSTR_IS_JMP, new_EIP);
}

  /* Far indirect jump */

  void
bx_cpu_c::JMP16_Ep(BxInstruction_t *i)
{
  Bit16u cs_raw;
  Bit16u op1_16;

    /* op1_16 is a register or memory reference */
    if (i->mod == 0xc0) {
      /* far indirect must specify a memory address */
      BX_PANIC(("JMP_Ep(): op1 is a register"));
      }

    /* pointer, segment address pair */
    read_virtual_word(i->seg, i->rm_addr, &op1_16);
    read_virtual_word(i->seg, i->rm_addr+2, &cs_raw);
    invalidate_prefetch_q();

#if BX_CPU_LEVEL >= 2
    if ( protected_mode() ) {
      bx_cpu. jump_protected(i, cs_raw, op1_16);
      goto done;
      }
#endif

    bx_cpu. eip = op1_16;
    load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_JMP,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}

  void
bx_cpu_c::IRET16(BxInstruction_t *i)
{
  Bit16u ip, cs_raw, flags;

#if BX_DEBUGGER
  bx_cpu. show_flag |= Flag_iret;
  bx_cpu. show_eip = bx_cpu. eip;
#endif

  invalidate_prefetch_q();

  if (v8086_mode()) {
    // IOPL check in stack_return_from_v86()
    stack_return_from_v86(i);
    goto done;
    }

#if BX_CPU_LEVEL >= 2
  if (bx_cpu. cr0.pe) {
    iret_protected(i);
    goto done;
    }
#endif


  pop_16(&ip);
  pop_16(&cs_raw);
  pop_16(&flags);

  load_seg_reg(&bx_cpu. sregs[BX_SEG_REG_CS], cs_raw);
  bx_cpu. eip = (Bit32u) ip;
  write_flags(flags, /* change IOPL? */ 1, /* change IF? */ 1);

done:
  BX_INSTR_FAR_BRANCH(BX_INSTR_IS_IRET,
                      bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  return;
}
