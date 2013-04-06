/*
 * Copyright (c) 2011, 2012 William Pitcock <nenolod@dereferenced.org>.
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

#ifdef MOWGLI_OS_OSX

# include <mach/mach.h>
# include <mach/mach_time.h>

#endif

#ifndef _WIN32

typedef int mowgli_descriptor_t;

#else

typedef SOCKET mowgli_descriptor_t;

#endif

typedef enum
{
	MOWGLI_EVENTLOOP_TYPE_POLLABLE,
	MOWGLI_EVENTLOOP_TYPE_HELPER,
	MOWGLI_EVENTLOOP_TYPE_ERROR = -1
} mowgli_eventloop_io_type_t;

typedef struct
{
	mowgli_eventloop_io_type_t type;
} mowgli_eventloop_io_obj_t;

typedef struct _mowgli_eventloop mowgli_eventloop_t;

typedef struct _mowgli_pollable mowgli_eventloop_pollable_t;
typedef struct _mowgli_helper mowgli_eventloop_helper_proc_t;

typedef struct _mowgli_linebuf mowgli_linebuf_t;

typedef enum
{
	MOWGLI_EVENTLOOP_IO_READ,
	MOWGLI_EVENTLOOP_IO_WRITE,
	MOWGLI_EVENTLOOP_IO_ERROR = -1
} mowgli_eventloop_io_dir_t;

typedef void mowgli_eventloop_io_t;

/* checked casts */
static inline mowgli_eventloop_pollable_t *
mowgli_eventloop_io_pollable(mowgli_eventloop_io_t *io)
{
	mowgli_eventloop_io_obj_t *obj = (mowgli_eventloop_io_obj_t *) io;

	return_val_if_fail(io != NULL, NULL);
	return_val_if_fail(obj->type == MOWGLI_EVENTLOOP_TYPE_POLLABLE, NULL);

	return (mowgli_eventloop_pollable_t *) io;
}

static inline mowgli_eventloop_helper_proc_t *
mowgli_eventloop_io_helper(mowgli_eventloop_io_t *io)
{
	mowgli_eventloop_io_obj_t *obj = (mowgli_eventloop_io_obj_t *) io;

	return_val_if_fail(io != NULL, NULL);
	return_val_if_fail(obj->type == MOWGLI_EVENTLOOP_TYPE_HELPER, NULL);

	return (mowgli_eventloop_helper_proc_t *) io;
}

static inline mowgli_eventloop_io_type_t
mowgli_eventloop_io_type(mowgli_eventloop_io_t *io)
{
	mowgli_eventloop_io_obj_t *obj = (mowgli_eventloop_io_obj_t *) io;

	return_val_if_fail(io != NULL, MOWGLI_EVENTLOOP_TYPE_ERROR);

	return obj->type;
}

typedef void mowgli_eventloop_io_cb_t (mowgli_eventloop_t * eventloop, mowgli_eventloop_io_t * io, mowgli_eventloop_io_dir_t dir, void *userdata);

struct _mowgli_pollable
{
	mowgli_eventloop_io_obj_t type;

	mowgli_descriptor_t fd;
	unsigned int slot;
	unsigned int events;

	mowgli_eventloop_io_cb_t *read_function;
	mowgli_eventloop_io_cb_t *write_function;
	mowgli_eventloop_io_cb_t *error_function;

	void *userdata;

	mowgli_node_t node;

	mowgli_eventloop_t *eventloop;
};

typedef struct
{
	void (*timeout_once)(mowgli_eventloop_t *eventloop, int timeout);
	void (*run_once)(mowgli_eventloop_t *eventloop);
	void (*pollsetup)(mowgli_eventloop_t *eventloop);
	void (*pollshutdown)(mowgli_eventloop_t *eventloop);
	void (*setselect)(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function);
	void (*select)(mowgli_eventloop_t *eventloop, int time);
	void (*destroy)(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable);
} mowgli_eventloop_ops_t;

struct _mowgli_eventloop
{
	time_t currtime;
	time_t deadline;

	const char *last_ran;

	mowgli_list_t timer_list;
	mowgli_mutex_t mutex;

	mowgli_eventloop_ops_t *eventloop_ops;
	void *poller;

	bool death_requested;

	void *data;

	time_t epochbias;
};

typedef void mowgli_event_dispatch_func_t (void *userdata);

typedef struct
{
	mowgli_node_t node;

	mowgli_event_dispatch_func_t *func;
	void *arg;
	const char *name;
	time_t frequency;
	time_t deadline;
	bool active;
} mowgli_eventloop_timer_t;

static inline void
mowgli_eventloop_set_time(mowgli_eventloop_t *eventloop, time_t newtime)
{
	return_if_fail(eventloop != NULL);

	eventloop->currtime = newtime;
}

static inline time_t
mowgli_eventloop_get_time(mowgli_eventloop_t *eventloop)
{
	return_val_if_fail(eventloop != NULL, 0);

	return eventloop->epochbias + eventloop->currtime;
}

static inline void
mowgli_eventloop_synchronize(mowgli_eventloop_t *eventloop)
{
	long long time_;

#if defined(CLOCK_MONOTONIC)
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);
	time_ = tp.tv_sec;
