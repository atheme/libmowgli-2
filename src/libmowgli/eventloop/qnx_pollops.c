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

#ifdef HAVE_DISPATCH_BLOCK

# include <sys/iofunc.h>
# include <sys/dispatch.h>

typedef struct
{
	dispatch_t *dpp;
	dispatch_context_t *ctp;
} mowgli_qnx_eventloop_private_t;

static void
mowgli_qnx_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	mowgli_qnx_eventloop_private_t *priv;

	priv = mowgli_alloc(sizeof(mowgli_qnx_eventloop_private_t));
	eventloop->poller = priv;

	priv->dpp = dispatch_create();

	return;
}

static void
mowgli_qnx_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	mowgli_qnx_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	if (priv->ctp != NULL)
		dispatch_context_free(priv->ctp);

	dispatch_destroy(priv->dpp);
	mowgli_free(priv);

	return;
}

static void
mowgli_qnx_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_qnx_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;

	if (select_detach(priv->dpp, pollable->fd) != 0)
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_qnx_eventloop_destroy(): select_detach failed: %d (%s)", errno, strerror(errno));
	}
}

static void
mowgli_qnx_eventloop_event_cb(select_context_t *ctp, mowgli_descriptor_t fd, unsigned int flags, void *userdata)
{
	mowgli_eventloop_t *eventloop;
	mowgli_eventloop_pollable_t *pollable;

	return_if_fail(ctp != NULL);
	return_if_fail(userdata != NULL);

	pollable = userdata;
	eventloop = pollable->eventloop;

	return_if_fail(eventloop != NULL);

	if (flags & (SELECT_FLAG_READ | SELECT_FLAG_EXCEPT))
		mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);

	if (flags & (SELECT_FLAG_WRITE | SELECT_FLAG_EXCEPT))
		mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);
}

static void
mowgli_qnx_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_qnx_eventloop_private_t *priv;
	select_attr_t attr = { };
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
		pollable->slot |= SELECT_FLAG_READ;
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		pollable->write_function = event_function;
		pollable->slot |= SELECT_FLAG_WRITE;
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	if (pollable->read_function == NULL)
		pollable->slot &= ~SELECT_FLAG_READ;

	if (pollable->write_function == NULL)
		pollable->slot &= ~SELECT_FLAG_WRITE;

	if ((old_flags == 0) && (pollable->slot == 0))
		return;

	if (old_flags)
		select_detach(priv->dpp, pollable->fd);

	if (pollable->slot)
		if (select_attach(priv->dpp, &attr, pollable->fd, pollable->slot, mowgli_qnx_eventloop_event_cb, pollable) != 0)
		{
			if (mowgli_eventloop_ignore_errno(errno))
				return;

			mowgli_log("mowgli_qnx_eventloop_setselect(): select_attach failed: %d (%s)", errno, strerror(errno));
		}

	return;
}

static void
mowgli_qnx_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	dispatch_context_t *new_ctp;
	mowgli_qnx_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	/* set timeout if needed */
	dispatch_timeout(priv->dpp, delay >= 0 ? &(struct timespec) { .tv_sec = delay / 1000, .tv_nsec = delay % 1000 * 1000000 } : NULL);

	if (priv->ctp != NULL)
		priv->ctp = dispatch_context_alloc(priv->dpp);

	/* if dispatch_block returns non-NULL, priv->ctp may have been realloc()'d, NULL is error condition */
	if ((new_ctp = dispatch_block(priv->ctp)) != NULL)
	{
		priv->ctp = new_ctp;
	}
	else
	{
		if (mowgli_eventloop_ignore_errno(errno))
			return;

		mowgli_log("mowgli_qnx_eventloop_select(): dispatch_block failed: %d (%s)", errno, strerror(errno));
	}

	mowgli_eventloop_synchronize(eventloop);
}

mowgli_eventloop_ops_t _mowgli_qnx_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_qnx_eventloop_pollsetup,
	.pollshutdown = mowgli_qnx_eventloop_pollshutdown,
	.setselect = mowgli_qnx_eventloop_setselect,
	.select = mowgli_qnx_eventloop_select,
	.destroy = mowgli_qnx_eventloop_destroy,
};

#endif
