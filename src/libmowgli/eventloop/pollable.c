/*
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
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

static mowgli_heap_t *pollable_heap = NULL;

mowgli_eventloop_pollable_t *mowgli_pollable_create(mowgli_eventloop_t *eventloop, mowgli_descriptor_t fd, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable;

	return_val_if_fail(eventloop != NULL, NULL);

	pollable = mowgli_heap_alloc(pollable_heap);

	pollable->fd = fd;
	pollable->userdata = userdata;

	return pollable;
}

void mowgli_pollable_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	/* unregister any interest in the pollable. */
	mowgli_pollable_setselect(eventloop, pollable, MOWGLI_EVENTLOOP_POLL_READ, NULL);
	mowgli_pollable_setselect(eventloop, pollable, MOWGLI_EVENTLOOP_POLL_WRITE, NULL);

	mowgli_heap_free(pollable_heap, pollable);
}

void mowgli_pollable_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, mowgli_pollevent_dispatch_func_t *event_function)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);
	return_if_fail(eventloop->eventloop_ops != NULL);

	eventloop->eventloop_ops->setselect(eventloop, pollable, dir, event_function);
}
