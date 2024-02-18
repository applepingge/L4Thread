/********************************************************************* *                
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *                
 * File path:     platform/innovator/plat.cc
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
#include INC_CPU(io.h)
#include INC_PLAT(reg.h)

/* 192k Internal SRAM region. Should be in good use. */
#define SRAM_START		0x20000000
#define SRAM_END		0x20030000

/* SDRAM region */
#define SDRAM_START		0x10000000
#define SDRAM_END		0x12000000

/* System Reserved Regions, shouldn't be used for any reason. */
#define RESERVED_1_START	0x02000000
#define RESERVED_1_END		0x04000000
#define RESERVED_2_START	0x06000000
#define RESERVED_2_END		0x08000000
#define RESERVED_3_START	0x0A000000
#define RESERVED_3_END		0x0C000000
#define RESERVED_4_START	0x0E000000
#define RESERVED_4_END		0x10000000
#define RESERVED_5_START	0x14000000
#define RESERVED_5_END		0x20000000
#define RESERVED_6_START	0x20030000
#define RESERVED_6_END		0x30000000

/*
 * Initialize the platform specific mappings needed
 * to start the kernel.
 * Add other hardware initialization here as well
 */
extern "C" void SECTION(".init") init_platform(void)
{
    space_t *space = get_kernel_space();

    /* Map peripherals and control registers */
    space->add_mapping((addr_t)IODEVICE_VADDR, (addr_t)PHYS_CTL_REG_BASE,
		    pgent_t::size_1m, true, true, true);
}

/*
 * Setup platform memory descriptors
 */
extern "C" void SECTION(".init") init_kip_platform(void)
{
    /* insert all other region as dedicated memory*/
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
		    (addr_t)0, (addr_t)SDRAM_START);
    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
    			(addr_t)(SRAM_END + 1), (addr_t) 0xFFFFFFFF);

    /* insert system reserved memory, depicted from dedicated memory */
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_1_START, (addr_t)RESERVED_1_END);
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_2_START, (addr_t)RESERVED_2_END);
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_3_START, (addr_t)RESERVED_3_END);
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_4_START, (addr_t)RESERVED_4_END);
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_5_START, (addr_t)RESERVED_5_END);
    get_kip()->memory_info.insert(memdesc_t::reserved, false,
    			(addr_t)RESERVED_6_START, (addr_t)RESERVED_6_END);
}

extern "C" void SECTION(".init") init_cpu_mappings(void)
{
    /* insert SDRAM region*/
    get_kip()->memory_info.insert(memdesc_t::conventional, false,
			(addr_t)SDRAM_START, (addr_t)SDRAM_END);

    /* insert SRAM region*/
//    get_kip()->memory_info.insert(memdesc_t::dedicated, false,
//    			(addr_t)SRAM_START, (addr_t)SRAM_END);
			
}

extern "C" void SECTION(".init") init_cpu(void)
{
}
