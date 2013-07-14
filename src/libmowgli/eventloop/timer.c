/*
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>
 * Copyright (c) 2005-2007 Atheme Project (http://www.atheme.org)
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

static mowgli_heap_t *timer_heap = NULL;

static mowgli_eventloop_timer_t *
mowgli_timer_add_real(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when, time_t frequency)
{
	mowgli_eventloop_timer_t *timer;

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(func != NULL, NULL);

	if (timer_heap == NULL)
		timer_heap = mowgli_heap_create(sizeof(mowgli_eventloop_timer_t), 16, BH_NOW);

	timer = mowgli_heap_alloc(timer_heap);

	timer->func = func;
	timer->name = name;
	timer->arg = arg;
	timer->deadline = mowgli_eventloop_get_time(eventloop) + when;
	timer->frequency = frequency;
	timer->active = true;

	if (eventloop->deadline != -1 && timer->deadline <= eventloop->deadline)
		eventloop->deadline = timer->deadline;

	mowgli_node_add(timer, &timer->node, &eventloop->timer_list);

#ifdef DEBUG
	mowgli_log("[timer(%p) add when:%d active:%d] [eventloop deadline:%d]", timer, timer->deadline, timer->active, eventloop->deadline);
#endif

	return timer;
}

/* add an event to the table to be continually ran */
mowgli_eventloop_timer_t *
mowgli_timer_add(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when)
{
	return mowgli_timer_add_real(eventloop, name, func, arg, when, when);
}

/* adds an event to the table to be ran only once */
mowgli_eventloop_timer_t *
mowgli_timer_add_once(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when)
{
	return mowgli_timer_add_real(eventloop, name, func, arg, when, 0);
}

/* delete an event from the table */
void
mowgli_timer_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_timer_t *timer)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(timer != NULL);

	if (eventloop->last_ran == timer->name)
		eventloop->last_ran = "<removed>";

	mowgli_node_delete(&timer->node, &eventloop->timer_list);
	mowgli_heap_free(timer_heap, timer);
}

/* checks all pending events */
void
mowgli_eventloop_run_timers(mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n, *tn;
	time_t currtime;

	return_if_fail(eventloop != NULL);

	currtime = mowgli_eventloop_get_time(eventloop);

	MOWGLI_ITER_FOREACH_SAFE(n, tn, eventloop->timer_list.head)
	{
		mowgli_eventloop_timer_t *timer = n->data;

		if (timer->active && (timer->deadline <= currtime))
		{
			/* now we call it */
			eventloop->last_ran = timer->name;
			timer->func(timer->arg);

			/* invalidate eventloop sleep-until time */
			eventloop->deadline = -1;

			/* event is scheduled more than once */
			if (timer->frequency)
			{
				timer->deadline = currtime + timer->frequency;
			}
			else
			{
				/* XXX: yuck.  find a better way to handle this. */
				eventloop->last_ran = "<onceonly>";

				mowgli_timer_destroy(eventloop, timer);
			}
		}
	}
}

/* returns the time the next mowgli_timer_run() should happen */
time_t
mowgli_eventloop_next_timer(mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n;

	return_val_if_fail(eventloop != NULL, 0);

	if (eventloop->deadline == -1)
		MOWGLI_ITER_FOREACH(n, eventloop->timer_list.head)
		{
			mowgli_eventloop_timer_t *timer = n->data;

			if (timer->active && ((timer->deadline < eventloop->deadline) || (eventloop->deadline == -1)))
				eventloop->deadline = timer->deadline;

#ifdef DEBUG
			mowgli_log("timer %p active:%d when:%ld deadline:%ld", timer, timer->active, timer->deadline, eventloop->deadline);
#endif
		}

#ifdef DEBUG
		mowgli_log("eventloop deadline:%ld", eventloop->deadline);

#endif

		return eventloop->deadline;
}

/* finds an event in the table */
mowgli_eventloop_timer_t *
mowgli_timer_find(mowgli_eventloop_t *eventloop, mowgli_event_dispatch_func_t *func, void *arg)
{
	mowgli_node_t *n;

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(func != NULL, NULL);

	MOWGLI_ITER_FOREACH(n, eventloop->timer_list.head)
	{
		mowgli_eventloop_timer_t *timer = n->data;

		if ((timer->func == func) && (timer->arg == arg))
			return timer;
	}

	return NULL;
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
