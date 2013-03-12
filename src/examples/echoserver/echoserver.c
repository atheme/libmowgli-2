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

mowgli_eventloop_t *base_eventloop;
mowgli_eventloop_pollable_t *listener;

typedef struct
{
	mowgli_eventloop_io_t *io;
	char buf[1024];
} client_t;

#ifdef DEBUG
static void
timer_tick(void *unused)
{
	static int ticks = 0;

	printf("tick: %d\n", ++ticks);
}

#endif

static int
setup_listener(void)
{
	struct sockaddr_in in = { };
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	in.sin_family = AF_INET;
	in.sin_port = htons(1337);

	if (bind(fd, (struct sockaddr *) &in, sizeof(struct sockaddr_in)) < 0)
	{
		in.sin_port = htons(31337);
		bind(fd, (struct sockaddr *) &in, sizeof(struct sockaddr_in));
	}

	listen(fd, 5);

	return fd;
}

static void
write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	client_t *client = userdata;

	if (*client->buf)
		send(pollable->fd, client->buf, strlen(client->buf), 0);

	memset(client->buf, '\0', sizeof(client->buf));

	mowgli_pollable_setselect(base_eventloop, client->io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
}

static void
read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	int ret;

	client_t *client = userdata;

	if ((ret = recv(pollable->fd, client->buf, sizeof(client->buf), 0)) <= 0)
	{
		mowgli_free(client);
		mowgli_pollable_destroy(eventloop, io);

		return;
	}

	mowgli_pollable_setselect(base_eventloop, client->io, MOWGLI_EVENTLOOP_IO_WRITE, write_data);
}

static void
client_error(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_free(userdata);
	mowgli_pollable_destroy(eventloop, io);
}

static void
accept_client(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	client_t *client;
	mowgli_descriptor_t new_fd, listener_fd;

	listener_fd = pollable->fd;

	new_fd = accept(listener_fd, NULL, NULL);

	client = mowgli_alloc(sizeof(client_t));

	client->io = mowgli_pollable_create(eventloop, new_fd, client);
	mowgli_pollable_set_nonblocking(client->io, true);

	mowgli_pollable_setselect(base_eventloop, client->io, MOWGLI_EVENTLOOP_IO_READ, read_data);
	mowgli_pollable_setselect(base_eventloop, client->io, MOWGLI_EVENTLOOP_IO_ERROR, client_error);
}

int
main(int argc, char *argv[])
{
	int fd;

	base_eventloop = mowgli_eventloop_create();

#ifdef DEBUG
	mowgli_timer_add(base_eventloop, "timer_tick", timer_tick, NULL, 1);
#endif

	fd = setup_listener();

	listener = mowgli_pollable_create(base_eventloop, fd, NULL);
	mowgli_pollable_set_nonblocking(listener, true);
	mowgli_pollable_setselect(base_eventloop, listener, MOWGLI_EVENTLOOP_IO_READ, accept_client);

	mowgli_eventloop_run(base_eventloop);

	mowgli_eventloop_destroy(base_eventloop);

	return EXIT_SUCCESS;
}
