/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>.
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

#ifdef HAVE_SELECT

# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# endif

typedef struct
{
	mowgli_list_t pollable_list;
} mowgli_select_eventloop_private_t;

static void
mowgli_select_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_select_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_select_eventloop_private_t));
	eventloop->poller = priv;

	return;
}

static void
mowgli_select_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n, *tn;
	mowgli_select_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
	{
		mowgli_node_delete(n, &priv->pollable_list);
	}

	mowgli_free(priv);
	return;
}

static void
mowgli_select_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_select_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;

	mowgli_node_delete(&pollable->node, &priv->pollable_list);
}

static void
mowgli_select_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_select_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;

# ifdef DEBUG
	mowgli_log("setselect %p fd %d func %p", pollable, pollable->fd, event_function);
# endif

	if (pollable->read_function || pollable->write_function)
		mowgli_node_delete(&pollable->node, &priv->pollable_list);

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:
		pollable->read_function = event_function;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		pollable->write_function = event_function;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	if (pollable->read_function || pollable->write_function)
		mowgli_node_add(pollable, &pollable->node, &priv->pollable_list);

	return;
}

static void
mowgli_select_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	mowgli_node_t *n, *tn;
	mowgli_eventloop_pollable_t *pollable;
	mowgli_select_eventloop_private_t *priv;
	mowgli_descriptor_t highest_fd = 0;
	fd_set rfds, wfds, efds;
	struct timeval tv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
	{
		mowgli_eventloop_pollable_t *pollable = n->data;

# ifdef DEBUG
		mowgli_log("considering fd %d pollable %p count %d", pollable->fd, pollable, priv->pollable_list.count);
# endif

		if (pollable->read_function || pollable->write_function)
		{
			if (pollable->fd > highest_fd)
				highest_fd = pollable->fd;

			if (pollable->read_function)
			{
				FD_SET(pollable->fd, &rfds);
				FD_SET(pollable->fd, &efds);
			}

			if (pollable->write_function)
			{
				FD_SET(pollable->fd, &wfds);
				FD_SET(pollable->fd, &efds);
			}
		}
	}

	tv.tv_sec = 1;

	if (select(highest_fd + 1, &rfds, &wfds, &efds, &tv) > 0)
	{
		mowgli_eventloop_synchronize(eventloop);

		/* iterate twice so we don't touch freed memory if a pollable is destroyed */
		MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
		{
			pollable = n->data;

			if ((FD_ISSET(pollable->fd, &rfds) || FD_ISSET(pollable->fd, &efds)))
			{
# ifdef DEBUG
				mowgli_log("run %p(%p, %p, MOWGLI_EVENTLOOP_IO_READ, %p)\n", pollable->read_function, eventloop, pollable, pollable->userdata);
# endif

				mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);
			}
		}

		MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
		{
			pollable = n->data;

			if ((FD_ISSET(pollable->fd, &wfds) || FD_ISSET(pollable->fd, &efds)))
			{
# ifdef DEBUG
				mowgli_log("run %p(%p, %p, MOWGLI_EVENTLOOP_IO_WRITE, %p)\n", pollable->write_function, eventloop, pollable, pollable->userdata);
# endif

				mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);
			}
		}
	}
}

mowgli_eventloop_ops_t _mowgli_select_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_select_eventloop_pollsetup,
	.pollshutdown = mowgli_select_eventloop_pollshutdown,
	.setselect = mowgli_select_eventloop_setselect,
	.select = mowgli_select_eventloop_select,
	.destroy = mowgli_select_eventloop_destroy,
};

#endif
