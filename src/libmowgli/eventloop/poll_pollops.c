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

#ifdef HAVE_POLL_H

#include <poll.h>

#ifndef POLLRDNORM
#define POLLRDNORM POLLIN
#endif
#ifndef POLLWRNORM
#define POLLWRNORM POLLOUT
#endif

typedef struct {
	struct pollfd pollfds[FD_SETSIZE];
	nfds_t nfds;
	mowgli_list_t pollable_list;
} mowgli_poll_eventloop_private_t;

static nfds_t update_poll_fds(mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n;
	mowgli_poll_eventloop_private_t *priv;
	nfds_t slot = 0;

	return_val_if_fail(eventloop != NULL, 0);

	priv = eventloop->poller;

        MOWGLI_ITER_FOREACH(n, priv->pollable_list.head)
        {
		mowgli_eventloop_pollable_t *pollable = n->data;

                if (pollable->read_function || pollable->write_function)
		{
			priv->pollfds[slot].fd = pollable->fd;

			if (pollable->read_function)
				priv->pollfds[slot].events |= POLLRDNORM;

			if (pollable->write_function)
				priv->pollfds[slot].events |= POLLWRNORM;

			priv->pollfds[slot].revents = 0;
			pollable->slot = slot;
                        slot++;
                }
                else
                        pollable->slot = -1;
        }

        return slot;
}

static void mowgli_poll_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_poll_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_poll_eventloop_private_t));
	eventloop->poller = priv;

	return;
}

static void mowgli_poll_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n, *tn;
	mowgli_poll_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
	{
		mowgli_node_delete(n, &priv->pollable_list);
	}

	mowgli_free(priv);
	return;
}

static void mowgli_poll_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, mowgli_pollevent_dispatch_func_t *event_function)
{
	switch (dir)
	{
	case MOWGLI_EVENTLOOP_POLL_READ:
		pollable->read_function = event_function;
		break;
	case MOWGLI_EVENTLOOP_POLL_WRITE:
		pollable->write_function = event_function;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

	return;
}

static void mowgli_poll_eventloop_select(mowgli_eventloop_t *eventloop, int time)
{
	mowgli_node_t *n, *tn;
	nfds_t nfds;
	mowgli_eventloop_pollable_t *pollable;
	mowgli_poll_eventloop_private_t *priv;
	int sr, slot;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	nfds = update_poll_fds(eventloop);

	if ((sr = poll(priv->pollfds, nfds, time)) > 0)
	{
		mowgli_eventloop_synchronize(eventloop);

		/* iterate twice so we don't touch freed memory if a pollable is destroyed */
		MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
		{
			pollable = n->data;
			slot = pollable->slot;

			if (slot == -1 || priv->pollfds[slot].revents == 0)
				continue;

			if (priv->pollfds[slot].revents & (POLLRDNORM | POLLIN | POLLHUP | POLLERR) && pollable->read_function)
			{
				priv->pollfds[slot].events &= ~(POLLRDNORM | POLLIN | POLLHUP | POLLERR);
				pollable->read_function(eventloop, pollable, MOWGLI_EVENTLOOP_POLL_READ, pollable->userdata);
			}
		}

		MOWGLI_ITER_FOREACH_SAFE(n, tn, priv->pollable_list.head)
		{
			pollable = n->data;
			slot = pollable->slot;

			if (slot == -1 || priv->pollfds[slot].revents == 0)
				continue;

			if (priv->pollfds[slot].revents & (POLLWRNORM | POLLOUT | POLLHUP | POLLERR) && pollable->write_function)
			{
				priv->pollfds[slot].events &= ~(POLLWRNORM | POLLOUT | POLLHUP | POLLERR);
				pollable->write_function(eventloop, pollable, MOWGLI_EVENTLOOP_POLL_WRITE, pollable->userdata);
			}
		}
	}
}

mowgli_eventloop_ops_t _mowgli_poll_pollops = {
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_poll_eventloop_pollsetup,
	.pollshutdown = mowgli_poll_eventloop_pollshutdown,
	.setselect = mowgli_poll_eventloop_setselect,
	.select = mowgli_poll_eventloop_select,
};

#endif
