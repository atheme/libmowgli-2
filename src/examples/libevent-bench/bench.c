/*
 * Copyright 2012 William Pitcock <nenolod@dereferenced.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

/*
 * Copyright 2003 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Mon 03/10/2003 - Modified by Davide Libenzi <davidel@xmailserver.org>
 *
 *     Added chain event propagation to improve the sensitivity of
 *     the measure respect to the event loop efficency.
 *
 *
 */

#define timersub(tvp, uvp, vvp)	\
	do \
	{ \
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
		if ((vvp)->tv_usec < 0)	\
		{ \
			(vvp)->tv_sec--; \
			(vvp)->tv_usec += 1000000; \
		} \
	} while (0)

#include <mowgli.h>

static int count, writes, fired;
static mowgli_eventloop_t *base_eventloop;
static mowgli_descriptor_t *pipes;
static int num_pipes, num_active, num_writes;
static mowgli_eventloop_pollable_t **events;
static int timers;

void
timer_cb(void *unused)
{
	/* nop */
}

void
read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *arg)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);

	int idx = (int) (long) arg, widx = idx + 1;
	u_char ch;

	count += read(pollable->fd, &ch, sizeof(ch));

	if (writes)
	{
		if (widx >= num_pipes)
			widx -= num_pipes;

		write(pipes[2 * widx + 1], "e", 1);
		writes--;
		fired++;
	}
}

#if NATIVE
void
read_thunk(struct ev_io *w, int revents)
{
	read_cb(w->fd, revents, w->data);
}

void
timer_cb(struct ev_timer *w, int revents)
{
	/* nop */
}

#endif

struct timeval *
run_once(void)
{
	int *cp, i, space;

	static struct timeval ta, ts, te;

	gettimeofday(&ta, NULL);

	for (cp = pipes, i = 0; i < num_pipes; i++, cp += 2)
	{
		if (events[i] != NULL)
			mowgli_pollable_destroy(base_eventloop, events[i]);

		events[i] = mowgli_pollable_create(base_eventloop, cp[0], (void *) (long) i);
		mowgli_pollable_setselect(base_eventloop, events[i], MOWGLI_EVENTLOOP_IO_READ, read_cb);
	}

	fired = 0;
	space = num_pipes / num_active;
	space = space * 2;

	for (i = 0; i < num_active; i++, fired++)
		write(pipes[i * space + 1], "e", 1);

	count = 0;
	writes = num_writes;

	int xcount = 0;
	gettimeofday(&ts, NULL);

	do
	{
		mowgli_eventloop_run_once(base_eventloop);
		xcount++;
	} while (count != fired);

	gettimeofday(&te, NULL);

	timersub(&te, &ta, &ta);
	timersub(&te, &ts, &ts);
	fprintf(stdout, "%ld\t%ld\n",
		ta.tv_sec * 1000000L + ta.tv_usec,
		ts.tv_sec * 1000000L + ts.tv_usec
		);

	return &te;
}

int
main(int argc, char **argv)
{
	struct rlimit rl;

	int i, c;
	int *cp;
	extern char *optarg;

	num_pipes = 100;
	num_active = 1;
	num_writes = num_pipes;

	while ((c = getopt(argc, argv, "n:a:w:te")) != -1)
	{
		switch (c)
		{
		case 'n':
			num_pipes = atoi(optarg);
			break;
		case 'a':
			num_active = atoi(optarg);
			break;
		case 'w':
			num_writes = atoi(optarg);
			break;
		case 't':
			timers = 1;
			break;
		default:
			fprintf(stderr, "Illegal argument \"%c\"\n", c);
			exit(1);
		}
	}

#if 1
	rl.rlim_cur = rl.rlim_max = num_pipes * 2 + 50;

	if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
		perror("setrlimit");

#endif

	events = calloc(num_pipes * 2, sizeof(mowgli_eventloop_pollable_t *));
	pipes = calloc(num_pipes * 2, sizeof(mowgli_descriptor_t));

	if ((events == NULL) || (pipes == NULL))
	{
		perror("malloc");
		exit(1);
	}

	mowgli_thread_set_policy(MOWGLI_THREAD_POLICY_DISABLED);
	base_eventloop = mowgli_eventloop_create();

	for (cp = pipes, i = 0; i < num_pipes; i++, cp += 2)
	{
#ifdef USE_PIPES

		if (pipe(cp) == -1)
		{
#else

		if (socketpair(AF_UNIX, SOCK_STREAM, 0, cp) == -1)
		{
#endif
			perror("pipe");
			exit(1);
		}
	}

	for (i = 0; i < 2; i++)
		run_once();

	exit(0);
}
