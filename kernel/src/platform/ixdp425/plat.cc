/********************************************************************* *                
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/ixdp425/plat.cc
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
 * $Id: plat.cc,v 1.2 2004/06/04 04:19:34 cvansch Exp $
 *                
 ********************************************************************/

#include INC_API(kernelinterface.h)
#include INC_GLUE(space.h)
#include INC_PLAT(console.h)
#include INC_CPU(timer.h)
#include INC_CPU(cpu.h)

// XXX FIXME
#define RAM_START    0x00000000
#define RAM_END      0x10000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map in the control registers */
    space->add_mapping((addr_t)IODEVICE_VADDR, (addr_t)XSCALE_DEV_PHYS,
		    pgent_t::size_64k, true, true, true);
}

/*
 * Setup platform memory descriptors
 */
extern "C" void SECTION(".init") init_kip_platform(void)
{
    /* PCI Data + Expansion Bus + Queue Manager */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)0x48000000, (addr_t)0x64000000);
    /* PCI Controller Config/Status + Expansion Bus Config/Status + IO Devices */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)0xC0000000, (addr_t)0xC800C000);
    /* SDRAM Config */
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)0xCC000000, (addr_t)0xCC000100);
}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
    /* physical memory map */
    get_kip()->memory_info.insert(memdesc_t::conventional, false,
		    (addr_t)RAM_START, (addr_t)RAM_END);
}

extern "C" void SECTION(".init") init_cpu(void)
{
    arm_cache::cache_invalidate_d();
    arm_cache::tlb_flush();
}
