/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:     glue/v4-arm/user.S
 * Description:   Kernel entry points for syscalls
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
 * $Id: user.S,v 1.13.2.1 2004/06/04 06:52:45 cvansch Exp $
 *
 ********************************************************************/

#include INC_ARCH(asm.h)
#include INC_ARCH(thread.h)
#include INC_GLUE(syscalls.h)

/* KIP could have 0xfe000000 + SYSCALL_##name in syscall link fields once 
 * fast-path is added.
 */

#define SYSCALL(name)				\
.section	.user.##name;			\
BEGIN_PROC(user_##name);			\
	mov	r12,	#0xfe000000;		\
	add	r12,	r12,	#SYSCALL_##name;\
	mov	pc,	r12;			\
END_PROC(user_##name)            

/* No fast-path, so write MRs back to UTCB. */

#define SYSCALL_MRS(name)			\
.section	.user.##name;			\
BEGIN_PROC(user_##name);			\
	mov	ip,	#0xff000000;		\
	ldr	ip,	[ip];			\
	add	ip,	ip,	#64;		\
	stmia	ip,	{r3-r7};		\
	str	ip,	[sp, #-4]!;		\
	mov	ip,	#0xfe000000;		\
	add	ip,	ip,	#SYSCALL_##name;\
	str	lr,	[sp, #-4]!;		\
	mov	lr,	pc;			\
	mov	pc,	ip;			\
	ldr	lr,	[sp],	#4;		\
	ldr	ip,	[sp],	#4;		\
	ldmia	ip,	{r3-r7};		\
	mov	pc,	lr;
END_PROC(user_##name)             

#define SYSCALL_UNIMPLEMENTED(name) \
.section    .user.##name;           \
BEGIN_PROC(user_##name);            \
        mov   pc, lr;               \
END_PROC(user_##name)

SYSCALL_MRS(ipc)
SYSCALL_MRS(lipc)
SYSCALL(thread_switch)
SYSCALL(space_control)
SYSCALL_MRS(unmap)
SYSCALL(system_clock)
SYSCALL(exchange_registers)
SYSCALL(thread_control)
SYSCALL(schedule)
SYSCALL(memory_control)

/* ProcessorControl currently unimplemented */

SYSCALL_UNIMPLEMENTED(processor_control)

