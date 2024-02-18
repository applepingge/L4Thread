/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/resource_functions.h
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
 * $Id: resource_functions.h,v 1.6 2004/06/04 07:39:10 htuch Exp $
 *
 ********************************************************************/


#ifndef __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__
#define __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__

#include INC_GLUE(resources.h)
#include INC_API(tcb.h)

/* FIXME - this is a naieve implementation of string IPC - it requires
 * the flushing of the data cache prior to the data cache synonym creation,
 * and after the operation completes. When using FASS, it is possible to
 * make this much more efficient, by doing a direct copy between the
 * address spaces when the source/destination sections are disjoint.
 */

/* FIXME - how to handle the situation when nested pagefault/string-copies
 * occur?
 */

/**
 * Enable a copy area.  
 *
 * @param tcb                   TCB of current thread
 * @param partner               TCB of partner thread
 * @param addr                  address within destination space
 *
 * @return an address into the copy area where kernel should perform
 * the IPC copy.
 */
INLINE addr_t thread_resources_t::enable_copy_area(tcb_t * tcb, tcb_t * partner,
        addr_t addr)
{
    word_t i;
    unsigned int start_section = (word_t)addr >> 20;

    tcb->resource_bits |= ARM_RESOURCE_COPYAREA; 

    arm_cache::cache_flush();
    arm_cache::tlb_flush();

    copy_dest_base = start_section << 20;

    for (i = 0; i < COPY_AREA_SECTIONS; ++i) {
        tcb->get_space()->pt.pd_split.copy_area[i] = 
                partner->get_space()->pt.pdir[start_section + i];
    }

    return addr_offset((addr_t)COPY_AREA_START, 
            (word_t) addr & (ARM_SECTION_SIZE - 1));
}

/**
 * Release all copy areas.
 *
 * @param tcb                   TCB of current thread
 * @param disable_copyarea      should copy area resource be disabled or not
 */
INLINE void thread_resources_t::release_copy_area (tcb_t * tcb,
                                                   bool disable_copyarea)
{
    if (tcb->resource_bits & ARM_RESOURCE_COPYAREA) {

        arm_cache::cache_flush();
    }
}

INLINE addr_t thread_resources_t::copy_area_real_address(addr_t addr)
{
    return (addr_t)((word_t)addr - COPY_AREA_START + copy_dest_base);
}



#endif /* __GLUE__V4_ARM__RESOURCE_FUNCTIONS_H__ */
