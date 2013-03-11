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

#ifdef HAVE_SYS_EPOLL_H

# include <sys/epoll.h>

typedef struct
{
	int epoll_fd;
	int pfd_size;
	struct epoll_event *pfd;
} mowgli_epoll_eventloop_private_t;

static void
mowgli_epoll_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_epoll_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_epoll_eventloop_private_t));
	eventloop->poller = priv;

	priv->pfd_size = getdtablesize();
	priv->epoll_fd = epoll_create(priv->pfd_size);
	priv->pfd = mowgli_alloc(sizeof(struct epoll_event) * priv->pfd_size);

	return;
}

static void
mowgli_epoll_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_epoll_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	close(priv->epoll_fd);

	mowgli_free(priv->pfd);
	mowgli_free(priv);
	return;
}

static void
mowgli_epoll_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_epoll_eventloop_private_t *priv;
	struct epoll_event ep_event;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;
	pollable->slot = 0;

	ep_event.events = pollable->slot;
	ep_event.data.ptr = pollable;

	if (epoll_ctl(priv->epoll_fd, EPOLL_CTL_DEL, pollable->fd, &ep_event) != 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_epoll_eventloop_destroy(): epoll_ctl failed: %d (%s)", errno, strerror(errno));
	}
}

static void
mowgli_epoll_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_epoll_eventloop_private_t *priv;
	struct epoll_event ep_event;

	int op = -1;
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
		pollable->slot |= EPOLLIN;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		pollable->write_function = event_function;
		pollable->slot |= EPOLLOUT;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	if (pollable->read_function == NULL)
		pollable->slot &= ~EPOLLIN;

	if (pollable->write_function == NULL)
		pollable->slot &= ~EPOLLOUT;

	if ((old_flags == 0) && (pollable->slot == 0))
		return;
	else if (pollable->slot <= 0)
		op = EPOLL_CTL_DEL;
	else if ((old_flags == 0) && (pollable->slot != 0))
		op = EPOLL_CTL_ADD;
	else if (pollable->slot != old_flags)
		op = EPOLL_CTL_MOD;

	if (op == -1)
		return;

	ep_event.events = pollable->slot;
	ep_event.data.ptr = pollable;

	if (epoll_ctl(priv->epoll_fd, op, pollable->fd, &ep_event) != 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_epoll_eventloop_setselect(): epoll_ctl failed: %d (%s)", errno, strerror(errno));
	}

	return;
}

static void
mowgli_epoll_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	mowgli_epoll_eventloop_private_t *priv;
	int i, num, o_errno;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	num = epoll_wait(priv->epoll_fd, priv->pfd, priv->pfd_size, delay);

	o_errno = errno;
	mowgli_eventloop_synchronize(eventloop);

	if (num < 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_epoll_eventloop_select(): epoll_wait failed: %d (%s)", o_errno, strerror(o_errno));
		return;
	}

	for (i = 0; i < num; i++)
	{
		mowgli_eventloop_pollable_t *pollable = priv->pfd[i].data.ptr;

		if (priv->pfd[i].events & (EPOLLIN | EPOLLHUP | EPOLLERR))
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);

		if (priv->pfd[i].events & (EPOLLOUT | EPOLLHUP | EPOLLERR))
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);
	}
}

mowgli_eventloop_ops_t _mowgli_epoll_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_epoll_eventloop_pollsetup,
	.pollshutdown = mowgli_epoll_eventloop_pollshutdown,
	.setselect = mowgli_epoll_eventloop_setselect,
	.select = mowgli_epoll_eventloop_select,
	.destroy = mowgli_epoll_eventloop_destroy,
};

#endif
