/////////////////////////////////////////////////////////////////////////
// $Id: lazy_flags.cc,v 1.6 2001/10/03 13:10:37 bdenney Exp $
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







  Boolean
bx_cpu_c::get_CF(void)
{
  switch ( bx_cpu. lf_flags_status & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. eflags.cf);

    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_XADD8:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.result_8 <
                              bx_cpu. oszapc.op1_8);
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_XADD16:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.result_16 <
                              bx_cpu. oszapc.op1_16);
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_XADD32:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.result_32 <
                              bx_cpu. oszapc.op1_32);
          break;
        case BX_INSTR_ADC8:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.result_8 <
             bx_cpu. oszapc.op1_8) ||
            (bx_cpu. oszapc.prev_CF &&
             bx_cpu. oszapc.result_8 ==
             bx_cpu. oszapc.op1_8);
          break;
        case BX_INSTR_ADC16:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.result_16 <
             bx_cpu. oszapc.op1_16) ||
            (bx_cpu. oszapc.prev_CF &&
             bx_cpu. oszapc.result_16 ==
             bx_cpu. oszapc.op1_16);
          break;
        case BX_INSTR_ADC32:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.result_32 <
             bx_cpu. oszapc.op1_32) ||
            (bx_cpu. oszapc.prev_CF &&
             bx_cpu. oszapc.result_32 ==
             bx_cpu. oszapc.op1_32);
          break;
        case BX_INSTR_SUB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.op1_8 <
                              bx_cpu. oszapc.op2_8);
          break;
        case BX_INSTR_SUB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.op1_16 <
                              bx_cpu. oszapc.op2_16);
          break;
        case BX_INSTR_SUB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
          bx_cpu. eflags.cf = (bx_cpu. oszapc.op1_32 <
                              bx_cpu. oszapc.op2_32);
          break;
        case BX_INSTR_SBB8:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_8 <
             bx_cpu. oszapc.result_8) ||
            ((bx_cpu. oszapc.op2_8==0xff) &&
             bx_cpu. oszapc.prev_CF);
          break;
        case BX_INSTR_SBB16:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_16 <
             bx_cpu. oszapc.result_16) ||
            ((bx_cpu. oszapc.op2_16==0xffff) &&
             bx_cpu. oszapc.prev_CF);
          break;
        case BX_INSTR_SBB32:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_32 <
             bx_cpu. oszapc.result_32) ||
            ((bx_cpu. oszapc.op2_32==0xffffffff) &&
             bx_cpu. oszapc.prev_CF);
          break;
        case BX_INSTR_NEG8:
          bx_cpu. eflags.cf =
            bx_cpu. oszapc.op1_8 != 0;
          break;
        case BX_INSTR_NEG16:
          bx_cpu. eflags.cf =
            bx_cpu. oszapc.op1_16 != 0;
          break;
        case BX_INSTR_NEG32:
          bx_cpu. eflags.cf =
            bx_cpu. oszapc.op1_32 != 0;
          break;
        case BX_INSTR_OR8:
        case BX_INSTR_OR16:
        case BX_INSTR_OR32:
        case BX_INSTR_AND8:
        case BX_INSTR_AND16:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST8:
        case BX_INSTR_TEST16:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR8:
        case BX_INSTR_XOR16:
        case BX_INSTR_XOR32:
          bx_cpu. eflags.cf = 0;
          break;
        case BX_INSTR_SHR8:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_8 >>
              (bx_cpu. oszapc.op2_8 - 1)) & 0x01;
          break;
        case BX_INSTR_SHR16:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_16 >>
              (bx_cpu. oszapc.op2_16 - 1)) & 0x01;
          break;
        case BX_INSTR_SHR32:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_32 >>
              (bx_cpu. oszapc.op2_32 - 1)) & 0x01;
          break;
        case BX_INSTR_SHL8:
          if (bx_cpu. oszapc.op2_8 <= 8) {
            bx_cpu. eflags.cf =
              (bx_cpu. oszapc.op1_8 >>
                (8 - bx_cpu. oszapc.op2_8)) & 0x01;
            }
          else {
            bx_cpu. eflags.cf = 0;
            }
          break;
        case BX_INSTR_SHL16:
          if (bx_cpu. oszapc.op2_16 <= 16) {
            bx_cpu. eflags.cf =
              (bx_cpu. oszapc.op1_16 >>
                (16 - bx_cpu. oszapc.op2_16)) & 0x01;
            }
          else {
            bx_cpu. eflags.cf = 0;
            }
          break;
        case BX_INSTR_SHL32:
          bx_cpu. eflags.cf =
            (bx_cpu. oszapc.op1_32 >>
              (32 - bx_cpu. oszapc.op2_32)) & 0x01;
          break;
        default:
          BX_PANIC(("get_CF: OSZAPC: unknown instr %u",
            (unsigned) bx_cpu. oszapc.instr));
        }
      bx_cpu. lf_flags_status &= 0xfffff0;
      return(bx_cpu. eflags.cf);

    default:
      BX_PANIC(("get_CF: unknown case"));
      return(0);
    }
}


  Boolean
