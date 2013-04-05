/*
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>.
 * Copyright (c) 2011 Jilles Tjoelker <jilles@stack.nl>.
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

#ifdef HAVE_KQUEUE

# include <sys/event.h>

typedef struct
{
	int kqueue_fd;
	int nevents;
	struct kevent *events;
} mowgli_kqueue_eventloop_private_t;

static void
mowgli_kqueue_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_kqueue_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_kqueue_eventloop_private_t));
	eventloop->poller = priv;

	priv->nevents = getdtablesize();
	priv->kqueue_fd = kqueue();
	priv->events = mowgli_alloc(sizeof(struct kevent) * priv->nevents);

	return;
}

static void
mowgli_kqueue_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_kqueue_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	close(priv->kqueue_fd);

	mowgli_free(priv->events);
	mowgli_free(priv);
	return;
}

static void
mowgli_kqueue_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_kqueue_eventloop_private_t *priv;
	struct kevent event;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;

	EV_SET(&event, pollable->fd, EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, pollable);

	if (kevent(priv->kqueue_fd, &event, 1, NULL, 0,
		   &(const struct timespec) { .tv_sec = 0, .tv_nsec = 0 }
		   ) != 0)
		mowgli_log("mowgli_kqueue_eventloop_setselect(): kevent failed: %d (%s)", errno, strerror(errno));
}

static void
mowgli_kqueue_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_kqueue_eventloop_private_t *priv;
	mowgli_eventloop_io_cb_t **fptr;
	struct kevent event;

	int filter;
	bool change;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;
	change = false;

# ifdef DEBUG
	mowgli_log("setselect %p fd %d func %p", pollable, pollable->fd, event_function);
# endif

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:
		fptr = &pollable->read_function;
		filter = EVFILT_READ;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		fptr = &pollable->write_function;
		filter = EVFILT_WRITE;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		return;
	}

	change = (*fptr != NULL) != (event_function != NULL);

	*fptr = event_function;

	if (!change)
		return;

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	EV_SET(&event, pollable->fd, filter,
	       event_function ? EV_ADD : EV_DELETE, 0, 0, pollable);

	if (kevent(priv->kqueue_fd, &event, 1, NULL, 0,
		   &(const struct timespec) { .tv_sec = 0, .tv_nsec = 0 }
		   ) != 0)
		mowgli_log("mowgli_kqueue_eventloop_setselect(): kevent failed: %d (%s)", errno, strerror(errno));
}

static void
mowgli_kqueue_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	mowgli_kqueue_eventloop_private_t *priv;
	int i, num, o_errno;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	num = kevent(priv->kqueue_fd, NULL, 0, priv->events, priv->nevents,
		     delay >= 0 ? &(const struct timespec) { .tv_sec = delay / 1000,
							     .tv_nsec = delay % 1000 * 1000000 } : NULL);

	o_errno = errno;
	mowgli_eventloop_synchronize(eventloop);

	if (num < 0)
	{
		if (mowgli_eventloop_ignore_errno(o_errno))
			return;

		mowgli_log("mowgli_kqueue_eventloop_select(): kevent failed: %d (%s)", o_errno, strerror(o_errno));
		return;
	}

	for (i = 0; i < num; i++)
	{
		mowgli_eventloop_pollable_t *pollable = priv->events[i].udata;

		if (priv->events[i].filter == EVFILT_READ)
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);

		if (priv->events[i].filter == EVFILT_WRITE)
			mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);

		/* XXX Perhaps we need to recheck read_function and
		 * write_function now.
		 */
	}
}

mowgli_eventloop_ops_t _mowgli_kqueue_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_kqueue_eventloop_pollsetup,
	.pollshutdown = mowgli_kqueue_eventloop_pollshutdown,
	.setselect = mowgli_kqueue_eventloop_setselect,
	.select = mowgli_kqueue_eventloop_select,
	.destroy = mowgli_kqueue_eventloop_destroy,
};

#endif
