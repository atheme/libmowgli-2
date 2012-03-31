/*
 * libmowgli: A collection of useful routines for programming.
 * coroutine.h: coroutines/fibers implementation
 *
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MOWGLI_COROUTINE_H__
#define __MOWGLI_COROUTINE_H__

typedef enum {
	MOWGLI_STACK_GROWS_DOWN = -1,
	MOWGLI_STACK_GROWS_UP = 1,
} mowgli_stack_dir_t;

extern mowgli_stack_dir_t mowgli_coroutine_get_stack_dir(void);

typedef void mowgli_coroutine_t;
typedef void (*mowgli_coroutine_start_fn_t)(void *userdata);

extern int mowgli_coroutine_create(mowgli_coroutine_start_fn_t start_fn, void *userdata);
extern void mowgli_coroutine_destroy(mowgli_coroutine_t *coroutine);
extern void mowgli_coroutine_yield(void);
extern void mowgli_coroutine_yield_to(mowgli_coroutine_t *next);
extern void mowgli_coroutine_exit(void);
extern void mowgli_coroutine_exit_to(mowgli_coroutine_t *next);
extern mowgli_coroutine_t *mowgli_coroutine_self(void);

#endif
