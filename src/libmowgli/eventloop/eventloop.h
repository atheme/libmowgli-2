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

#ifndef _WIN32

typedef int mowgli_descriptor_t;

#else

typedef HANDLE mowgli_descriptor_t;

#endif

typedef struct _mowgli_eventloop mowgli_eventloop_t;

typedef struct _mowgli_pollable mowgli_eventloop_pollable_t;

typedef enum {
	MOWGLI_EVENTLOOP_POLL_READ,
	MOWGLI_EVENTLOOP_POLL_WRITE,
} mowgli_eventloop_pollable_dir_t;

typedef void mowgli_pollevent_dispatch_func_t(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, void *userdata);

struct _mowgli_pollable {
	mowgli_descriptor_t fd;
	unsigned int slot;

	mowgli_pollevent_dispatch_func_t *read_function;
	mowgli_pollevent_dispatch_func_t *write_function;

	void *userdata;

	mowgli_node_t node;
};

typedef struct {
	void (*run_once)(mowgli_eventloop_t *eventloop);
	void (*pollsetup)(mowgli_eventloop_t *eventloop);
	void (*pollshutdown)(mowgli_eventloop_t *eventloop);
	void (*setselect)(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, mowgli_pollevent_dispatch_func_t *event_function);
	void (*select)(mowgli_eventloop_t *eventloop, int time);
} mowgli_eventloop_ops_t;

struct _mowgli_eventloop {
	time_t currtime;
	time_t time_min;

	const char *last_ran;

	mowgli_list_t timer_list;

	mowgli_eventloop_ops_t *eventloop_ops;
	void *poller;

	bool death_requested;
};

typedef void mowgli_event_dispatch_func_t(void *userdata);

typedef struct {
	mowgli_node_t node;

	mowgli_event_dispatch_func_t *func;
	void *arg;
	const char *name;
	time_t frequency;
	time_t when;
	bool active;
} mowgli_eventloop_timer_t;

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

/* null_pollops.c */
extern void mowgli_simple_eventloop_run_once(mowgli_eventloop_t *eventloop);

/* eventloop.c */
extern mowgli_eventloop_t *mowgli_eventloop_create(void);
extern void mowgli_eventloop_destroy(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_run(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_run_once(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_break(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_timers_only(mowgli_eventloop_t *eventloop);

/* timer.c */
extern mowgli_eventloop_timer_t *mowgli_timer_add(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when);
extern mowgli_eventloop_timer_t *mowgli_timer_add_once(mowgli_eventloop_t *eventloop, const char *name, mowgli_event_dispatch_func_t *func, void *arg, time_t when);
extern void mowgli_timer_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_timer_t *timer);
extern void mowgli_eventloop_run_timers(mowgli_eventloop_t *eventloop);
extern time_t mowgli_eventloop_next_timer(mowgli_eventloop_t *eventloop);
extern mowgli_eventloop_timer_t *mowgli_timer_find(mowgli_eventloop_t *eventloop, mowgli_event_dispatch_func_t *func, void *arg);

/* pollable.c */
extern mowgli_eventloop_pollable_t *mowgli_pollable_create(mowgli_eventloop_t *eventloop, mowgli_descriptor_t fd, void *userdata);
extern void mowgli_pollable_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable);
extern void mowgli_pollable_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, mowgli_pollevent_dispatch_func_t *event_function);

#endif
