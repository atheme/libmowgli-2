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

mowgli_eventloop_pollable_t *
mowgli_pollable_create(mowgli_eventloop_t *eventloop, mowgli_descriptor_t fd, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable;

	return_val_if_fail(eventloop != NULL, NULL);

	if (pollable_heap == NULL)
		pollable_heap = mowgli_heap_create(sizeof(mowgli_eventloop_pollable_t), 16, BH_NOW);

	pollable = mowgli_heap_alloc(pollable_heap);

	pollable->eventloop = eventloop;
	pollable->type.type = MOWGLI_EVENTLOOP_TYPE_POLLABLE;
	pollable->fd = fd;
	pollable->userdata = userdata;

	return pollable;
}

void
mowgli_pollable_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	/* unregister any interest in the pollable. */
	eventloop->eventloop_ops->destroy(eventloop, pollable);

	mowgli_heap_free(pollable_heap, pollable);
}

void
mowgli_pollable_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);
	return_if_fail(eventloop->eventloop_ops != NULL);

	eventloop->eventloop_ops->setselect(eventloop, pollable, dir, event_function);
}

void
mowgli_pollable_set_nonblocking(mowgli_eventloop_pollable_t *pollable, bool nonblocking)
{
#if defined(HAVE_FCNTL)
	unsigned long flags;

	return_if_fail(pollable != NULL);

	flags = fcntl(pollable->fd, F_GETFL);

	if (nonblocking)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	fcntl(pollable->fd, F_SETFL, flags);
#elif defined(HAVE_WINSOCK2_H)
	u_long mode;

	return_if_fail(pollable != NULL);

	mode = nonblocking;

	ioctlsocket(pollable->fd, FIONBIO, &mode);
#endif
}

void
mowgli_pollable_trigger(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir)
{
	mowgli_eventloop_io_cb_t *event_function;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:
		event_function = pollable->read_function;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		event_function = pollable->write_function;
		break;
	default:
		event_function = NULL;
		return;
	}

	if (event_function == NULL)
		return;

	event_function(eventloop, pollable, dir, pollable->userdata);
}
