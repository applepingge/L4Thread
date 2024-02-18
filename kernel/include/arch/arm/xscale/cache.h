/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arm/xscale/cache.h
 * Description:   Functions which manipulate the XScale cache
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
 * $Id: cache.h,v 1.2 2004/06/04 02:14:24 cvansch Exp $
 *
 ********************************************************************/

#ifndef __PLATFORM__ARM__XSCALE_CACHE_H_
#define __PLATFORM__ARM__XSCALE_CACHE_H_

#include <debug.h>
#include INC_CPU(syscon.h)

#define	    CACHE_SIZE	    32*1024
#define	    CACHE_LINE_SIZE 32


class arm_cache
{
public:

    static inline void cache_flush(void)
    {
	word_t target;

	/* Drain write buffer */
        __asm__ __volatile__ (
            "mcr  p15, 0, r0, c7, c10, 4 \n"
        );
	CPWAIT;

	for (target = VAR_AREA_START; target < (VAR_AREA_START+CACHE_SIZE); target += CACHE_LINE_SIZE)
	{
	    __asm__ __volatile__ (
		"mcr  p15, 0, %0, c7, c2, 5 \n"	    /* Allocate Line */
	    :: "r" (target));
	}
    }

    static inline void cache_flush_debug(void)
    {
        printf("About to cache flush... ");
        cache_flush();
        printf("done.\n");
    }

    static inline void tlb_flush(void)
    {
	/* Flush I&D TLB */
        __asm__ __volatile__ (
            "mcr    p15, 0, r0, c8, c7, 0    \n"
        ::); 
	CPWAIT;
    }

    static inline void tlb_flush_ent(addr_t vaddr, word_t log2size)
    {
	word_t a = (word_t)vaddr;

	for (word_t i=0; i < (1ul << log2size); i += ARM_PAGE_SIZE)
	{
	    __asm__ __volatile__ (
		"    mcr     p15, 0, %0, c8, c5, 1    \n"	/* Invalidate I TLB entry */
		"    mcr     p15, 0, %0, c8, c6, 1    \n"	/* Invalidate D TLB entry */
	    :: "r" (a));
	    a += ARM_PAGE_SIZE;
	}
    }

    static inline void tlb_flush_debug(void)
    {
        printf("About to TLB flush... ");
        tlb_flush();
        printf("done.\n");
    }

    static inline void cache_enable()
    {
        __asm__ __volatile__ (
            "mcr    p15, 0, r0, c7, c10, 4  \n"	// Drain the pending operations
            "mrc    p15, 0, r0, c1, c0, 0   \n"	// Get the control register
	    "orr    r0, r0, #0x1004	    \n"	// Set bit 12 - the I & D bit
            "mcr    p15, 0, r0, c1, c0, 0   \n"	// Set the control register
	    ::: "r0"
        );
	CPWAIT;
    }

    static inline void cache_invalidate_d_line(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c6, 1 \n"
        :: "r" (target));
	CPWAIT;
    }

    static inline void cache_invalidate_d()
    {
        __asm__ __volatile__ (
            "mcr p15, 0, r0, c7, c6, 0 \n"
        ::: "r0"
	);
	CPWAIT;
    }

    static inline void cache_clean(word_t target)
    {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c7, c10, 1 \n"
        :: "r" (target));
	CPWAIT;
    }
};

#endif /* __PLATFORM__ARM__XSCALE_CACHE_H_ */
