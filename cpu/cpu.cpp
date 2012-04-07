/////////////////////////////////////////////////////////////////////////
// $Id: cpu.cc,v 1.25 2002/03/20 23:50:23 bdenney Exp $
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

#define BX_INSTR_SPY 0


#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#define LOG_THIS bx_cpu.

#if BX_USE_CPU_SMF
#define this (BX_CPU(0))
#endif

//unsigned counter[2] = { 0, 0 };



#if BX_SIM_ID == 0   // only need to define once
// This array defines a look-up table for the even parity-ness
// of an 8bit quantity, for optimal assignment of the parity bit
// in the EFLAGS register
const Boolean bx_parity_lookup[256] = {
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
  1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
  };
#endif


#if BX_SMP_PROCESSORS==1
// single processor simulation, so there's one of everything
bx_cpu_c    bx_cpu;
BX_MEM_C    bx_mem;
#else
// multiprocessor simulation, we need an array of cpus and memories
bx_cpu_c    *bx_cpu_array[BX_SMP_PROCESSORS];
BX_MEM_C    *bx_mem_array[BX_ADDRESS_SPACES];
#endif



// notes:
//
// check limit of CS?

#ifdef REGISTER_IADDR
extern void REGISTER_IADDR(Bit32u addr);
#endif

// The CHECK_MAX_INSTRUCTIONS macro allows cpu_loop to execute a few
// instructions and then return so that the other processors have a chance to
// run.  This is used only when simulating multiple processors.
// 
// If maximum instructions have been executed, return.  A count less
// than zero means run forever.
#define CHECK_MAX_INSTRUCTIONS(count) \
  if (count >= 0) {                   \
    count--; if (count == 0) return;  \
  }

#if BX_SMP_PROCESSORS==1
#  define BX_TICK1_IF_SINGLE_PROCESSOR() BX_TICK1()
#else
#  define BX_TICK1_IF_SINGLE_PROCESSOR()
#endif

void process_eip_bp(Bit32u eip);

#if BX_DYNAMIC_TRANSLATION == 0
  void
