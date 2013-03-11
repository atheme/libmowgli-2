/*
 * libmowgli: A collection of useful routines for programming.
 * bootstrap.c: Initialization of libmowgli.
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

extern void mowgli_log_bootstrap(void);
extern void mowgli_node_bootstrap(void);
extern void mowgli_queue_bootstrap(void);
extern void mowgli_object_class_bootstrap(void);
extern void mowgli_argstack_bootstrap(void);
extern void mowgli_bitvector_bootstrap(void);
extern void mowgli_global_storage_bootstrap(void);
extern void mowgli_hook_bootstrap(void);
extern void mowgli_random_bootstrap(void);
extern void mowgli_allocation_policy_bootstrap(void);
extern void mowgli_allocator_bootstrap(void);
extern void mowgli_memslice_bootstrap(void);
extern void mowgli_cacheline_bootstrap(void);

/* TODO: rename to mowgli_bootstrap next time there is a LIB_MAJOR bump */
MOWGLI_BOOTSTRAP_FUNC(mowgli_bootstrap_real)
{
	static bool bootstrapped = 0;

	if (bootstrapped)
		return;

	/* initial bootstrap */
	mowgli_log_bootstrap();
	mowgli_node_bootstrap();
	mowgli_queue_bootstrap();
	mowgli_object_class_bootstrap();
	mowgli_argstack_bootstrap();
	mowgli_bitvector_bootstrap();
	mowgli_global_storage_bootstrap();
	mowgli_hook_bootstrap();
	mowgli_random_bootstrap();
	mowgli_allocation_policy_bootstrap();
	mowgli_allocator_bootstrap();
	mowgli_memslice_bootstrap();
	mowgli_cacheline_bootstrap();

#ifdef _WIN32
	extern void mowgli_winsock_bootstrap(void);

	mowgli_winsock_bootstrap();
#endif

	/* now that we're bootstrapped, we can use a more optimised allocator
	   if one is available. */
	mowgli_allocator_set_policy(mowgli_allocator_malloc);

	bootstrapped = true;
}

void
mowgli_init(void)
{
	mowgli_log("mowgli_init() is a deprecated function, provided only for backwards compatibility with Mowgli-1.  You should remove it if you no longer support using Mowgli-1.");
}
