/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_memorypool.c: Memory pooling.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

/* visibility of this object is not available to the outside */
struct mowgli_memorypool_t_ {
	mowgli_list_t stack;
	mowgli_destructor_t destructor;
#ifdef NOTYET
	mowgli_mutex_t *mutex;
#endif
};

mowgli_memorypool_t *mowgli_memory_pool_new(void)
{
	mowgli_memorypool_t *pool;

	pool = mowgli_alloc_array(sizeof(mowgli_memorypool_t), 1);
	pool->destructor = mowgli_free;
#ifdef NOTYET
	pool->mutex = mowgli_mutex_new();
#endif
	return pool;
}

mowgli_memorypool_t *mowgli_memory_pool_with_custom_destructor(mowgli_destructor_t destructor)
{
	mowgli_memorypool_t *pool;

	pool = mowgli_alloc_array(sizeof(mowgli_memorypool_t), 1);
	pool->destructor = destructor;
#ifdef NOTYET
	pool->mutex = mowgli_mutex_new();
#endif
	return pool;
}

void *mowgli_memory_pool_add(mowgli_memorypool_t * pool, void * ptr)
{
#ifdef NOTYET
	mowgli_mutex_lock(pool->mutex);
#endif
	mowgli_node_add(ptr, mowgli_node_create(), &pool->stack);
#ifdef NOTYET
	mowgli_mutex_unlock(pool->mutex);
#endif
	return ptr;
}

void *
mowgli_memory_pool_allocate(mowgli_memorypool_t * pool, size_t sz)
{
	void * addr;

#ifdef NOTYET
	mowgli_mutex_lock(pool->mutex);
#endif
	addr = mowgli_alloc(sz);
	mowgli_node_add(addr, mowgli_node_create(), &pool->stack);
#ifdef NOTYET
	mowgli_mutex_unlock(pool->mutex);
#endif
	return addr;
}

void
mowgli_memory_pool_release(mowgli_memorypool_t * pool, void * addr)
{
	mowgli_node_t *n;
#ifdef NOTYET
	mowgli_mutex_lock(pool->mutex);
#endif
	n = mowgli_node_find(addr, &pool->stack);
	mowgli_node_delete(n, &pool->stack);
	pool->destructor(addr);
#ifdef NOTYET
	g_mutex_unlock(pool->mutex);
#endif
}

static void
mowgli_memory_pool_cleanup_nolock(mowgli_memorypool_t * pool)
{
	mowgli_node_t *n, *tn;

	MOWGLI_LIST_FOREACH_SAFE(n, tn, pool->stack.head)
	{
		mowgli_log("mowgli_memorypool_t<%p> element at %p was not released until cleanup!", pool, n->data);
		pool->destructor(n->data);
		mowgli_node_delete(n, &pool->stack);
	}
}

void
mowgli_memory_pool_cleanup(mowgli_memorypool_t * pool)
{
#ifdef NOTYET
	mowgli_mutex_lock(pool->mutex);
#endif
	mowgli_memory_pool_cleanup_nolock(pool);
#ifdef NOTYET
	mowgli_mutex_unlock(pool->mutex);
#endif
}

void
mowgli_memory_pool_destroy(mowgli_memorypool_t * pool)
{
#ifdef NOTYET
	mowgli_mutex_lock(pool->mutex);
#endif

	mowgli_memory_pool_cleanup_nolock(pool);

#ifdef NOTYET
	mowgli_mutex_unlock(pool->mutex);

	mowgli_mutex_free(pool->mutex);
#endif

	mowgli_free(pool);
}

char *
mowgli_memory_pool_strdup(mowgli_memorypool_t * pool, char * src)
{
	char *out;
	size_t sz = strlen(src) + 1;

	out = mowgli_memory_pool_allocate(pool, sz);
	strncpy(out, src, sz);

	return out;
}
