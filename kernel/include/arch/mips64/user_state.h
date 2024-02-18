/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *                
 * File path:     arch/mips64/user_state.h
 * Description:   MIPS64 user_state utility functions
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
 * $Id: user_state.h,v 1.6 2004/06/04 02:14:25 cvansch Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__MIPS64__USER_STATE_H__
#define __ARCH__MIPS64__USER_STATE_H__

#include INC_GLUE(context.h)
#include INC_ARCH(mips_cpu.h)

/* NB: This class is normally a pointer to a user object
 * remember to validate all accesses
 */

class user_state_t {
public:
    bool copy_to_tcb(tcb_t *tcb);
    bool copy_from_tcb(tcb_t *tcb);
    bool swap_with_tcb(tcb_t *tcb);

private:
    mips64_irq_context_t gp_regs;
    u64_t fpu_gprs[32]; /* 32 FPRs */
    u64_t fpu_fpcsr;    /* FPU control/status register */
};

static void cleanup_lazy_fpu( tcb_t * tcb )
{
    tcb->resources.mips64_fpu_spill( tcb );
}


bool user_state_t::copy_to_tcb(tcb_t *tcb)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;
    space_t *space = get_current_space();
    mips64_irq_context_t *context =
	    (mips64_irq_context_t *)((word_t)tcb + MIPS64_PAGE_SIZE) -1;

    user_state_t *dummy = this + 1;

    /* Validate user pointer */
    if ((!space->lookup_mapping ((addr_t)this, &pg, &pgsize)) || 
	(!space->lookup_mapping ((addr_t)((word_t)dummy - sizeof(word_t)), &pg, &pgsize)))
    {
	return false;
    }

    /* Copy state */

    word_t status = context->status;
    *context = this->gp_regs;
    /* Fix status register */
    context->status = status;

    word_t *tcb_fpu = (word_t*)&tcb->resources;
    cleanup_lazy_fpu(tcb);

    /* Hack to get fpu regs for now */
    for (word_t i = 0; i < 32; i ++)
	tcb_fpu[i] = this->fpu_gprs[i]; 
    tcb_fpu[32] = this->fpu_fpcsr;

    return true;
}

bool user_state_t::copy_from_tcb(tcb_t *tcb)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;
    space_t *space = get_current_space();
    mips64_irq_context_t *context =
	    (mips64_irq_context_t *)((word_t)tcb + MIPS64_PAGE_SIZE) -1;

    user_state_t *dummy = this + 1;

    /* Validate user pointer */
    if (!space->lookup_mapping ((addr_t)this, &pg, &pgsize) ||
	!pg->is_writable(space, pgsize)  ||
	!space->lookup_mapping ((addr_t)((word_t)dummy - sizeof(word_t)), &pg, &pgsize) ||
	!pg->is_writable(space, pgsize) )
    {
	return false;
    }

    /* Copy state */

    this->gp_regs = *context;

    word_t *tcb_fpu = (word_t*)&tcb->resources;
    cleanup_lazy_fpu(tcb);

    /* Hack to get fpu regs for now */
    for (word_t i = 0; i < 32; i ++)
	this->fpu_gprs[i] = tcb_fpu[i]; 
    this->fpu_fpcsr = tcb_fpu[32];

    return true;
}

bool user_state_t::swap_with_tcb(tcb_t *tcb)
{
    pgent_t * pg;
    pgent_t::pgsize_e pgsize;
    space_t *space = get_current_space();
    mips64_irq_context_t *context =
	    (mips64_irq_context_t *)((word_t)tcb + MIPS64_PAGE_SIZE) -1;

    user_state_t *dummy = this + 1;

    /* Validate user pointer */
    if (!space->lookup_mapping ((addr_t)this, &pg, &pgsize) ||
	!pg->is_writable(space, pgsize)  ||
	!space->lookup_mapping ((addr_t)((word_t)dummy - sizeof(word_t)), &pg, &pgsize) ||
	!pg->is_writable(space, pgsize) )
    {
	return false;
    }

    mips64_irq_context_t temp_gp_regs;
    u64_t temp_fpu_gprs[32];
    u64_t temp_fpu_fpcsr;

    /* Exchange state */

    temp_gp_regs = this->gp_regs;

    cleanup_lazy_fpu(tcb);

    /* Hack to get fpu regs for now */
    for (word_t i = 0; i < 32; i ++)
	temp_fpu_gprs[i] = this->fpu_gprs[i]; 
    temp_fpu_fpcsr = this->fpu_fpcsr;

    this->gp_regs = *context;

    word_t *tcb_fpu = (word_t*)&tcb->resources;
    /* Hack to get fpu regs for now */
    for (word_t i = 0; i < 32; i ++)
	this->fpu_gprs[i] = tcb_fpu[i]; 
    this->fpu_fpcsr = tcb_fpu[32];
    
    word_t status = context->status;
    *context = temp_gp_regs;
    /* Fix status register */
    context->status = status;

    /* Hack to get fpu regs for now */
    for (word_t i = 0; i < 32; i ++)
	tcb_fpu[i] = temp_fpu_gprs[i]; 
    tcb_fpu[32] = temp_fpu_fpcsr;

    return true;
}

#endif /* !__ARCH__MIPS64__USER_STATE_H__ */
