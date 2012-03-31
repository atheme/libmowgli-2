/*
 * libmowgli: A collection of useful routines for programming.
 * coroutine.c: coroutines/fibers implementation
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

#include "mowgli.h"
#include "coroutine_private.h"

static mowgli_stack_dir_t mowgli_coroutine_get_stack_dir_leaf_ptr(void *callstack_p)
{
	mowgli_stack_dir_t dir;
	void *childstack_p = &callstack_p;

	dir = (childstack_p > callstack_p ? MOWGLI_STACK_GROWS_UP : MOWGLI_STACK_GROWS_DOWN);

	return dir;
}

mowgli_stack_dir_t mowgli_coroutine_get_stack_dir(void)
{
	int dummy1 = 0;
	void *dummy2 = &dummy1;

	return mowgli_coroutine_get_stack_dir_leaf_ptr(dummy2);
}

