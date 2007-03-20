/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_list.c: Linked lists.
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

#ifdef NOTYET
static mowgli_heap_t *mowgli_node_heap;
#endif

void mowgli_mowgli_node_init(void)
{
#ifdef NOTYET
        mowgli_node_heap = mowgli_heap_create(sizeof(mowgli_node_t), 256);

	if (mowgli_node_heap == NULL)
	{
		mowgli_log("heap allocator failure.");
		exit(EXIT_FAILURE);
	}
#endif
}

/* creates a new node */
mowgli_node_t *mowgli_node_create(void)
{
        mowgli_node_t *n;

        /* allocate it */
#ifdef NOTYET
        n = mowgli_heap_alloc(mowgli_node_heap);
#else
	n = mowgli_alloc(sizeof(mowgli_node_t));
#endif

        /* initialize */
        n->next = n->prev = n->data = NULL;

        /* return a pointer to the new node */
        return n;
}

/* frees a node */
void mowgli_node_free(mowgli_node_t *n)
{
	return_if_fail(n != NULL);

        /* free it */
#ifdef NOTYET
        mowgli_heap_release(mowgli_node_heap, n);
#else
	mowgli_free(n);
#endif
}

/* adds a node to the end of a list */
void mowgli_node_add(void *data, mowgli_node_t *n, mowgli_list_t *l)
{
	mowgli_node_t *tn;

	return_if_fail(n != NULL);
	return_if_fail(l != NULL);

	n->next = n->prev = NULL;
	n->data = data;

	/* first node? */
	if (l->head == NULL)
	{
		l->head = n;
		l->tail = n;
		l->count++;
		return;
	}

	/* use the cached tail. */
	tn = l->tail;

	/* set the our `prev' to the last node */
	n->prev = tn;

	/* set the last node's `next' to us */
	n->prev->next = n;

	/* set the list's `tail' to us */
	l->tail = n;

	/* up the count */
	l->count++;
}

/* adds a node to the head of a list */
void mowgli_node_add_head(void *data, mowgli_node_t *n, mowgli_list_t *l)
{
	mowgli_node_t *tn;

	return_if_fail(n != NULL);
	return_if_fail(l != NULL);

	n->next = n->prev = NULL;
	n->data = data;

	/* first node? */
	if (!l->head)
	{
		l->head = n;
		l->tail = n;
		l->count++;
		return;
	}

	tn = l->head;
	n->next = tn;
	tn->prev = n;
	l->head = n;
	l->count++;
}

/* adds a node to a list before another node, or to the end */
void mowgli_node_add_before(void *data, mowgli_node_t *n, mowgli_list_t *l, mowgli_node_t *before)
{
	return_if_fail(n != NULL);
	return_if_fail(l != NULL);

	if (before == NULL)
		mowgli_node_add(data, n, l);
	else if (before == l->head)
		mowgli_node_add_head(data, n, l);
	else
	{
		n->data = data;
		n->prev = before->prev;
		n->next = before;
		before->prev = n;
		l->count++;
	}
}

void mowgli_node_del(mowgli_node_t *n, mowgli_list_t *l)
{
	return_if_fail(n != NULL);
	return_if_fail(l != NULL);

        /* are we the head? */
        if (!n->prev)
                l->head = n->next;
        else
                n->prev->next = n->next;

        /* are we the tail? */
        if (!n->next)
                l->tail = n->prev;
        else
                n->next->prev = n->prev;

        /* down the count */
        l->count--;
}

/* finds a node by `data' */
mowgli_node_t *mowgli_node_find(void *data, mowgli_list_t *l)
{
	mowgli_node_t *n;

	return_val_if_fail(l != NULL, NULL);

	MOWGLI_LIST_FOREACH(n, l->head) if (n->data == data)
		return n;

	return NULL;
}

void mowgli_node_move(mowgli_node_t *m, mowgli_list_t *oldlist, mowgli_list_t *newlist)
{
	return_if_fail(m != NULL);
	return_if_fail(oldlist != NULL);
	return_if_fail(newlist != NULL);

        /* Assumption: If m->next == NULL, then list->tail == m
         *      and:   If m->prev == NULL, then list->head == m
         */
        if (m->next)
                m->next->prev = m->prev;
        else
                oldlist->tail = m->prev;

        if (m->prev)
                m->prev->next = m->next;
        else
                oldlist->head = m->next;

        m->prev = NULL;
        m->next = newlist->head;
        if (newlist->head != NULL)
                newlist->head->prev = m;
        else if (newlist->tail == NULL)
                newlist->tail = m;
        newlist->head = m;

        oldlist->count--;
        newlist->count++;
}
