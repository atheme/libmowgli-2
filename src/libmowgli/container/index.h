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

#ifndef __MOWGLI_INDEX_H__
#define __MOWGLI_INDEX_H__

struct mowgli_index_;

typedef struct mowgli_index_ mowgli_index_t;

mowgli_index_t *mowgli_index_create(void);
void mowgli_index_destroy(mowgli_index_t *index);
int mowgli_index_count(mowgli_index_t *index);
void mowgli_index_allocate(mowgli_index_t *index, int size);
void mowgli_index_set(mowgli_index_t *index, int at, void *value);
void *mowgli_index_get(mowgli_index_t *index, int at);
void mowgli_index_insert(mowgli_index_t *index, int at, void *value);
void mowgli_index_append(mowgli_index_t *index, void *value);
void mowgli_index_copy_set(mowgli_index_t *source, int from, mowgli_index_t *target, int to, int count);
void mowgli_index_copy_insert(mowgli_index_t *source, int from, mowgli_index_t *target, int to, int count);
void mowgli_index_copy_append(mowgli_index_t *source, int from, mowgli_index_t *target, int count);
void mowgli_index_merge_insert(mowgli_index_t *first, int at, mowgli_index_t *second);
void mowgli_index_merge_append(mowgli_index_t *first, mowgli_index_t *second);
void mowgli_index_move(mowgli_index_t *index, int from, int to, int count);
void mowgli_index_delete(mowgli_index_t *index, int at, int count);
void mowgli_index_sort(mowgli_index_t *index, int (*compare)(const void *a, const void *b));
void mowgli_index_sort_with_data(mowgli_index_t *index, int(*compare)
				 (const void *a, const void *b, void *data), void *data);

#endif
