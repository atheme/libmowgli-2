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

#ifndef MOWGLI_SRC_LIBMOWGLI_CORE_BOOTSTRAP_INTERNAL_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_CORE_BOOTSTRAP_INTERNAL_H_INCLUDE_GUARD 1

extern void mowgli_allocation_policy_bootstrap(void);
extern void mowgli_allocator_bootstrap(void);
extern void mowgli_argstack_bootstrap(void);
extern void mowgli_bitvector_bootstrap(void);
extern void mowgli_cacheline_bootstrap(void);
extern void mowgli_global_storage_bootstrap(void);
extern void mowgli_hook_bootstrap(void);
extern void mowgli_interface_bootstrap(void);
extern void mowgli_log_bootstrap(void);
extern void mowgli_memslice_bootstrap(void);
extern void mowgli_node_bootstrap(void);
extern void mowgli_object_class_bootstrap(void);
extern void mowgli_queue_bootstrap(void);
extern void mowgli_random_bootstrap(void);

#ifdef _WIN32
extern void mowgli_winsock_bootstrap(void);
#endif

#endif /* MOWGLI_SRC_LIBMOWGLI_CORE_BOOTSTRAP_INTERNAL_H_INCLUDE_GUARD */