bx_cpu_c::cpu_loop(Bit32s max_instr_count)
{
  unsigned ret;
  BxInstruction_t i;
  unsigned maxisize;
  Bit8u *fetch_ptr;
  Boolean is_32;

  (void) setjmp( bx_cpu. jmp_buf_env );

  // not sure if these two are used during the async handling... --bbd
  bx_cpu. prev_eip = EIP; // commit new EIP
  bx_cpu. prev_esp = ESP; // commit new ESP

main_cpu_loop:

 // BX_DEBUG(("eip = 0x%.8x", EIP));

  process_eip_bp(EIP);

  // ???
  bx_cpu. EXT = 0;
  bx_cpu. errorno = 0;

  // First check on events which occurred for previous instructions
  // (traps) and ones which are asynchronous to the CPU
  // (hardware interrupts).
  if (bx_cpu. async_event)
    goto handle_async_event;

async_events_processed:
  // added so that all debugging/tracing code uses the correct EIP even in the
  // instruction just after a trap/interrupt.  If you use the prev_eip that was
  // set before handle_async_event, traces and breakpoints fail to show the
  // first instruction of int/trap handlers.
  bx_cpu. prev_eip = EIP; // commit new EIP
  bx_cpu. prev_esp = ESP; // commit new ESP
  
  // Now we can handle things which are synchronous to instruction
  // execution.
  if (bx_cpu. eflags.rf) {
    bx_cpu. eflags.rf = 0;
    }

  // We have ignored processing of external interrupts and
  // debug events on this boundary.  Reset the mask so they
  // will be processed on the next boundary.
  bx_cpu. inhibit_mask = 0;

  is_32 = bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.d_b;

  if (bx_cpu. bytesleft == 0) {
    prefetch();
    }
  fetch_ptr = bx_cpu. fetch_ptr;

  maxisize = 16;
  if (bx_cpu. bytesleft < 16)
    maxisize = bx_cpu. bytesleft;
  ret = FetchDecode(fetch_ptr, &i, maxisize, is_32);

  if (ret) {
    if (i.ResolveModrm) {
      // call method on bx_cpu_c object
      i.ResolveModrm(&i);
      }
    bx_cpu. fetch_ptr += i.ilen;
    bx_cpu. bytesleft -= i.ilen;
fetch_decode_OK:

    if (i.rep_used && (i.attr & BxRepeatable)) {
//repeat_loop:
      if (i.attr & BxRepeatableZF) {
        if (i.as_32) {
          if (ECX != 0) {
            i.execute(&i);
            ECX -= 1;
            }
          if ((i.rep_used==0xf3) && (get_ZF()==0)) goto repeat_done;
          if ((i.rep_used==0xf2) && (get_ZF()!=0)) goto repeat_done;
          if (ECX == 0) goto repeat_done;
          goto repeat_not_done;
          }
        else {
          if (CX != 0) {
            i.execute(&i);
            CX -= 1;
            }
          if ((i.rep_used==0xf3) && (get_ZF()==0)) goto repeat_done;
          if ((i.rep_used==0xf2) && (get_ZF()!=0)) goto repeat_done;
          if (CX == 0) goto repeat_done;
          goto repeat_not_done;
          }
        }
      else { // normal repeat, no concern for ZF
        if (i.as_32) {
          if (ECX != 0) {
            i.execute(&i);
            ECX -= 1;
            }
          if (ECX == 0) goto repeat_done;
          goto repeat_not_done;
          }
        else { // 16bit addrsize
          if (CX != 0) {
            i.execute(&i);
            CX -= 1;
            }
          if (CX == 0) goto repeat_done;
          goto repeat_not_done;
          }
        }
      // shouldn't get here from above
repeat_not_done:

//      BX_TICK1_IF_SINGLE_PROCESSOR();


repeat_done:
      bx_cpu. eip += i.ilen;
      }
    else {
      // non repeating instruction
      bx_cpu. eip += i.ilen;
      i.execute(&i);
      }

    bx_cpu. prev_eip = EIP; // commit new EIP
    bx_cpu. prev_esp = ESP; // commit new ESP

//    BX_TICK1_IF_SINGLE_PROCESSOR();

//debugger_check:

    goto main_cpu_loop;
    }
  else {
    unsigned remain, j;
static Bit8u FetchBuffer[16];
    Bit8u *temp_ptr;

    // read all leftover bytes in current page
    for (j=0; j<bx_cpu. bytesleft; j++) {
      FetchBuffer[j] = *fetch_ptr++;
      }

    // get remaining bytes for prefetch in next page
    // prefetch() needs eip current
    bx_cpu. eip += bx_cpu. bytesleft;
    remain = bx_cpu. bytesleft;
    prefetch();

    if (bx_cpu. bytesleft < 16) {
      // make sure (bytesleft - remain) below doesn't go negative
      BX_PANIC(("fetch_decode: bytesleft==0 after prefetch"));
      }
    temp_ptr = fetch_ptr = bx_cpu. fetch_ptr;

    // read leftover bytes in next page
    for (; j<16; j++) {
      FetchBuffer[j] = *temp_ptr++;
      }
    ret = FetchDecode(FetchBuffer, &i, 16, is_32);
    if (ret==0)
      BX_PANIC(("fetchdecode: cross boundary: ret==0"));
    if (i.ResolveModrm) {
      i.ResolveModrm(&i);
      }
    remain = i.ilen - remain;

    // note: eip has already been advanced to beginning of page
    bx_cpu. fetch_ptr = fetch_ptr + remain;
    bx_cpu. bytesleft -= remain;
    //bx_cpu. eip += remain;
    bx_cpu. eip = bx_cpu. prev_eip;
    goto fetch_decode_OK;
    }



  //
  // This area is where we process special conditions and events.
  //

handle_async_event:

  if (bx_cpu. debug_trap & 0x80000000) {
    // I made up the bitmask above to mean HALT state.
    bx_cpu. debug_trap = 0; // clear traps for after resume
    bx_cpu. inhibit_mask = 0; // clear inhibits for after resume
    // for one processor, pass the time as quickly as possible until
    // an interrupt wakes up the CPU.
    while (1) {
      if (bx_cpu. INTR && bx_cpu. eflags.if_) {
        break;
        }
//      BX_TICK1();
    }
  }


  //// Priority 1: Hardware Reset and Machine Checks
  ////   RESET
  ////   Machine Check
  //// (bochs doesn't support these)

  //// Priority 2: Trap on Task Switch
  ////   T flag in TSS is set
  //if (bx_cpu. debug_trap & 0x00008000) {
  //  bx_cpu. dr6 |= bx_cpu. debug_trap;
  //  exception(BX_DB_EXCEPTION, 0, 0); // no error, not interrupt
  //  }

  //// Priority 3: External Hardware Interventions
  ////   FLUSH
  ////   STOPCLK
  ////   SMI
  ////   INIT
  //// (bochs doesn't support these)

  //// Priority 4: Traps on Previous Instruction
  ////   Breakpoints
  ////   Debug Trap Exceptions (TF flag set or data/IO breakpoint)
  //if ( bx_cpu. debug_trap &&
  //     !(bx_cpu. inhibit_mask & BX_INHIBIT_DEBUG) ) {
  //  // A trap may be inhibited on this boundary due to an instruction
  //  // which loaded SS.  If so we clear the inhibit_mask below
  //  // and don't execute this code until the next boundary.
  //  // Commit debug events to DR6
  //  bx_cpu. dr6 |= bx_cpu. debug_trap;
  //  exception(BX_DB_EXCEPTION, 0, 0); // no error, not interrupt
  //  }

  //// Priority 5: External Interrupts
  ////   NMI Interrupts
  ////   Maskable Hardware Interrupts
  //if (bx_cpu. inhibit_mask & BX_INHIBIT_INTERRUPTS) {
  //  // Processing external interrupts is inhibited on this
  //  // boundary because of certain instructions like STI.
  //  // inhibit_mask is cleared below, in which case we will have
  //  // an opportunity to check interrupts on the next instruction
  //  // boundary.
  //  }
  //else if (bx_cpu. INTR && bx_cpu. eflags.if_ && BX_DBG_ASYNC_INTR) {
  //  Bit8u vector;

  //  // NOTE: similar code in ::take_irq()
  //  // if no local APIC, always acknowledge the PIC.
  //  vector = BX_IAC(); // may set INTR with next interrupt
  //  //BX_DEBUG(("decode: interrupt %u",
  //  //                                   (unsigned) vector));
  //  bx_cpu. errorno = 0;
  //  bx_cpu. EXT   = 1; /* external event */
  //  interrupt(vector, 0, 0, 0);
  //  BX_INSTR_HWINTERRUPT(vector, bx_cpu. sregs[BX_SEG_REG_CS].selector.value, bx_cpu. eip);
  //  }
  //else if (BX_HRQ && BX_DBG_ASYNC_DMA) {
  //  // NOTE: similar code in ::take_dma()
  //  // assert Hold Acknowledge (HLDA) and go into a bus hold state
  //  BX_RAISE_HLDA();
  //  }

  // Priority 6: Faults from fetching next instruction
  //   Code breakpoint fault
  //   Code segment limit violation (priority 7 on 486/Pentium)
  //   Code page fault (priority 7 on 486/Pentium)
  // (handled in main decode loop)

  // Priority 7: Faults from decoding next instruction
  //   Instruction length > 15 bytes
  //   Illegal opcode
  //   Coprocessor not available
  // (handled in main decode loop etc)

  // Priority 8: Faults on executing an instruction
  //   Floating point execution
  //   Overflow
  //   Bound error
  //   Invalid TSS
  //   Segment not present
  //   Stack fault
  //   General protection
  //   Data page fault
  //   Alignment check
  // (handled by rest of the code)


  if (bx_cpu. eflags.tf) {
    // TF is set before execution of next instruction.  Schedule
    // a debug trap (#DB) after execution.  After completion of
    // next instruction, the code above will invoke the trap.
    bx_cpu. debug_trap |= 0x00004000; // BS flag in DR6
    }

  if ( !(bx_cpu. INTR ||
         bx_cpu. debug_trap ||
//         BX_HRQ ||
         bx_cpu. eflags.tf) )
    bx_cpu. async_event = 0;
  goto async_events_processed;
}
#endif  // #if BX_DYNAMIC_TRANSLATION == 0




