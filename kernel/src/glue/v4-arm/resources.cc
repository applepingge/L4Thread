/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/resource.cc
 * Description:   Dummy resources
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
 * $Id: resources.cc,v 1.3 2004/06/04 06:32:02 cvansch Exp $
 *
 ********************************************************************/

#include INC_API(tcb.h)
#include INC_GLUE(resource_functions.h)
#include <debug.h>

void thread_resources_t::save(tcb_t * tcb)
{
    UNIMPLEMENTED();
}

void thread_resources_t::load(tcb_t * tcb)
{
    UNIMPLEMENTED();
}
   
void thread_resources_t::purge(tcb_t * tcb)
{
    UNIMPLEMENTED();
}

void thread_resources_t::init(tcb_t * tcb)
{
    ASSERT(tcb);

    tcb->resource_bits = 0;
}

void thread_resources_t::free(tcb_t * tcb)
{
    //UNIMPLEMENTED();
}

