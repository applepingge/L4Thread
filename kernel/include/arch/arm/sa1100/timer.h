/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/sa1100/timer.h
 * Description:   Functions which manipulate the SA-1100 OS timer
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
 * $Id: timer.h,v 1.6 2004/06/04 02:14:23 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__SA1100__TIMER_H_
#define __ARCH__ARM__SA1100__TIMER_H_

#define SA1100_TIMER_PHYS	0x90000000ul

#define TIMER_MAX_RATE          3686400
#define TIMER_PERIOD            (TIMER_MAX_RATE/100)

#define SA1100_OS_TIMER_BASE    (IO_AREA_START+(ARM_SECTION_SIZE * 3))
#define SA1100_OS_TIMER_OSMR0   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x00))
#define SA1100_OS_TIMER_OSMR1   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x04))
#define SA1100_OS_TIMER_OSMR2   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x08))
#define SA1100_OS_TIMER_OSMR3   (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x0c))
#define SA1100_OS_TIMER_OSCR    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x10))
#define SA1100_OS_TIMER_OSSR    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x14))
#define SA1100_OS_TIMER_OWER    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x18))
#define SA1100_OS_TIMER_OIER    (*(volatile word_t *)(SA1100_OS_TIMER_BASE + 0x1c))

#endif /* __ARCH__ARM__SA1100__TIMER_H_*/
