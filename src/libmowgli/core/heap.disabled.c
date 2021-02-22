/*
 * libmowgli: A collection of useful routines for programming.
 * heap.c: Heap allocation.
 *
 * Copyright (c) 2019 Aaron M. D. Jones <aaronmdjones@gmail.com>
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

struct mowgli_heap_
{
	mowgli_list_t blocks; // retained for ABI compatibility
	mowgli_allocation_policy_t *allocator;
	size_t elem_size;
};

mowgli_heap_t *
mowgli_heap_create_full(const size_t elem_size,
                        const size_t MOWGLI_VATTR_UNUSED mowgli_heap_elems,
                        const unsigned int MOWGLI_VATTR_UNUSED flags,
                        mowgli_allocation_policy_t *restrict allocator)
{
	return_null_if_fail(elem_size != 0);

	if (! allocator)
		allocator = mowgli_allocator_get_policy();

	return_null_if_fail(allocator != NULL);
	return_null_if_fail(allocator->allocate != NULL);

	mowgli_heap_t *const heap = allocator->allocate(sizeof *heap);

	if (! heap)
	{
#ifdef HEAP_DEBUG
		(void) mowgli_log_warning("cannot allocate memory for heap");
#endif
		return NULL;
	}

	(void) memset(heap, 0x00, sizeof *heap);

	heap->allocator = allocator;
	heap->elem_size = elem_size;

#ifdef HEAP_DEBUG
	(void) mowgli_log("pseudoheap@%p: created (elem_size %zu)", heap, elem_size);
#endif

	return heap;
}

mowgli_heap_t *
mowgli_heap_create(const size_t elem_size,
                   const size_t MOWGLI_VATTR_UNUSED mowgli_heap_elems,
                   const unsigned int MOWGLI_VATTR_UNUSED flags)
{
	return_null_if_fail(elem_size != 0);

	return mowgli_heap_create_full(elem_size, 0, 0, NULL);
}

void
mowgli_heap_destroy(mowgli_heap_t *const restrict heap)
{
	return_if_fail(heap != NULL);
	return_if_fail(heap->allocator != NULL);
	return_if_fail(heap->allocator->deallocate != NULL);

	(void) heap->allocator->deallocate(heap);

#ifdef HEAP_DEBUG
	(void) mowgli_log("pseudoheap@%p: destroyed (elem_size %zu)", heap, elem_size);
#endif
}

void *
mowgli_heap_alloc(mowgli_heap_t *const restrict heap)
{
	return_null_if_fail(heap != NULL);
	return_null_if_fail(heap->allocator != NULL);
	return_null_if_fail(heap->allocator->allocate != NULL);
	return_null_if_fail(heap->allocator->deallocate != NULL);

	void *ptr = heap->allocator->allocate(heap->elem_size);

	if (!ptr)
	{
#ifdef HEAP_DEBUG
		(void) mowgli_log_warning("pseudoheap@%p: cannot allocate memory for ptr", heap);
#endif
		return NULL;
	}

#ifdef HEAP_DEBUG
	(void) mowgli_log("pseudoheap@%p: allocated ptr %p (elem_size %zu)", heap, block->ptr, heap->elem_size);
#endif

	return memset(ptr, 0x00, heap->elem_size);
}

void
mowgli_heap_free(mowgli_heap_t *const restrict heap, void *const restrict ptr)
{
	return_if_fail(heap != NULL);
	return_if_fail(heap->allocator != NULL);
	return_if_fail(heap->allocator->deallocate != NULL);
	return_if_fail(ptr != NULL);

#ifdef HEAP_DEBUG
	(void) mowgli_log("pseudoheap@%p: freeing ptr %p (elem_size %zu)",
	                  heap, ptr, heap->elem_size);
#endif

	(void) heap->allocator->deallocate(ptr);
}
