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

static void mowgli_null_eventloop_run_once(mowgli_eventloop_t *eventloop)
{
	time_t delay;

	mowgli_eventloop_synchronize(eventloop);

	delay = mowgli_eventloop_next_timer(eventloop);
	delay -= mowgli_eventloop_get_time(eventloop);

	if (delay <= 0)
		mowgli_eventloop_run_timers(eventloop);
	else
		sleep(delay);
}

static void mowgli_null_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	return;
}

static void mowgli_null_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	return;
}

static void mowgli_null_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, mowgli_pollevent_dispatch_func_t *event_function)
{
	mowgli_log("null eventloop does not really do polling, events for pollable<%p> will be ignored", pollable);

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

static void mowgli_null_eventloop_select(mowgli_eventloop_t *eventloop)
{
	return;
}

mowgli_eventloop_ops_t _mowgli_null_pollops = {
	.run_once = mowgli_null_eventloop_run_once,
	.pollsetup = mowgli_null_eventloop_pollsetup,
	.pollshutdown = mowgli_null_eventloop_pollshutdown,
	.setselect = mowgli_null_eventloop_setselect,
	.select = mowgli_null_eventloop_select,
};