bx_cpu_c::get_AF(void)
{
  switch ( (bx_cpu. lf_flags_status>>8) & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. eflags.af);

    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_ADC8:
        case BX_INSTR_SUB8:
        case BX_INSTR_SBB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_XADD8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
          bx_cpu. eflags.af =
            ((bx_cpu. oszapc.op1_8 ^
              bx_cpu. oszapc.op2_8) ^
             bx_cpu. oszapc.result_8) & 0x10;
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_ADC16:
        case BX_INSTR_SUB16:
        case BX_INSTR_SBB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_XADD16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
          bx_cpu. eflags.af =
            ((bx_cpu. oszapc.op1_16 ^
              bx_cpu. oszapc.op2_16) ^
             bx_cpu. oszapc.result_16) & 0x10;
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_ADC32:
        case BX_INSTR_SUB32:
        case BX_INSTR_SBB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_XADD32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
          bx_cpu. eflags.af =
            ((bx_cpu. oszapc.op1_32 ^
              bx_cpu. oszapc.op2_32) ^
             bx_cpu. oszapc.result_32) & 0x10;
          break;
        case BX_INSTR_NEG8:
          bx_cpu. eflags.af =
            (bx_cpu. oszapc.op1_8 & 0x0f) > 0;
          break;
        case BX_INSTR_NEG16:
          bx_cpu. eflags.af =
            (bx_cpu. oszapc.op1_16 & 0x0f) > 0;
          break;
        case BX_INSTR_NEG32:
          bx_cpu. eflags.af =
            (bx_cpu. oszapc.op1_32 & 0x0f) > 0;
          break;
        case BX_INSTR_OR8:
        case BX_INSTR_OR16:
        case BX_INSTR_OR32:
        case BX_INSTR_AND8:
        case BX_INSTR_AND16:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST8:
        case BX_INSTR_TEST16:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR8:
        case BX_INSTR_XOR16:
        case BX_INSTR_XOR32:
        case BX_INSTR_SHR8:
        case BX_INSTR_SHR16:
        case BX_INSTR_SHR32:
        case BX_INSTR_SHL8:
        case BX_INSTR_SHL16:
        case BX_INSTR_SHL32:
          bx_cpu. eflags.af = 0;
          /* undefined */
          break;
        default:
          BX_PANIC(("get_AF: OSZAPC: unknown instr %u",
            (unsigned) bx_cpu. oszapc.instr));
        }
      bx_cpu. lf_flags_status &= 0xfff0ff;
      return(bx_cpu. eflags.af);

    case BX_LF_INDEX_OSZAP:
      switch (bx_cpu. oszap.instr) {
        case BX_INSTR_INC8:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_8 & 0x0f) == 0;
          break;
        case BX_INSTR_INC16:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_16 & 0x0f) == 0;
          break;
        case BX_INSTR_INC32:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_32 & 0x0f) == 0;
          break;
        case BX_INSTR_DEC8:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_8 & 0x0f) == 0x0f;
          break;
        case BX_INSTR_DEC16:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_16 & 0x0f) == 0x0f;
          break;
        case BX_INSTR_DEC32:
          bx_cpu. eflags.af =
            (bx_cpu. oszap.result_32 & 0x0f) == 0x0f;
          break;
        default:
          BX_PANIC(("get_AF: OSZAP: unknown instr %u",
            (unsigned) bx_cpu. oszap.instr));
        }
      bx_cpu. lf_flags_status &= 0xfff0ff;
      return(bx_cpu. eflags.af);

    default:
      BX_PANIC(("get_AF: unknown case"));
      return(0);
    }
}


  Boolean
