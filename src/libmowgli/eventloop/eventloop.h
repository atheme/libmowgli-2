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

#ifndef __MOWGLI_EVENTLOOP_EVENTLOOP_H__
#define __MOWGLI_EVENTLOOP_EVENTLOOP_H__

typedef void mowgli_event_dispatch_func_t(void *);

typedef struct {
	mowgli_node_t node;

	mowgli_event_dispatch_func_t *func;
	void *arg;
	const char *name;
	time_t frequency;
	time_t when;
	bool active;
} mowgli_eventloop_timer_t;

typedef struct {
	time_t currtime;
	time_t time_min;

	const char *last_ran;

	mowgli_list_t timer_list;
} mowgli_eventloop_t;

static inline void mowgli_eventloop_set_time(mowgli_eventloop_t *eventloop, time_t time)
{
	return_if_fail(eventloop != NULL);

	eventloop->currtime = time;
}

static inline time_t mowgli_eventloop_get_time(mowgli_eventloop_t *eventloop)
{
	return_val_if_fail(eventloop != NULL, 0);

	return eventloop->currtime;
}

static inline void mowgli_eventloop_synchronize(mowgli_eventloop_t *eventloop)
{
	mowgli_eventloop_set_time(eventloop, time(NULL));
}

extern mowgli_eventloop_t *mowgli_eventloop_create(void);
extern void mowgli_eventloop_destroy(mowgli_eventloop_t *eventloop);

/* timer.c */
extern mowgli_eventloop_timer_t *mowgli_timer_add(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when);
extern mowgli_eventloop_timer_t *mowgli_timer_add_once(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when);
extern void mowgli_timer_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_timer_t *timer);
extern void mowgli_eventloop_run_timers(mowgli_eventloop_t *eventloop);
extern time_t mowgli_eventloop_next_timer(mowgli_eventloop_t *eventloop);
extern mowgli_eventloop_timer_t *mowgli_timer_find(mowgli_eventloop_t *eventloop, mowgli_event_dispatch_func_t *func, void *arg);

#endif
