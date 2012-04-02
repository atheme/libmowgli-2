/*
 * libmowgli: A collection of useful routines for programming.
 * coroutine_private.h: coroutines/fibers implementation
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

#ifndef __MOWGLI_COROUTINE_PRIVATE_H__
#define __MOWGLI_COROUTINE_PRIVATE_H__

#ifdef HAVE_UCONTEXT_H
#include <ucontext.h>

typedef ucontext_t mowgli_coroutine_basectx_t;
#else
#include <setjmp.h>

typedef jmp_buf mowgli_coroutine_basectx_t;
#endif

typedef struct {
	mowgli_coroutine_basectx_t base;
} mowgli_coroutine_ctx_t;

typedef struct {
	mowgli_coroutine_ctx_t ctx;
	mowgli_coroutine_t *callee;
	mowgli_coroutine_t *branch;
	mowgli_coroutine_start_fn_t start_fn;
	void *userdata;
} mowgli_coroutine_priv_t;

#define MOWGLI_COROUTINE_STACK_ALIGN	(256)
#define MOWGLI_COROUTINE_STACK_RESVSIZE ((sizeof(mowgli_coroutine_priv_t) + MOWGLI_COROUTINE_STACK_ALIGN - 1) & ~(MOWGLI_COROUTINE_STACK_ALIGN - 1))
#define MOWGLI_COROUTINE_STACK_DEFSIZE	(16 * MOWGLI_COROUTINE_STACK_ALIGN)

typedef struct {
	mowgli_coroutine_t *current;
	char stack[MOWGLI_COROUTINE_STACK_DEFSIZE];
} mowgli_coroutine_priv_ctx_t;

#endif
