/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/xscale/cpu.h
 * Description:   Intel XScale CPU control functions
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
 * $Id: cpu.h,v 1.2 2004/06/04 02:14:24 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__XSCALE__CPU_H_
#define __ARCH__ARM__XSCALE__CPU_H_

#define IODEVICE_VADDR		IO_AREA0_VADDR
#define XSCALE_DEV_PHYS		0xc8000000

class arm_cpu
{
public:
    static inline void cli(void)
    {
	__asm__  __volatile__ (
	    "        mrs     r0,     cpsr          \n"
	    "        orr     r0,     r0,     #0xd0 \n"
	    "        msr     cpsr_c, r0            \n"
	::: "r0");
    }

    static inline void sti(void)
    {
	__asm__  __volatile__ (
	    "        mrs     r0,     cpsr          \n"
	    "        and     r0,     r0,     #0x1f \n"
	    "        msr     cpsr_c, r0            \n"
	::: "r0");
    }

    static inline void sleep(void)
    {
	/* Can maybe execute idle instructions here to save power */
	for (int i = 0; i < 10000; ++i)
	    __asm__ __volatile__ ("nop");
    }
};

#endif /* __ARCH__ARM__XSCALE__CPU_H_ */
