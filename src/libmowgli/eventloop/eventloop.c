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

static mowgli_heap_t *eventloop_heap = NULL;

extern mowgli_eventloop_ops_t _mowgli_null_pollops;

#ifdef HAVE_PORT_CREATE
extern mowgli_eventloop_ops_t _mowgli_ports_pollops;
#endif
#ifdef HAVE_DISPATCH_BLOCK
extern mowgli_eventloop_ops_t _mowgli_qnx_pollops;
#endif
#ifdef HAVE_SELECT
extern mowgli_eventloop_ops_t _mowgli_select_pollops;
#endif
#ifdef HAVE_POLL_H
extern mowgli_eventloop_ops_t _mowgli_poll_pollops;
#endif
#ifdef HAVE_SYS_EPOLL_H
extern mowgli_eventloop_ops_t _mowgli_epoll_pollops;
#endif
#ifdef HAVE_KQUEUE
extern mowgli_eventloop_ops_t _mowgli_kqueue_pollops;
#endif
#if 0
extern mowgli_eventloop_ops_t _mowgli_winsock_pollops;
#endif

mowgli_eventloop_t *
mowgli_eventloop_create(void)
{
	mowgli_eventloop_t *eventloop;

	if (eventloop_heap == NULL)
		eventloop_heap = mowgli_heap_create(sizeof(mowgli_eventloop_t), 16, BH_NOW);

	eventloop = mowgli_heap_alloc(eventloop_heap);

	eventloop->eventloop_ops = &_mowgli_null_pollops;

#ifdef HAVE_SELECT
	eventloop->eventloop_ops = &_mowgli_select_pollops;
#endif
#ifdef HAVE_POLL_H
	eventloop->eventloop_ops = &_mowgli_poll_pollops;
#endif
#ifdef HAVE_SYS_EPOLL_H
	eventloop->eventloop_ops = &_mowgli_epoll_pollops;
#endif
#ifdef HAVE_KQUEUE
	eventloop->eventloop_ops = &_mowgli_kqueue_pollops;
#endif
#ifdef HAVE_DISPATCH_BLOCK
	eventloop->eventloop_ops = &_mowgli_qnx_pollops;
#endif
#ifdef HAVE_PORT_CREATE
	eventloop->eventloop_ops = &_mowgli_ports_pollops;
#endif
#if 0
	eventloop->eventloop_ops = &_mowgli_winsock_pollops;
#endif

	if (mowgli_mutex_init(&eventloop->mutex) != 0)
	{
		mowgli_log("couldn't create mutex for eventloop %p, aborting...", (void *) eventloop);
		abort();
	}

	eventloop->eventloop_ops->pollsetup(eventloop);

	mowgli_eventloop_calibrate(eventloop);

	return eventloop;
}

void
mowgli_eventloop_destroy(mowgli_eventloop_t *eventloop)
{
	eventloop->eventloop_ops->pollshutdown(eventloop);

	mowgli_mutex_uninit(&eventloop->mutex);
	mowgli_heap_free(eventloop_heap, eventloop);
}

void
mowgli_eventloop_run(mowgli_eventloop_t *eventloop)
{
	return_if_fail(eventloop != NULL);

	mowgli_mutex_lock(&eventloop->mutex);

	eventloop->death_requested = false;

	while (!eventloop->death_requested)
	{
		eventloop->eventloop_ops->run_once(eventloop);
	}

	mowgli_mutex_unlock(&eventloop->mutex);
}

void
mowgli_eventloop_run_once(mowgli_eventloop_t *eventloop)
{
	return_if_fail(eventloop != NULL);

	mowgli_mutex_lock(&eventloop->mutex);

	eventloop->eventloop_ops->run_once(eventloop);

	mowgli_mutex_unlock(&eventloop->mutex);
}

void
mowgli_eventloop_timeout_once(mowgli_eventloop_t *eventloop, int timeout)
{
	return_if_fail(eventloop != NULL);

	mowgli_mutex_lock(&eventloop->mutex);

	if (timeout >= 0)
		eventloop->eventloop_ops->timeout_once(eventloop, timeout);
	else
		eventloop->eventloop_ops->run_once(eventloop);

	mowgli_mutex_unlock(&eventloop->mutex);
}

void
mowgli_eventloop_break(mowgli_eventloop_t *eventloop)
{
	return_if_fail(eventloop != NULL);

	eventloop->death_requested = true;
}

/* convenience function to request null pollops */
void
mowgli_eventloop_timers_only(mowgli_eventloop_t *eventloop)
{
	return_if_fail(eventloop != NULL);

	eventloop->eventloop_ops = &_mowgli_null_pollops;
}

/* userdata setting/getting functions (for bindings) */
void *
mowgli_eventloop_get_data(mowgli_eventloop_t *eventloop)
{
	return_val_if_fail(eventloop != NULL, NULL);

	return eventloop->data;
}

void
mowgli_eventloop_set_data(mowgli_eventloop_t *eventloop, void *data)
{
	return_if_fail(eventloop != NULL);

	eventloop->data = data;
}
