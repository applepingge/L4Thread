/*********************************************************************
 *
 * Copyright (C) 2003,  University of New South Wales
 *
 * File path:      glue/v4-arm/traps.S
 * Description:    Exception vectors
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
 * $Id: traps.S,v 1.19.2.1 2004/06/04 06:52:45 cvansch Exp $
 *
 ********************************************************************/

#include INC_ARCH(thread.h)
#include INC_GLUE(syscalls.h)
#include INC_ARCH(asm.h)

.balign 4096

/* Relative branches, loads and stores to locations outside this 4K page are
 * broken, as this is remapped to the high interrupt vector 0xFFFF0000
 */
BEGIN_PROC(arm_high_vector)
	b	arm_reset_exception
	b	arm_undefined_inst_exception
	b	arm_swi_exception 
	b	arm_l4_syscall
	b	arm_data_abort_exception
	nop
	b	arm_irq_exception
	b	arm_fiq_exception
END_PROC(arm_high_vector)

arm_common_return:
	SET_USER_DACR
	RESTORE_ALL
	movs	pc,	lr

arm_reset_exception:
	SAVE_ALL_INT
	SAVE_CPSR_MRS 
	SET_KERNEL_DACR

	/* Call C function reset_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=reset_exception
	ldr	lr,	=arm_common_return
	mov	pc,	r1

arm_undefined_inst_exception:
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function undefined_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=undefined_exception
	ldr	lr,	=arm_common_return
	mov	pc,	r1

arm_fiq_exception:
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function fiq_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=fiq_exception
	ldr	lr,	=arm_common_return
	mov	pc,	r1

arm_swi_exception:
	SAVE_ALL_INT
	SAVE_CPSR_MRS
	SET_KERNEL_DACR

	/* Call C function syscall_exception(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=syscall_exception
	ldr	lr,	=arm_common_return
	mov	pc,	r1
 
arm_l4_syscall:
       /* Save R14_abt, SPSR_abt */
        sub     lr,     lr,     #4
        str     lr,     tmp_r14_abt
        mrs     lr,     spsr
        str     lr,     tmp_spsr_abt

        /* Enter supervisor mode, IRQ/FIQ disabled */
        msr     cpsr_c, #0x000000d3

        /* since SAVE_ALL_INT only does user's banked lr */
        str     lr,     [sp, #(-PT_SIZE + PT_KLR)]

        ldr     lr,     tmp_r14_abt

        /* Test to see if it is a syscall */
        cmp     lr,     #0xfe000000 /* (HT) FIXME - need stricter test */
        bcc     arm_prefetch_abort_exception

	/* svc SP should point to current thread's stack in KTCB - this is
	* given as it is an invariant that the banked svc SP is to the
	* currently executing thread's KTCB stack
	*/

	/* Test to see if it is a syscall */
	and	lr,	lr,	#0x000000ff
	cmp	lr,	#SYSCALL_limit
	bhi	arm_non_l4_syscall

	SET_KERNEL_DACR

	/* It is a syscall, so save the user's banked SP and LR as well as
	 * CPSR
	 */

	ldr     r12,    tmp_spsr_abt
	stmdb   sp,     {r12, sp, lr}^
	nop
	sub     sp,     sp,     #ARM_IPC_STACK_SIZE

	/* Calling registers:
	 *   r0, r1, r2, r3, r4, r5, r6, r7 : arguments 1 - 8
	 * Retuned registers:
	 *   r0, r1, r2, r3, r4, r5, r6     : returned 1 - 7
	 */
	mov	r12,	lr
	adr	lr,	syscall_return
	ldr	pc,	[pc, r12 /* , lsl #2*/]
	nop

.word	sys_ipc
.word	sys_thread_switch
.word	sys_thread_control_exargs
.word	sys_exchange_registers_exargs
.word	sys_schedule_exargs 
.word	sys_unmap 
.word	sys_space_control
.word	syscall_return /* sys_processor_control */
.word	sys_memory_control_exargs 
.word   sys_clock
.word	0
.word   sys_ipc /* lipc */

#define SYS_EXARGS(name, lastreg, numregs)	\
name##_exargs:					\
	stmdb	sp!,	{r4##lastreg};		\
	ldr	r12,	=name##;		\
	mov	lr,	pc;			\
	mov	pc,	r12;			\
						\
	add	sp,	sp,     #(numregs * 4);	\
	b	syscall_return;			

SYS_EXARGS(sys_thread_control, , 1)
SYS_EXARGS(sys_exchange_registers,-r6,3)
SYS_EXARGS(sys_schedule, , 1)
SYS_EXARGS(sys_memory_control, , 1)

.global syscall_return
syscall_return:
	/* restore the user's banked SP, LR, CPSR */

	SET_USER_DACR

	ldmia	sp,	{r12, sp}^
	nop
	add	sp,	sp,	#ARM_IPC_STACK_SIZE
	ldr	lr,	[sp, #-4]
	msr	spsr,	r12

	movs	pc,	lr

arm_non_l4_syscall:
	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Call C function arm_swi(arm_irq_context_t *) */
	mov	r0,	sp
	ldr	r1,	=arm_misc_l4_syscall
	mov	lr,	pc
	mov	pc,	r1

	SET_USER_DACR
	RESTORE_ALL

	ldr	lr,	[sp, #(-PT_SIZE + PT_LR)]

	movs	pc,	lr

arm_prefetch_abort_exception:
	/* Even if the fault came from the kernel, it won't be on the current
	 * stack as KTCBs are faulted on for allocation prior to the use of
	 * their stacks 
	 */

	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Pointer to base of current arm_irq_context_t record */
	mov	r2,	sp

	/* Faulting address */
	ldr	r1,	[sp, #PT_PC]

	/* Assume it was a page fault for now */

	/* Fault status - not updated on prefetch abort */
	mov	r0,	#0

	mov	r3,	#0

	/* Process the page fault */
 
	ldr	r4,	=arm_page_fault
	ldr	lr,	=arm_abort_return
	mov	pc,	r4

arm_data_abort_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#8
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt

	/* Even if the fault came from the kernel, it won't be on the current
	 * stack as KTCBs are faulted on for allocation prior to the use of
	 * their stacks
	 */
	
	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	/* Pointer to base of current arm_irq_context_t record */
	mov	r2,	sp

	/* Fault status */
	mrc	p15, 0, r0, c5, c0, 0

	/* Faulting address */
	mrc	p15, 0, r1, c6, c0, 0

	mov	r3,	#1

	ldr     r4,	=arm_page_fault
	mov	lr,	pc
	mov	pc,	r4

.global arm_abort_return
arm_abort_return:
	SET_USER_DACR_K
	RESTORE_ALL

	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	ldr	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	/* Enter abort mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d7

	/* Restore R13_abt, SPSR_abt */
	ldr	lr,	tmp_spsr_abt
	msr	spsr,	lr
	ldr	lr,	tmp_r14_abt

	movs	pc,	r14

arm_irq_exception:
	/* Save R13_abt, SPSR_abt */
	sub	lr,	lr,	#4
	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	/* Enter supervisor mode, IRQ/FIQ disabled */
	msr	cpsr_c,	#0x000000d3

	/* since SAVE_ALL_INT only does user's banked lr */
	str	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	ldr	lr,	tmp_r14_abt

	SAVE_ALL_INT
	SAVE_CPSR_TMP
	SET_KERNEL_DACR

	mov	r0,	sp
       
	ldr	r4,	=arm_irq
	mov	lr,	pc
	mov	pc,	r4

	SET_USER_DACR_K
	RESTORE_ALL

	str	lr,	tmp_r14_abt
	mrs	lr,	spsr
	str	lr,	tmp_spsr_abt

	ldr	lr,	[sp, #(-PT_SIZE + PT_KLR)]

	/* Enter irq mode, IRQ/FIQ disabled */
	msr	cpsr_c, #0x000000d2

	/* Restore R13_abt, SPSR_abt */
	ldr	lr,	tmp_spsr_abt
	msr	spsr,	lr
	ldr	lr,	tmp_r14_abt

	movs	pc,	lr

.balign 32

.global utcb_dirty
utcb_dirty: 
	.word	0xdeadbeef

.global domain_dirty
domain_dirty:
	.word	0xdeadbeef

.global current_domain
current_domain:
	.word	0xdeadbeef

.global current_pid
current_pid:
	.word	0xdeadbeef

.balign 32

tmp_r14_abt:
	.word	0xdeadbeef
tmp_spsr_abt:
	.word	0xdeadbeef

.ltorg

.balign 4096

