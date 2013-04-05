/*
 * Copyright 2009-2011 John Lindgren
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

#include <stdlib.h>
#include <string.h>
#include "mowgli.h"

struct mowgli_index_
{
	void **data;
	int count, size;
	int (*compare)(const void *a, const void *b, void *data);
	void *compare_data;
};

static mowgli_heap_t *index_heap = NULL;

void
mowgli_index_init(void)
{
	index_heap = mowgli_heap_create(sizeof(mowgli_index_t), 32, BH_NOW);
}

mowgli_index_t *
mowgli_index_create(void)
{
	mowgli_index_t *index = mowgli_heap_alloc(index_heap);

	index->data = NULL;
	index->count = 0;
	index->size = 0;
	index->compare = NULL;
	index->compare_data = NULL;

	return index;
}

void
mowgli_index_destroy(mowgli_index_t *index)
{
	mowgli_free(index->data);
	mowgli_heap_free(index_heap, index);
}

int
mowgli_index_count(mowgli_index_t *index)
{
	return index->count;
}

void
mowgli_index_allocate(mowgli_index_t *index, int size)
{
	size_t oldsize;
	void *new_ptr;

	if (size <= index->size)
		return;

	if (!index->size)
		index->size = 64;

	oldsize = index->size;

	while (size > index->size)
	{
		index->size <<= 1;
	}

	new_ptr = mowgli_alloc_array(sizeof(void *), index->size);

	if (index->data != NULL)
	{
		memcpy(new_ptr, index->data, oldsize);
		mowgli_free(index->data);
	}

	index->data = new_ptr;
}

void
mowgli_index_set(mowgli_index_t *index, int at, void *value)
{
	index->data[at] = value;
}

void *
mowgli_index_get(mowgli_index_t *index, int at)
{
	return index->data[at];
}

static void
make_room(mowgli_index_t *index, int at, int count)
{
	mowgli_index_allocate(index, index->count + count);

	if (at < index->count)
		memmove(index->data + at + count, index->data + at, sizeof(void *) *
			(index->count - at));

	index->count += count;
}

void
mowgli_index_insert(mowgli_index_t *index, int at, void *value)
{
	make_room(index, at, 1);
	index->data[at] = value;
}

void
mowgli_index_append(mowgli_index_t *index, void *value)
{
	mowgli_index_insert(index, index->count, value);
}

void
mowgli_index_copy_set(mowgli_index_t *source, int from, mowgli_index_t *target, int to, int count)
{
	memcpy(target->data + to, source->data + from, sizeof(void *) * count);
}

void
mowgli_index_copy_insert(mowgli_index_t *source, int from, mowgli_index_t *target, int to, int count)
{
	make_room(target, to, count);
	memcpy(target->data + to, source->data + from, sizeof(void *) * count);
}

void
mowgli_index_copy_append(mowgli_index_t *source, int from, mowgli_index_t *target, int count)
{
	mowgli_index_copy_insert(source, from, target, target->count, count);
}

void
mowgli_index_merge_insert(mowgli_index_t *first, int at, mowgli_index_t *second)
{
	mowgli_index_copy_insert(second, 0, first, at, second->count);
}

void
mowgli_index_merge_append(mowgli_index_t *first, mowgli_index_t *second)
{
	mowgli_index_copy_insert(second, 0, first, first->count, second->count);
}

void
mowgli_index_move(mowgli_index_t *index, int from, int to, int count)
{
	memmove(index->data + to, index->data + from, sizeof(void *) * count);
}

void
mowgli_index_delete(mowgli_index_t *index, int at, int count)
{
	index->count -= count;
	memmove(index->data + at, index->data + at + count, sizeof(void *) *
		(index->count - at));
}

void
mowgli_index_sort(mowgli_index_t *index, int (*compare)(const void *, const void *))
{
	qsort(index->data, index->count, sizeof(void *), compare);
}

#ifdef NOTYET
static int
mowgli_index_compare_with_data(const void *a, const void *b, void *_index)
{
	mowgli_index_t *index = _index;

	return index->compare(*(const void **) a, *(const void **) b,
			      index->compare_data);
}

void
mowgli_index_sort_with_data(mowgli_index_t *index, int(*compare)
			    (const void *a, const void *b, void *data), void *data)
{
	index->compare = compare;
	index->compare_data = data;
	g_qsort_with_data(index->data, index->count, sizeof(void *),
			  mowgli_index_compare_with_data, index);
	index->compare = NULL;
	index->compare_data = NULL;
}

#endif
