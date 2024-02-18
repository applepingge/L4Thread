/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     glue/v4-arm/timer.cc
 * Description:   Periodic timer handling
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
 * $Id: timer.cc,v 1.3 2004/06/04 08:06:05 htuch Exp $
 *
 ********************************************************************/

#include <debug.h>
#include INC_API(schedule.h)
#include INC_GLUE(timer.h)
#include INC_GLUE(intctrl.h)
#include INC_CPU(timer.h)

timer_t timer;

extern "C" void handle_timer_interrupt(word_t irq, arm_irq_context_t *context)
{
    get_current_scheduler()->handle_timer_interrupt();

    /* inaccurate */
    SA1100_OS_TIMER_OSSR |= (1 << 0);
    SA1100_OS_TIMER_OSCR = 0;
    SA1100_OS_TIMER_OSMR0 = TIMER_PERIOD;
}

void timer_t::init_global(void)
{
    UNIMPLEMENTED();
}

void timer_t::init_cpu(void)
{
    get_interrupt_ctrl()->register_interrupt_handler(SA1100_IRQ_OS_TIMER_0, 
            handle_timer_interrupt);

    SA1100_OS_TIMER_OSCR = 0;
    SA1100_OS_TIMER_OSMR0 = TIMER_PERIOD;
    SA1100_OS_TIMER_OIER = 0x00000001UL;

    get_interrupt_ctrl()->unmask(SA1100_IRQ_OS_TIMER_0);
}
