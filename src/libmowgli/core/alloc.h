/*
 * libmowgli: A collection of useful routines for programming.
 * alloc.h: Safe, portable implementations of malloc, calloc, and free.
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

#ifndef MOWGLI_SRC_LIBMOWGLI_CORE_ALLOC_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_CORE_ALLOC_H_INCLUDE_GUARD 1

#include "core/allocation_policy.h"
#include "core/stdinc.h"
#include "platform/attributes.h"

extern void *mowgli_alloc_array_using_policy(mowgli_allocation_policy_t *policy, size_t size, size_t count)
    MOWGLI_FATTR_MALLOC MOWGLI_FATTR_ALLOC_SIZE_PRODUCT(2, 3);
extern void *mowgli_alloc_using_policy(mowgli_allocation_policy_t *policy, size_t size)
    MOWGLI_FATTR_MALLOC MOWGLI_FATTR_ALLOC_SIZE(2);
extern char *mowgli_strdup_using_policy(mowgli_allocation_policy_t *policy, const char *in)
    MOWGLI_FATTR_MALLOC;
extern char *mowgli_strndup_using_policy(mowgli_allocation_policy_t *policy, const char *in, size_t size)
    MOWGLI_FATTR_MALLOC;

extern void *mowgli_alloc_array(size_t size, size_t count)
    MOWGLI_FATTR_MALLOC MOWGLI_FATTR_ALLOC_SIZE_PRODUCT(1, 2);
extern void *mowgli_alloc(size_t size)
    MOWGLI_FATTR_MALLOC MOWGLI_FATTR_ALLOC_SIZE(1);
extern char *mowgli_strdup(const char *in)
    MOWGLI_FATTR_MALLOC;
extern char *mowgli_strndup(const char *in, size_t size)
    MOWGLI_FATTR_MALLOC;

extern void mowgli_free(void *ptr);

#endif /* MOWGLI_SRC_LIBMOWGLI_CORE_ALLOC_H_INCLUDE_GUARD */
