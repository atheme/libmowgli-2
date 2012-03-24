/*
 * libmowgli: A collection of useful routines for programming.
 * init.c: Initialization of libmowgli.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
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

MOWGLI_INITIALIZER_FUNC(mowgli_init_real)
{
	static bool initialized = 0;

	if (initialized)
		return;

#ifdef _WIN32
	extern void mowgli_winsock_bootstrap(void);

	mowgli_winsock_bootstrap();
#endif

	/* initial bootstrap */
	mowgli_node_init();
	mowgli_queue_init();
	mowgli_argstack_init();
	mowgli_bitvector_init();
	mowgli_global_storage_init();
	mowgli_hook_init();
	mowgli_random_init();
	mowgli_allocation_policy_init();
	mowgli_allocator_init();
	mowgli_memslice_init();

	/* now that we're bootstrapped, we can use a more optimised allocator
	   if one is available. */
	mowgli_allocator_set_policy(mowgli_allocator_malloc);

	initialized = true;
}

void mowgli_init(void)
{
	mowgli_log("mowgli_init() is a deprecated function, provided only for backwards compatibility with Mowgli-1.  You should remove it if you no longer support using Mowgli-1.");
}