#elif defined(CLOCK_HIGHRES)
	struct timespec tp;

	clock_gettime(CLOCK_HIGHRES, &tp);
	time_ = tp.tv_sec;
#elif defined(MOWGLI_OS_WIN)
	static ULONGLONG (CALLBACK *GetTickCount64)(void) = NULL;
	static OSVERSIONINFOEX *winver = NULL;
	static bool load_err = false;

	if (winver == NULL)
	{
		winver = mowgli_alloc(sizeof(OSVERSIONINFOEX));
		winver->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (!GetVersionEx((OSVERSIONINFO *) winver))
		{
			mowgli_free(winver);
			winver = NULL;	/* FIXME */
		}
	}

	if (winver && (winver->dwMajorVersion >= 6))
	{
		if ((GetTickCount64 == NULL) && !load_err)
		{
			HINSTANCE hKernel32;

			hKernel32 = GetModuleHandle("KERNEL32");
			GetTickCount64 = GetProcAddress(hKernel32, "GetTickCount64");

			if (GetTickCount64 == NULL)
				load_err = true;
		}

		if (load_err)
		{
			time_ = time(NULL);
		}
		else
		{
			soft_assert(GetTickCount64 != NULL);

			time_ = (int) (GetTickCount64() * 1e-3);
		}
	}
	else
	{
		time_ = time(NULL);
	}

#elif defined(MOWGLI_OS_OSX)
	static mach_timebase_info_data_t timebase;

	if (timebase.denom == 0)
		mach_timebase_info(&timebase);

	time_ = (int) (mach_absolute_time() * timebase.numer / timebase.denom * 1e-9);
#else
	time_ = time(NULL);
#endif
	mowgli_eventloop_set_time(eventloop, (time_t) time_);
}

/* Sets the bias of eventloop->currtime relative to Jan 1 00:00:00 1970 */
static inline void
mowgli_eventloop_calibrate(mowgli_eventloop_t *eventloop)
{
	mowgli_eventloop_synchronize(eventloop);
	eventloop->epochbias = time(NULL) - eventloop->currtime;
}

static inline bool
mowgli_eventloop_ignore_errno(int error)
{
	switch (error)
	{
#ifdef EINPROGRESS
	case EINPROGRESS:
#endif
#if defined(EWOULDBLOCK)
	case EWOULDBLOCK:
#endif
#if defined(EAGAIN) && (EWOULDBLOCK != EAGAIN)
	case EAGAIN:
#endif
#ifdef ETIME
	case ETIME:
#endif
#ifdef EINTR
	case EINTR:
#endif
#ifdef ERESTART
	case ERESTART:
#endif
#ifdef ENOBUFS
	case ENOBUFS:
#endif
#ifdef ENOENT
	case ENOENT:
#endif
		return true;
	default:
		break;
	}

	return false;
}

typedef void mowgli_eventloop_helper_start_fn_t (mowgli_eventloop_helper_proc_t * helper, void *userdata);

struct _mowgli_helper
{
	mowgli_eventloop_io_obj_t type;

	mowgli_process_t *child;
	mowgli_eventloop_t *eventloop;

	mowgli_descriptor_t fd;
	mowgli_eventloop_pollable_t *pfd;

	mowgli_eventloop_io_cb_t *read_function;

	void *userdata;
};

/* helper.c */
extern mowgli_eventloop_helper_proc_t *mowgli_helper_create(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_start_fn_t *start_fn, const char *helpername, void *userdata);

/* creation of helpers inside other executable images */
extern mowgli_eventloop_helper_proc_t *mowgli_helper_spawn(mowgli_eventloop_t *eventloop, const char *path, char *const argv[]);
extern mowgli_eventloop_helper_proc_t *mowgli_helper_setup(mowgli_eventloop_t *eventloop);

/* synchronization of helpers happens on reading from mowgli_eventloop_helper_proc_t::in_pfd. */
extern void mowgli_helper_set_read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper, mowgli_eventloop_io_cb_t *read_fn);
extern void mowgli_helper_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper);

/* null_pollops.c */
extern void mowgli_simple_eventloop_run_once(mowgli_eventloop_t *eventloop);
extern void mowgli_simple_eventloop_timeout_once(mowgli_eventloop_t *eventloop, int timeout);
extern void mowgli_simple_eventloop_error_handler(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);

/* eventloop.c */
extern mowgli_eventloop_t *mowgli_eventloop_create(void);
extern void mowgli_eventloop_destroy(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_run(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_run_once(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_timeout_once(mowgli_eventloop_t *eventloop, int timeout);
extern void mowgli_eventloop_break(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_timers_only(mowgli_eventloop_t *eventloop);
extern void mowgli_eventloop_set_data(mowgli_eventloop_t *eventloop, void *data);
extern void *mowgli_eventloop_get_data(mowgli_eventloop_t *eventloop);

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
extern void mowgli_pollable_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function);
extern void mowgli_pollable_set_nonblocking(mowgli_eventloop_pollable_t *pollable, bool nonblocking);
extern void mowgli_pollable_trigger(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir);

#endif
