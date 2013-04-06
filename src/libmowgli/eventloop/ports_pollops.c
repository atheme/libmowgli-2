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

#ifdef HAVE_PORT_CREATE

# include <port.h>

typedef struct
{
	int port_fd;
	int pfd_size;
	port_event_t *pfd;
} mowgli_ports_eventloop_private_t;

static void
mowgli_ports_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_ports_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_ports_eventloop_private_t));
	eventloop->poller = priv;

	priv->pfd_size = getdtablesize();
	priv->port_fd = port_create();
	priv->pfd = mowgli_alloc(sizeof(port_event_t) * priv->pfd_size);

	return;
}

static void
mowgli_ports_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_ports_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	close(priv->port_fd);

	mowgli_free(priv->pfd);
	mowgli_free(priv);
	return;
}

static void
mowgli_ports_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_ports_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;
	pollable->slot = 0;

	if (port_dissociate(priv->port_fd, PORT_SOURCE_FD, (uintptr_t) pollable->fd) < 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_ports_eventloop_destroy(): port_dissociate failed: %d (%s)", errno, strerror(errno));
	}
}

static void
mowgli_ports_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_ports_eventloop_private_t *priv;
	unsigned int old_flags;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;
	old_flags = pollable->slot;

# ifdef DEBUG
	mowgli_log("setselect %p fd %d func %p", pollable, pollable->fd, event_function);
# endif

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:
		pollable->read_function = event_function;
		pollable->slot |= POLLIN;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		pollable->write_function = event_function;
		pollable->slot |= POLLOUT;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	if (pollable->read_function == NULL)
		pollable->slot &= ~POLLIN;

	if (pollable->write_function == NULL)
		pollable->slot &= ~POLLOUT;

	if ((old_flags == 0) && (pollable->slot == 0))
	{
		return;
	}
	else if (pollable->slot == 0)
	{
		port_dissociate(priv->port_fd, PORT_SOURCE_FD, (uintptr_t) pollable->fd);
		return;
	}

	if (port_associate(priv->port_fd, PORT_SOURCE_FD, (uintptr_t) pollable->fd, pollable->slot, pollable) < 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_ports_eventloop_setselect(): port_associate failed: %d (%s)", errno, strerror(errno));
	}

	return;
}

static void
mowgli_ports_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	mowgli_ports_eventloop_private_t *priv;
	int i, ret, o_errno, nget = 1;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	ret = port_getn(priv->port_fd, priv->pfd, priv->pfd_size, &nget,
			delay >= 0 ? &(struct timespec) { .tv_sec = delay / 1000, .tv_nsec = delay % 1000 * 1000000 } : NULL);

	o_errno = errno;
	mowgli_eventloop_synchronize(eventloop);

	if (ret == -1)
	{
		if (mowgli_eventloop_ignore_errno(o_errno))
			return;

		mowgli_log("mowgli_ports_eventloop_select(): port_getn failed: %d (%s)", o_errno, strerror(o_errno));
		return;
	}

	for (i = 0; i < nget; i++)
	{
		mowgli_eventloop_pollable_t *pollable = priv->pfd[i].portev_user;

		if (priv->pfd[i].portev_source != PORT_SOURCE_FD)
			continue;

		if (priv->pfd[i].portev_events & (POLLIN | POLLHUP | POLLERR))
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);

		if (priv->pfd[i].portev_events & (POLLOUT | POLLHUP | POLLERR))
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);
	}
}

mowgli_eventloop_ops_t _mowgli_ports_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_ports_eventloop_pollsetup,
	.pollshutdown = mowgli_ports_eventloop_pollshutdown,
	.setselect = mowgli_ports_eventloop_setselect,
	.select = mowgli_ports_eventloop_select,
	.destroy = mowgli_ports_eventloop_destroy,
};

#endif
