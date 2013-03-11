/*
 * libmowgli: A collection of useful routines for programming.
 * echoserver.c: Testing of the I/O system
 *
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>
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

mowgli_eventloop_t *eventloop;

void
timer_oneshot(void *unused)
{
	printf("oneshot timer hit\n");
}

void
timer_tick(void *unused)
{
	static int ticks = 0;

	printf("tick: %d\n", ++ticks);

	if (ticks > 20)
		mowgli_eventloop_break(eventloop);
}

int
main(int argc, char *argv[])
{
	eventloop = mowgli_eventloop_create();

	mowgli_timer_add(eventloop, "timer_tick", timer_tick, NULL, 1);
	mowgli_timer_add_once(eventloop, "timer_oneshot", timer_oneshot, NULL, 5);

	mowgli_eventloop_run(eventloop);

	printf("eventloop halted\n");

	mowgli_eventloop_destroy(eventloop);

	return EXIT_SUCCESS;
}
