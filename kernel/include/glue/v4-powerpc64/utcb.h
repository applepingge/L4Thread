/*********************************************************************
 *                
 * Copyright (C) 2003,  National ICT Australia (NICTA)
 *                
 * File path:     glue/v4-powerpc64/utcb.h
 * Description:   UTCB for PowerPC64
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
 ********************************************************************/
#ifndef __GLUE__V4_POWERPC__UTCB_H__
#define __GLUE__V4_POWERPC__UTCB_H__

#include INC_API(thread.h)
#include INC_API(types.h)

class utcb_t
{
public:
    threadid_t		my_global_id;		/* 0 */
    word_t		processor_no;		/* 8 */
    word_t		user_defined_handle;	/* 16 */
    threadid_t		pager;			/* 24 */
    threadid_t		exception_handler;	/* 32 */
    u8_t		preempt_flags;		/* 40 */
    u8_t		cop_flags;              /* 41 */
    u16_t		__reserved0;            /* 42 */
    u32_t               __reserved1;		/* 44 */
    timeout_t		xfer_timeout;		/* 48 */
    word_t		error_code;		/* 56 */
    threadid_t		intended_receiver;	/* 64 */
    threadid_t		virtual_sender;		/* 72 */
    word_t              __padding0[6];          /* 80 .. 128 */
    word_t		mr[IPC_NUM_MR];		/* 128 .. 640 */
    word_t		br[IPC_NUM_BR];		/* 640 .. 904 */
    word_t              __padding1[15];         /* 904 .. 1024 */
}; // __attribute__((packed));

#endif /* __GLUE__V4_POWERPC__UTCB_H__ */
