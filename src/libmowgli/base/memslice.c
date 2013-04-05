/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>
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

static mowgli_list_t allocator_list;
static mowgli_heap_t *allocator_heap;

/*
 * Our slice allocation engine.
 */
typedef struct
{
	size_t size;

	mowgli_heap_t *heap;
	mowgli_node_t node;
} slice_alloc_t;

/*
 * Allocation tag.
 */
typedef struct
{
	slice_alloc_t *owner;
} slice_tag_t;

/*
 * Given a size_t, determine the closest power-of-two, which is larger.
 */
static inline size_t
nexthigher(size_t k)
{
	size_t i;

	k--;

	for (i = 1; i < sizeof(k) * 8; i <<= 1)
		k |= k >> i;

	return k + 1;
}

/*
 * Set up an allocator.
 */
static inline slice_alloc_t *
create_allocator(size_t k)
{
	slice_alloc_t *a;

	a = mowgli_heap_alloc(allocator_heap);
	mowgli_node_add(a, &a->node, &allocator_list);

	a->size = k;
	a->heap = mowgli_heap_create(k, 16, BH_LAZY);

	return a;
}

/*
 * Find an allocator which fits the requested allocation size.
 */
static inline slice_alloc_t *
find_or_create_allocator(size_t i)
{
	size_t k;
	mowgli_node_t *n;

	k = nexthigher(i);
	MOWGLI_ITER_FOREACH(n, allocator_list.head)
	{
		slice_alloc_t *a = n->data;

		if (a->size == k)
			return a;
	}

	return create_allocator(k);
}

/*
 * Allocate a slice of memory.
 */
static void *
memslice_alloc(size_t i)
{
	void *ptr;
	slice_alloc_t *alloc;
	size_t adj_size;

	adj_size = i + sizeof(slice_tag_t);
	alloc = find_or_create_allocator(adj_size);

	ptr = mowgli_heap_alloc(alloc->heap);
	((slice_tag_t *) ptr)->owner = alloc;

	return (char *) ptr + sizeof(slice_tag_t);
}

/*
 * Free a slice of memory.
 */
static void
memslice_free(void *ptr)
{
	slice_tag_t *tag;

	return_if_fail(ptr != NULL);

	tag = (void *) ((char *) ptr - sizeof(slice_tag_t));
	mowgli_heap_free(tag->owner->heap, tag);
}

/*
 * Initialize memslice.
 */
static mowgli_allocation_policy_t *memslice = NULL;

void
mowgli_memslice_bootstrap(void)
{
	allocator_heap = mowgli_heap_create(sizeof(slice_alloc_t), 16, BH_NOW);

	memslice = mowgli_allocation_policy_create("memslice", memslice_alloc, memslice_free);
}

mowgli_allocation_policy_t *
mowgli_memslice_get_policy(void)
{
	return memslice;
}
