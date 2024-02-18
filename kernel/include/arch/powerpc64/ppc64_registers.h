/****************************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:	arch/powerpc64/ppc64_registers.h
 * Description:	SPR register encodings, and functions which misc PowerPC64
 * 		registers.
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: ppc64_registers.h,v 1.4 2004/06/04 02:14:26 cvansch Exp $
 *
 ***************************************************************************/

#ifndef __ARCH__POWERPC64__PPC64_REGISTERS_H__
#define __ARCH__POWERPC64__PPC64_REGISTERS_H__

/*  SPR encodings for mfspr
 */
#define SPR_XER		1
#define SPR_LR		8
#define SPR_CTR		9
#define SPR_DSISR	18
#define SPR_DAR		19
#define SPR_DEC		22
#define SPR_SDR1	25
#define SPR_SRR0	26
#define SPR_SRR1	27
#define SPR_SPRG0	272
#define SPR_SPRG1	273
#define SPR_SPRG2	274
#define SPR_SPRG3	275
#define SPR_ASR		280
#if 0
#define SPR_EAR		282
#endif
#define SPR_PVR		287
#define SPR_DABR	1013

#ifndef ASSEMBLY

INLINE word_t ppc64_get_sprg( const int which )
{
    word_t val;
    asm volatile( "mfsprg %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc64_set_sprg( const int which, word_t val )
{
    asm volatile( "mtsprg %0, %1" : : "i" (which), "r" (val) );
}

INLINE word_t ppc64_get_spr( const int which )
{
    word_t val;
    asm volatile( "mfspr %0, %1" : "=r" (val) : "i" (which) );
    return val;
}

INLINE void ppc64_set_spr( const int which, word_t val )
{
    asm volatile( "mtspr %0, %1" : : "i" (which), "r" (val) );
}

#if 0
INLINE void ppc_set_srr0( word_t val )
	{ asm volatile( "mtsrr0 %0" : : "r" (val) ); }

INLINE void ppc_set_srr1( word_t val )
	{ asm volatile( "mtsrr1 %0" : : "r" (val) ); }

INLINE word_t ppc_get_srr0( void )
{
	word_t val;
	asm volatile( "mfsrr0 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_srr1( void )
{
	word_t val;
	asm volatile( "mfsrr1 %0" : "=r" (val) );
	return val;
}

INLINE word_t ppc_get_sdr1( void )
{
    word_t val;
    asm volatile( "mfsdr1 %0" : "=r" (val) );
    return val;
}

INLINE void ppc_set_sdr1( word_t val )
{
    asm volatile( "mtsdr1 %0" : : "r" (val) );
}

INLINE word_t ppc_get_dar( void )
{
    word_t val;
    asm volatile( "mfspr %0, 19" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dsisr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 18" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_dabr( void )
{
    word_t val;
    asm volatile( "mfspr %0, 1013" : "=r" (val) );
    return val;
}

INLINE word_t ppc_get_ear( void )
{
    word_t val;
    asm volatile( "mfspr %0, 282" : "=r" (val) );
    return val;
}
#endif

INLINE word_t ppc64_get_dec( void )
{
    word_t val;
    asm volatile( "mfdec %0" : "=r" (val) );
    return val;
}

INLINE void ppc64_set_dec( word_t val )
{
    asm volatile( "mtdec %0" : : "r" (val) );
}

INLINE word_t ppc64_get_tb( void )
{
    word_t val;
    asm volatile( "mftb %0" : "=r" (val) );
    return val;
}

INLINE void ppc64_set_tbl( word_t val )
{
    asm volatile( "mttbl %0" : : "r" (val) );
}

INLINE void ppc64_set_tbu( word_t val )
{
    asm volatile( "mttbu %0" : : "r" (val) );
}


#endif	/* ASSEMBLY */

#endif	/* __ARCH__POWERPC64__PPC64_REGISTERS_H__ */