// boundaries of consideration:
//
//  * physical memory boundary: 1024k (1Megabyte) (increments of...)
//  * A20 boundary:             1024k (1Megabyte)
//  * page boundary:            4k
//  * ROM boundary:             2k (dont care since we are only reading)
//  * segment boundary:         any



  void
bx_cpu_c::prefetch(void)
{
  // cs:eIP
  // prefetch QSIZE byte quantity aligned on corresponding boundary
  Bit32u new_linear_addr;
  Bit32u new_phy_addr;
  Bit32u temp_eip, temp_limit;

  temp_eip   = bx_cpu. eip;
  temp_limit = bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.limit_scaled;

  new_linear_addr = bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.base + temp_eip;
  bx_cpu. prev_linear_page = new_linear_addr & 0xfffff000;
  if (temp_eip > temp_limit) {
    BX_PANIC(("prefetch: EIP > CS.limit"));
    }

    {
    new_phy_addr = A20ADDR(new_linear_addr);
    }

  if ( new_phy_addr >= bx_cpu. mem->len ) {
    // don't take this out if dynamic translation enabled,
    // otherwise you must make a check to see if bytesleft is 0 after
    // a call to prefetch() in the dynamic code.
    BX_ERROR(("prefetch: running in bogus memory"));
    }

  // max physical address as confined by page boundary
  bx_cpu. prev_phy_page = new_phy_addr & 0xfffff000;
  bx_cpu. max_phy_addr = bx_cpu. prev_phy_page | 0x00000fff;

  // check if segment boundary comes into play
  //if ((temp_limit - temp_eip) < 4096) {
  //  }

  bx_cpu. bytesleft = (bx_cpu. max_phy_addr - new_phy_addr) + 1;
  bx_cpu. fetch_ptr = bx_cpu. mem->vector(new_phy_addr);
}


  // If control has transfered locally, it is possible the prefetch Q is
  // still valid.  This would happen for repeat instructions, and small
  // branches.
  void
bx_cpu_c::revalidate_prefetch_q(void)
{
  Bit32u new_linear_addr, new_linear_page, new_linear_offset;
  Bit32u new_phy_addr;

  new_linear_addr = bx_cpu. sregs[BX_SEG_REG_CS].cache.u.segment.base + bx_cpu. eip;

  new_linear_page = new_linear_addr & 0xfffff000;
  if (new_linear_page == bx_cpu. prev_linear_page) {
    // same linear address, old linear->physical translation valid
    new_linear_offset = new_linear_addr & 0x00000fff;
    new_phy_addr = bx_cpu. prev_phy_page | new_linear_offset;
    bx_cpu. bytesleft = (bx_cpu. max_phy_addr - new_phy_addr) + 1;
    bx_cpu. fetch_ptr = bx_cpu. mem->vector(new_phy_addr);
    }
  else {
    bx_cpu. bytesleft = 0; // invalidate prefetch Q
    }
}

  void
bx_cpu_c::invalidate_prefetch_q(void)
{
  bx_cpu. bytesleft = 0;
}


