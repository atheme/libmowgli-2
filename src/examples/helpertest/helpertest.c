/*
 * libmowgli: A collection of useful routines for programming.
 * echoserver.c: Testing of the I/O system
 *
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>
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

#include <mowgli.h>

int helper_count = 0;

void
timer_oneshot(mowgli_eventloop_helper_proc_t *helper)
{
	mowgli_writef(helper->fd, "oneshot timer hit\n");
}

void
timer_tick(mowgli_eventloop_helper_proc_t *helper)
{
	static int ticks = 0;

	mowgli_writef(helper->fd, "tick: %d\n", ++ticks);

	if (ticks > 10)
		mowgli_eventloop_break(helper->eventloop);
}

void
helper_start(mowgli_eventloop_helper_proc_t *helper, void *userdata)
{
	mowgli_eventloop_t *eventloop = helper->eventloop;

	mowgli_writef(helper->fd, "hi from pid %d\n", getpid());

	mowgli_timer_add(eventloop, "timer_tick", (mowgli_event_dispatch_func_t *) timer_tick, helper, 1);
	mowgli_timer_add_once(eventloop, "timer_oneshot", (mowgli_event_dispatch_func_t *) timer_oneshot, helper, 5);

	mowgli_eventloop_run(eventloop);

	mowgli_writef(helper->fd, "eventloop halted\n");

	mowgli_eventloop_destroy(eventloop);
}

void helper_read(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);

void
helper_spawn(mowgli_eventloop_t *eventloop)
{
	mowgli_eventloop_helper_proc_t *helper;

	if (helper_count >= 100)
		return;

	helper = mowgli_helper_create(eventloop, helper_start, "Spawned helper", NULL);
	mowgli_helper_set_read_cb(eventloop, helper, helper_read);

	helper_count++;
}

void
helper_read(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	size_t r;
	char buf[16384];
	mowgli_eventloop_helper_proc_t *helper = mowgli_eventloop_io_helper(io);

	bzero(buf, sizeof buf);
	r = read(helper->fd, buf, sizeof buf);

	if (r > 0)
	{
		printf("helper %p [%d/%d]: %s", helper, helper->child->pid, helper->fd, buf);
	}
	else if (r <= 0)
	{
		helper_count--;
		mowgli_helper_destroy(eventloop, helper);
	}

	if ((rand() % helper_count) == 0)
		helper_spawn(eventloop);
}

int
main(int argc, char *argv[])
{
	mowgli_eventloop_t *base_eventloop;

	/* Bleh this is needed to ensure some systems can set the process title */
	argv = mowgli_proctitle_init(argc, argv);

	base_eventloop = mowgli_eventloop_create();

	helper_spawn(base_eventloop);

	mowgli_eventloop_run(base_eventloop);

	return EXIT_SUCCESS;
}
