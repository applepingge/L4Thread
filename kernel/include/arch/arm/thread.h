/*********************************************************************
 *
 * Copyright (C) 2003-2004,  National ICT Australia (NICTA)
 *
 * File path:     arch/arm/thread.h
 * Description:   Thread switch and interrupt stack frames
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
 * $Id: thread.h,v 1.14 2004/06/04 07:49:40 htuch Exp $
 *
 ********************************************************************/

#ifndef __ARCH__ARM__THREAD_H__
#define __ARCH__ARM__THREAD_H__

#define ARM_SWITCH_STACK_SIZE 36
#define ARM_IPC_STACK_SIZE    12
#define ARM_USER_FLAGS_MASK   0xf0000000UL


#if !defined(__ASSEMBLER__)

/* Update ARM_SWITCH_STACK_SIZE when making changes */
class arm_switch_stack_t {
    public:
	u32_t r4;   /* -0 - */
	u32_t r5;   /*  4   */
	u32_t r11;  /*  8   */
	u32_t lr;   /*  12  */
};
#endif

#define SS_R4	0
#define SS_R5	4
#define SS_R11	8
#define SS_LR	12

#if !defined(__ASSEMBLER__)


/* Must match #defines below */
class arm_irq_context_t {
    public:
        u32_t klr;    /* -0 - */
        u32_t pc;     /*  4   */
        u32_t cpsr;   /*  8   */
        u32_t r0;     /*  12  */
        u32_t r1;     /*  16  */
        u32_t r2;     /* -20- */
        u32_t r3;     /*  24  */
        u32_t r4;     /*  28  */
        u32_t r5;     /*  32  */
        u32_t r6;     /*  36  */
        u32_t r7;     /* -40- */
        u32_t r8;     /*  44  */
        u32_t r9;     /*  48  */
        u32_t r10;    /*  52  */
        u32_t r11;    /*  56  */
        u32_t r12;    /* -60- */
        u32_t sp;     /*  64  */
        u32_t lr;     /*  68  */
};
#endif

#define PT_KLR      0
#define PT_PC       4
#define PT_CPSR     8
#define PT_R0       12 
#define PT_R1       16
#define PT_R2       20
#define PT_R3       24
#define PT_R4       28
#define PT_R5       32
#define PT_R6       36
#define PT_R7       40
#define PT_R8       44
#define PT_R9       48
#define PT_R10      52
#define PT_R11      56
#define PT_R12      60
#define PT_SP       64
#define PT_LR       68

#define PT_SIZE     72

#define SAVE_ALL_INT                    \
    stmdb   sp,     {r0-r14}^;          \
    nop;                                \
    sub     sp,     sp,     #PT_SIZE;   \
    str     lr,     [sp, #PT_PC];       

#define SAVE_CPSR_MRS                   \
    mrs     lr,     spsr;               \
    str     lr,     [sp, #PT_CPSR];

#define SAVE_CPSR_TMP                   \
    ldr     lr,     tmp_spsr_abt;       \
    str     lr,     [sp, #PT_CPSR];


#define RESTORE_ALL                     \
    ldr     r0,     [sp, #PT_CPSR];     \
    msr     spsr,   r0;                 \
    ldr     lr,     [sp, #PT_PC];       \
    add     sp,     sp,     #PT_SIZE;   \
    ldmdb   sp,     {r0-r14}^;          \
    nop;

#ifdef CONFIG_ENABLE_FASS 
#define SET_KERNEL_DACR                \
        ldr     ip,     =0x55555555;   \
        mcr     p15, 0, ip, c3, c0;    \
        mov     ip,     #0;            \
        mcr     p15, 0, ip, c13, c0;
#else
#define SET_KERNEL_DACR
#endif

// dacr = (0x00000001 | (1 << (2 * current_domain)));
#ifdef CONFIG_ENABLE_FASS             
#define SET_USER_DACR                    \
        ldr     ip,     =current_domain; \
        ldr     ip,     [ip];            \
        mov     ip,     ip, lsl #1;      \
        mov     lr,     #1;              \
        orr     lr,     lr, lr, lsl ip;  \
        mcr     p15, 0, lr, c3, c0;      \
        ldr     ip,     =current_pid;    \
        ldr     ip,     [ip];            \
        mcr     p15, 0, ip, c13, c0;

/* If inside the kernel, don't modify DACR */
#define SET_USER_DACR_K                \
        ldr     ip,     [sp, #PT_CPSR];\
        and     ip,     ip,     #0x1f; \
        cmp     ip,     #0x10;         \
        bne     1f;                    \
        SET_USER_DACR                  \
1:  

#else
#define SET_USER_DACR
#define SET_USER_DACR_K
#endif

#endif /* __ARCH__ARM__THREAD_H__ */
