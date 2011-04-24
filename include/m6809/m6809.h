/*
 * Copyright 2006-2010 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _ASM_6809_H
#define _ASM_6809_H

/**
 * \brief Various definitions/macros for the Motorola 6809 MPU.
 */

/* Basic properties of the CPU architecture */
#define BITS_PER_BYTE 8
#define BITS_PER_WORD 16

/* Defines for various bits in the condition code register */
#define CC_CARRY 		0x1
#define CC_OVERFLOW 	0x2
#define CC_ZERO 		0x4
#define CC_NEGATIVE 	0x8
#define CC_IRQ 		0x10
#define CC_HALF 		0x20
#define CC_FIRQ 		0x40
#define CC_E 			0x80

/* Defines for various assembler routines that can be called from C */
__attribute__((noreturn)) void start (void);
U8 far_read8 (const void *address, U8 page);
U16 far_read16 (const void *address, U8 page);
void *far_read_pointer (const void *address, U8 page);
typedef void (*void_function) (void);
void far_indirect_call_handler (void_function address, U8 page);
typedef U8 (*value_function) (void);
U8 far_indirect_call_value_handler (value_function address, U8 page);
void bitmap_blit_asm (U8 *dst, U8 shift);
U16 strlen (const char *);

/* Other externals */

extern void *_far_call_address;
extern U8 _far_call_page;
extern const U8 *bitmap_src;
extern U8 *bitmap_dst;

void slow_memset (U8 *dst, U8 val, U16 len);
void slow_memcpy (U8 *dst, const U8 *src, U16 len);

/* Give an alternate, global name to a function */
#define aka(name) asm (C_STRING(_) name C_STRING(::))

#define far_call_pointer(function, page, arg) \
do { \
	_far_call_address = (void *)function; \
	_far_call_page = page; \
	if (sizeof (arg) == 1) \
		asm (";short arg" :: "q" (arg)); \
	else \
		asm (";long arg" :: "a" (arg)); \
	asm ("jsr\t_far_call_pointer_handler"); \
} while (0);

void *malloc (U8 size);
void free (void *ptr);


extern inline void set_stack_pointer (const U16 s)
{
	asm __volatile__ ("lds\t%0" :: "g" (s) : "d");
}


extern inline U16 get_stack_pointer (void)
{
	U16 result;
	asm __volatile__ ("lea%0\t,s" : "=a" (result));
	return result;
}

extern inline void set_direct_page_pointer (const U8 dp)
{
	asm __volatile__ ("tfr\tb, dp" :: "q" (dp));
}


extern inline void m6809_andcc (const U8 bits)
{
	asm __volatile__ ("andcc\t%0" :: "i" (bits));
}

extern inline void m6809_orcc (const U8 bits)
{
	asm __volatile__ ("orcc\t%0" :: "i" (bits));
}

extern inline void m6809_firq_save_regs (void)
{
	asm __volatile__ ("pshs\td,x");
}

extern inline void m6809_firq_restore_regs (void)
{
	asm __volatile__ ("puls\td,x");
}

extern inline void __blockclear16 (void *s1, U16 n)
{
	register U16 *_s1 = (U16 *)s1;

	/* It is tempting to predivide n by 16, and then
	just decrement it inside the loop, but this does
	not actually help any. */
	do
	{
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		*_s1++ = 0UL;
		n -= 16;
	} while (n > 0);
}

extern inline void __blockcopy16 (void *s1, const void *s2, U16 n)
{
	register U16 *_s1 = (U16 *)s1;
	register U16 *_s2 = (U16 *)s2;

	/* It is tempting to predivide n by 16, and then
	just decrement it inside the loop, but this does
	not actually help any. */
	do
	{
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		*_s1++ = *_s2++;
		n -= 16;
	} while (n > 0);
}


/**
 * These C library functions are declared as GCC builtins, so that
 * GCC itself can replace the call with inline code if the amount
 * of data is small enough.  Else, it will resolve to a library
 * call.  For FreeWPC those C library functions are hand-coded in
 * assembler for speed.
 */
#define memset __builtin_memset
#define memcpy __builtin_memcpy
#define memcmp __builtin_memcmp

#endif /* _ASM_6809_H */
