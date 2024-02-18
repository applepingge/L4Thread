/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/arm/plat.cc
 * Description:   
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
 * $Id: plat.cc,v 1.4 2004/06/04 08:06:05 htuch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_CPU(timer.h)

// XXX FIXME
#define RAM_B1_START    0xC0000000
#define RAM_B1_END      0xC1000000
#define RAM_B2_START    0xC8000000
#define RAM_B2_END      0xC9000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map the console */
    space->add_mapping((addr_t)CONSOLE_VADDR, (addr_t)CONSOLE_PADDR,
		    pgent_t::size_4k, true, true, true);

    /* Map the timer, interrupt, reset controllers */
    space->add_mapping((addr_t)SA1100_OS_TIMER_BASE, (addr_t)SA1100_TIMER_PHYS,
		    pgent_t::size_1m, true, true, true);
}

/*
 * Setup platform memory descriptors
 */
extern "C" void SECTION(".init") init_kip_platform(void)
{
    /* physical memory map */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)0, (addr_t)RAM_B1_START);
    get_kip()->dedicated_mem0.set((addr_t)0, (addr_t)RAM_B1_START);

    get_kip()->memory_info.insert(memdesc_t::conventional, false,
		    (addr_t)RAM_B1_START, (addr_t)RAM_B1_END);

    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)RAM_B1_END, (addr_t)RAM_B2_START);
    get_kip()->dedicated_mem1.set((addr_t)RAM_B1_END, (addr_t)RAM_B2_START);

    get_kip()->memory_info.insert(memdesc_t::conventional, false,
		    (addr_t)RAM_B2_START, (addr_t)RAM_B2_END);

    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)RAM_B2_END, (addr_t)0xFFFFFFFF);
    get_kip()->dedicated_mem2.set((addr_t)RAM_B2_END,
		    (addr_t)0xFFFFFFFF);

}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
    /* Map sa1100 zero bank */
    get_kernel_space()->add_mapping( (addr_t)ZERO_BANK_VADDR, (addr_t)0xE0000000,
		    pgent_t::size_1m, true, true );
}

extern "C" void init_cpu(void)
{
}