bx_cpu_c::get_ZF(void)
{
  switch ( (bx_cpu. lf_flags_status>>12) & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. eflags.zf);

    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_ADC8:
        case BX_INSTR_SUB8:
        case BX_INSTR_SBB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_NEG8:
        case BX_INSTR_XADD8:
        case BX_INSTR_OR8:
        case BX_INSTR_AND8:
        case BX_INSTR_TEST8:
        case BX_INSTR_XOR8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
        case BX_INSTR_SHR8:
        case BX_INSTR_SHL8:
          bx_cpu. eflags.zf = (bx_cpu. oszapc.result_8 == 0);
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_ADC16:
        case BX_INSTR_SUB16:
        case BX_INSTR_SBB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_NEG16:
        case BX_INSTR_XADD16:
        case BX_INSTR_OR16:
        case BX_INSTR_AND16:
        case BX_INSTR_TEST16:
        case BX_INSTR_XOR16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
        case BX_INSTR_SHR16:
        case BX_INSTR_SHL16:
          bx_cpu. eflags.zf = (bx_cpu. oszapc.result_16 == 0);
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_ADC32:
        case BX_INSTR_SUB32:
        case BX_INSTR_SBB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_NEG32:
        case BX_INSTR_XADD32:
        case BX_INSTR_OR32:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
        case BX_INSTR_SHR32:
        case BX_INSTR_SHL32:
          bx_cpu. eflags.zf = (bx_cpu. oszapc.result_32 == 0);
          break;
        default:
          BX_PANIC(("get_ZF: OSZAPC: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xff0fff;
      return(bx_cpu. eflags.zf);

    case BX_LF_INDEX_OSZAP:
      switch (bx_cpu. oszap.instr) {
        case BX_INSTR_INC8:
        case BX_INSTR_DEC8:
          bx_cpu. eflags.zf = (bx_cpu. oszap.result_8 == 0);
          break;
        case BX_INSTR_INC16:
        case BX_INSTR_DEC16:
          bx_cpu. eflags.zf = (bx_cpu. oszap.result_16 == 0);
          break;
        case BX_INSTR_INC32:
        case BX_INSTR_DEC32:
          bx_cpu. eflags.zf = (bx_cpu. oszap.result_32 == 0);
          break;
        default:
          BX_PANIC(("get_ZF: OSZAP: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xff0fff;
      return(bx_cpu. eflags.zf);

    default:
      BX_PANIC(("get_ZF: unknown case"));
      return(0);
    }
}


  Boolean
bx_cpu_c::get_SF(void)
{
  switch ( (bx_cpu. lf_flags_status>>16) & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. eflags.sf);

    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_ADC8:
        case BX_INSTR_SUB8:
        case BX_INSTR_SBB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_NEG8:
        case BX_INSTR_XADD8:
        case BX_INSTR_OR8:
        case BX_INSTR_AND8:
        case BX_INSTR_TEST8:
        case BX_INSTR_XOR8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
        case BX_INSTR_SHR8:
        case BX_INSTR_SHL8:
          bx_cpu. eflags.sf =
            (bx_cpu. oszapc.result_8 >= 0x80);
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_ADC16:
        case BX_INSTR_SUB16:
        case BX_INSTR_SBB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_NEG16:
        case BX_INSTR_XADD16:
        case BX_INSTR_OR16:
        case BX_INSTR_AND16:
        case BX_INSTR_TEST16:
        case BX_INSTR_XOR16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
        case BX_INSTR_SHR16:
        case BX_INSTR_SHL16:
          bx_cpu. eflags.sf =
            (bx_cpu. oszapc.result_16 >= 0x8000);
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_ADC32:
        case BX_INSTR_SUB32:
        case BX_INSTR_SBB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_NEG32:
        case BX_INSTR_XADD32:
        case BX_INSTR_OR32:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
        case BX_INSTR_SHR32:
        case BX_INSTR_SHL32:
          bx_cpu. eflags.sf =
            (bx_cpu. oszapc.result_32 >= 0x80000000);
          break;
        default:
          BX_PANIC(("get_SF: OSZAPC: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xf0ffff;
      return(bx_cpu. eflags.sf);

    case BX_LF_INDEX_OSZAP:
      switch (bx_cpu. oszap.instr) {
        case BX_INSTR_INC8:
        case BX_INSTR_DEC8:
          bx_cpu. eflags.sf =
            (bx_cpu. oszap.result_8 >= 0x80);
          break;
        case BX_INSTR_INC16:
        case BX_INSTR_DEC16:
          bx_cpu. eflags.sf =
            (bx_cpu. oszap.result_16 >= 0x8000);
          break;
        case BX_INSTR_INC32:
        case BX_INSTR_DEC32:
          bx_cpu. eflags.sf =
            (bx_cpu. oszap.result_32 >= 0x80000000);
          break;
        default:
          BX_PANIC(("get_SF: OSZAP: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xf0ffff;
      return(bx_cpu. eflags.sf);

    default:
      BX_PANIC(("get_SF: unknown case"));
      return(0);
    }
}

  Boolean
bx_cpu_c::get_OF(void)
{
  Bit8u op1_b7, op2_b7, result_b7;
  Bit16u op1_b15, op2_b15, result_b15;
  Bit32u op1_b31, op2_b31, result_b31;

  switch ( (bx_cpu. lf_flags_status>>20) & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. eflags.of);

    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_ADC8:
        case BX_INSTR_XADD8:
          op1_b7 = bx_cpu. oszapc.op1_8 & 0x80;
          op2_b7 = bx_cpu. oszapc.op2_8 & 0x80;
          result_b7 = bx_cpu. oszapc.result_8 & 0x80;

          bx_cpu. eflags.of =  (op1_b7 == op2_b7) && (result_b7 ^ op2_b7);
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_ADC16:
        case BX_INSTR_XADD16:
          op1_b15 = bx_cpu. oszapc.op1_16 & 0x8000;
          op2_b15 = bx_cpu. oszapc.op2_16 & 0x8000;
          result_b15 = bx_cpu. oszapc.result_16 & 0x8000;

          bx_cpu. eflags.of =  (op1_b15 == op2_b15) && (result_b15 ^ op2_b15);
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_ADC32:
        case BX_INSTR_XADD32:
          op1_b31 = bx_cpu. oszapc.op1_32 & 0x80000000;
          op2_b31 = bx_cpu. oszapc.op2_32 & 0x80000000;
          result_b31 = bx_cpu. oszapc.result_32 & 0x80000000;

          bx_cpu. eflags.of =  (op1_b31 == op2_b31) && (result_b31 ^ op2_b31);
          break;
        case BX_INSTR_SUB8:
        case BX_INSTR_SBB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
          op1_b7 = bx_cpu. oszapc.op1_8 & 0x80;
          op2_b7 = bx_cpu. oszapc.op2_8 & 0x80;
          result_b7 = bx_cpu. oszapc.result_8 & 0x80;

          bx_cpu. eflags.of =  (op1_b7 ^ op2_b7) && (op1_b7 ^ result_b7);
          break;
        case BX_INSTR_SUB16:
        case BX_INSTR_SBB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
          op1_b15 = bx_cpu. oszapc.op1_16 & 0x8000;
          op2_b15 = bx_cpu. oszapc.op2_16 & 0x8000;
          result_b15 = bx_cpu. oszapc.result_16 & 0x8000;

          bx_cpu. eflags.of =  (op1_b15 ^ op2_b15) && (op1_b15 ^ result_b15);
          break;
        case BX_INSTR_SUB32:
        case BX_INSTR_SBB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
          op1_b31 = bx_cpu. oszapc.op1_32 & 0x80000000;
          op2_b31 = bx_cpu. oszapc.op2_32 & 0x80000000;
          result_b31 = bx_cpu. oszapc.result_32 & 0x80000000;

          bx_cpu. eflags.of =  (op1_b31 ^ op2_b31) && (op1_b31 ^ result_b31);
          break;
        case BX_INSTR_NEG8:
          bx_cpu. eflags.of =
            (bx_cpu. oszapc.op1_8 == 0x80);
          break;
        case BX_INSTR_NEG16:
          bx_cpu. eflags.of =
            (bx_cpu. oszapc.op1_16 == 0x8000);
          break;
        case BX_INSTR_NEG32:
          bx_cpu. eflags.of =
            (bx_cpu. oszapc.op1_32 == 0x80000000);
          break;
        case BX_INSTR_OR8:
        case BX_INSTR_OR16:
        case BX_INSTR_OR32:
        case BX_INSTR_AND8:
        case BX_INSTR_AND16:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST8:
        case BX_INSTR_TEST16:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR8:
        case BX_INSTR_XOR16:
        case BX_INSTR_XOR32:
          bx_cpu. eflags.of = 0;
          break;
        case BX_INSTR_SHR8:
          if (bx_cpu. oszapc.op2_8 == 1)
            bx_cpu. eflags.of =
              (bx_cpu. oszapc.op1_8 >= 0x80);
          break;
        case BX_INSTR_SHR16:
          if (bx_cpu. oszapc.op2_16 == 1)
            bx_cpu. eflags.of =
              (bx_cpu. oszapc.op1_16 >= 0x8000);
          break;
        case BX_INSTR_SHR32:
          if (bx_cpu. oszapc.op2_32 == 1)
            bx_cpu. eflags.of =
              (bx_cpu. oszapc.op1_32 >= 0x80000000);
          break;
        case BX_INSTR_SHL8:
          if (bx_cpu. oszapc.op2_8 == 1)
            bx_cpu. eflags.of =
              ((bx_cpu. oszapc.op1_8 ^
                bx_cpu. oszapc.result_8) & 0x80) > 0;
          break;
        case BX_INSTR_SHL16:
          if (bx_cpu. oszapc.op2_16 == 1)
            bx_cpu. eflags.of =
              ((bx_cpu. oszapc.op1_16 ^
                bx_cpu. oszapc.result_16) & 0x8000) > 0;
          break;
        case BX_INSTR_SHL32:
          if (bx_cpu. oszapc.op2_32 == 1)
            bx_cpu. eflags.of =
              ((bx_cpu. oszapc.op1_32 ^
                bx_cpu. oszapc.result_32) & 0x80000000) > 0;
          break;
        default:
          BX_PANIC(("get_OF: OSZAPC: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0x0fffff;
      return(bx_cpu. eflags.of);

    case BX_LF_INDEX_OSZAP:
      switch (bx_cpu. oszap.instr) {
        case BX_INSTR_INC8:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_8 == 0x80;
          break;
        case BX_INSTR_INC16:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_16 == 0x8000;
          break;
        case BX_INSTR_INC32:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_32 == 0x80000000;
          break;
        case BX_INSTR_DEC8:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_8 == 0x7F;
          break;
        case BX_INSTR_DEC16:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_16 == 0x7FFF;
          break;
        case BX_INSTR_DEC32:
          bx_cpu. eflags.of =
            bx_cpu. oszap.result_32 == 0x7FFFFFFF;
          break;
        default:
          BX_PANIC(("get_OF: OSZAP: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0x0fffff;
      return(bx_cpu. eflags.of);

    default:
      BX_PANIC(("get_OF: unknown case"));
      return(0);
    }
}

  Boolean
bx_cpu_c::get_PF(void)
{
  switch ( (bx_cpu. lf_flags_status>>4) & 0x00000f ) {
    case BX_LF_INDEX_KNOWN:
      return(bx_cpu. lf_pf);
    case BX_LF_INDEX_OSZAPC:
      switch (bx_cpu. oszapc.instr) {
        case BX_INSTR_ADD8:
        case BX_INSTR_ADC8:
        case BX_INSTR_SUB8:
        case BX_INSTR_SBB8:
        case BX_INSTR_CMP8:
        case BX_INSTR_NEG8:
        case BX_INSTR_XADD8:
        case BX_INSTR_OR8:
        case BX_INSTR_AND8:
        case BX_INSTR_TEST8:
        case BX_INSTR_XOR8:
        case BX_INSTR_CMPS8:
        case BX_INSTR_SCAS8:
        case BX_INSTR_SHR8:
        case BX_INSTR_SHL8:
          bx_cpu. lf_pf =
            bx_parity_lookup[bx_cpu. oszapc.result_8];
          break;
        case BX_INSTR_ADD16:
        case BX_INSTR_ADC16:
        case BX_INSTR_SUB16:
        case BX_INSTR_SBB16:
        case BX_INSTR_CMP16:
        case BX_INSTR_NEG16:
        case BX_INSTR_XADD16:
        case BX_INSTR_OR16:
        case BX_INSTR_AND16:
        case BX_INSTR_TEST16:
        case BX_INSTR_XOR16:
        case BX_INSTR_CMPS16:
        case BX_INSTR_SCAS16:
        case BX_INSTR_SHR16:
        case BX_INSTR_SHL16:
          bx_cpu. lf_pf =
            bx_parity_lookup[(Bit8u) bx_cpu. oszapc.result_16];
          break;
        case BX_INSTR_ADD32:
        case BX_INSTR_ADC32:
        case BX_INSTR_SUB32:
        case BX_INSTR_SBB32:
        case BX_INSTR_CMP32:
        case BX_INSTR_NEG32:
        case BX_INSTR_XADD32:
        case BX_INSTR_OR32:
        case BX_INSTR_AND32:
        case BX_INSTR_TEST32:
        case BX_INSTR_XOR32:
        case BX_INSTR_CMPS32:
        case BX_INSTR_SCAS32:
        case BX_INSTR_SHR32:
        case BX_INSTR_SHL32:
          bx_cpu. lf_pf =
            bx_parity_lookup[(Bit8u) bx_cpu. oszapc.result_32];
          break;
        default:
          BX_PANIC(("get_PF: OSZAPC: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xffff0f;
      return(bx_cpu. lf_pf);

    case BX_LF_INDEX_OSZAP:
      switch (bx_cpu. oszap.instr) {
        case BX_INSTR_INC8:
        case BX_INSTR_DEC8:
          bx_cpu. lf_pf =
            bx_parity_lookup[bx_cpu. oszap.result_8];
          break;
        case BX_INSTR_INC16:
        case BX_INSTR_DEC16:
          bx_cpu. lf_pf =
            bx_parity_lookup[(Bit8u) bx_cpu. oszap.result_16];
          break;
        case BX_INSTR_INC32:
        case BX_INSTR_DEC32:
          bx_cpu. lf_pf =
            bx_parity_lookup[(Bit8u) bx_cpu. oszap.result_32];
          break;
        default:
          BX_PANIC(("get_PF: OSZAP: unknown instr"));
        }
      bx_cpu. lf_flags_status &= 0xffff0f;
      return(bx_cpu. lf_pf);

    case BX_LF_INDEX_P:
      bx_cpu. lf_pf = bx_parity_lookup[bx_cpu. eflags.pf_byte];
      bx_cpu. lf_flags_status &= 0xffff0f;
      return(bx_cpu. lf_pf);

    default:
      BX_PANIC(("get_PF: unknown case"));
      return(0);
    }
}
