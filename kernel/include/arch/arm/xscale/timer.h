/*********************************************************************
 *
 * Copyright (C) 2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/xscale/timer.h
 * Description:   Functions which manipulate the XScale timer
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
 * $Id: timer.h,v 1.2 2004/06/04 02:14:24 cvansch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__XSCALE__TIMER_H_
#define __ARCH__ARM__XSCALE__TIMER_H_

#include INC_CPU(cpu.h)

/* Timer */
#define TIMER_OFFSET		0x5000

#define TIMER_MAX_RATE          66660000
#define TIMER_PERIOD            (TIMER_MAX_RATE/100)

#define XSCALE_TIMERS		(IODEVICE_VADDR + TIMER_OFFSET)

#define XSCALE_OS_TIMER		(*(volatile word_t *)(XSCALE_TIMERS + 0x04))
#define XSCALE_OS_TIMER_RELOAD	(*(volatile word_t *)(XSCALE_TIMERS + 0x08))
#define XSCALE_OS_TIMER_STATUS	(*(volatile word_t *)(XSCALE_TIMERS + 0x20))

#define XSCALE_WATCHDOG_TIMER   (*(volatile word_t *)(XSCALE_TIMERS + 0x14))
#define XSCALE_WATCHDOG_EN      (*(volatile word_t *)(XSCALE_TIMERS + 0x18))
#define XSCALE_WATCHDOG_KEY     (*(volatile word_t *)(XSCALE_TIMERS + 0x1c))

#endif /* __ARCH__ARM__XSCALE__TIMER_H_*/
