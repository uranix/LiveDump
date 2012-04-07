/*---------------------------------------------------------------------------+
 |  fpu_system.h                                                             |
 |  $Id: fpu_system.h,v 1.3 2001/10/06 03:53:46 bdenney Exp $
 |                                                                           |
 | Copyright (C) 1992,1994,1997                                              |
 |                       W. Metzenthen, 22 Parker St, Ormond, Vic 3163,      |
 |                       Australia.  E-mail   billm@suburbia.net             |
 |                                                                           |
 +---------------------------------------------------------------------------*/

#ifndef _FPU_SYSTEM_H
#define _FPU_SYSTEM_H


#define VERIFY_READ 0
#define VERIFY_WRITE 1

#ifndef NULL
#ifdef __cplusplus
#define NULL (0)
#else
#define NULL ((void *)0)
#endif
#endif

/* -----------------------------------------------------------
 * Slimmed down version used to compile against a CPU simulator
 * rather than a kernel (ported by Kevin Lawton)
 * ------------------------------------------------------------ */

/* Get data sizes from config.h generated from simulator's
 * configure script
 */
#include "config.h"
typedef Bit8u  u8;
typedef Bit8s  s8;
typedef Bit16u u16;
typedef Bit16s s16;
typedef Bit32u u32;
typedef Bit32s s32;
typedef Bit64u u64;
typedef Bit64s s64;

/* bbd: include ported linux headers after config.h for GCC_ATTRIBUTE macro */
//#include <linux/kernel.h>
//#include <linux/mm.h>
//#include <asm/math_emu.h>
//#include <linux/types.h>

#ifndef WORDS_BIGENDIAN
#error "WORDS_BIGENDIAN not defined in config.h"
#elif WORDS_BIGENDIAN == 1
#define EMU_BIG_ENDIAN 1
#else
/* Nothing needed.  Lack of defining EMU_BIG_ENDIAN means
 * small endian
 */
#endif


extern unsigned fpu_get_user(void *ptr, unsigned len);
extern void fpu_put_user(unsigned val, void *ptr, unsigned len);

extern void fpu_verify_area(unsigned what, void *ptr, unsigned n);
extern void math_emulate_init(void);
extern unsigned fpu_get_ds(void);
extern void fpu_set_ax(u16);

#ifndef __ASSEMBLY__

struct info {
#ifdef BX_NO_EMPTY_STRUCTS
  unsigned char donotindexme;
#endif
  };

#define FPU_info ((struct info *) 0)


//
// Minimal i387 structure, pruned from the linux headers.  Only
// the fields which were necessary are included.
//
typedef struct {
  struct {
    s32      cwd;
    s32      swd;
    s32      twd;
    s32      fip;
    s32      fcs;
    s32      foo;
    s32      fos;
    u32      fill0; /* to make sure the following aligns on an 8byte boundary */
    u64      st_space[16];   /* 8*16 bytes per FP-reg (aligned) = 128 bytes */
    unsigned char   ftop;
    unsigned char   no_update;
    unsigned char   rm;
    unsigned char   alimit;
    } GCC_ATTRIBUTE((aligned(16), packed)) soft;
  } i387_t;

extern i387_t i387;


#endif

#define 	SIGILL   4
#define 	SIGSEGV   11
#define 	SIGFPE   8

#define I387     i387


#define SET_AX(val16)           fpu_set_ax(val16);

#define no_ip_update            (*(u_char *)&(I387.soft.no_update))
#define FPU_rm                  (*(u_char *)&(I387.soft.rm))


/* Number of bytes of data which can be legally accessed by the current
   instruction. This only needs to hold a number <= 108, so a byte will do. */
#define access_limit            (*(u_char *)&(I387.soft.alimit))

#define partial_status          (I387.soft.swd)
#define control_word            (I387.soft.cwd)
#define fpu_tag_word            (I387.soft.twd)
#define registers               (I387.soft.st_space)
#define top                     (I387.soft.ftop)

#define instruction_address     (*(struct address *)&I387.soft.fip)
#define operand_address         (*(struct address *)&I387.soft.foo)

#define FPU_verify_area(x,y,z)	fpu_verify_area(x,y,z)
#define FPU_get_user(x,y)       ((x) = fpu_get_user((y), sizeof(*(y))))
#define FPU_put_user(val,ptr)   fpu_put_user((val),(ptr),sizeof(*(ptr)))

#define FPU_DS  (fpu_get_ds())

// bbd: Change a pointer to an int, with type conversions that make it legal.
// First make it a void pointer, then convert to an integer of the same
// size as the pointer.  Otherwise, on machines with 64-bit pointers, 
// compilers complain when you typecast a 64-bit pointer into a 32-bit integer.
#define PTR2INT(x) ((bx_ptr_equiv_t)(void *)(x))

// bbd: Change an int to a pointer, with type conversions that make it legal.
// Same strategy as PTR2INT: change to bx_ptr_equiv_t which is an integer
// type of the same size as FPU_REG*.  Then the conversion to pointer
// is legal.
#define REGNO2PTR(x)		((FPU_REG*)((bx_ptr_equiv_t)(x)))

#endif
