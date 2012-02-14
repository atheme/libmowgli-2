/*
 * libmowgli: A collection of useful routines for programming.
 * lineserver.c: Testing of the linebuffer
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
char buf[512];

typedef struct {
	mowgli_linebuf_t *linebuf;
	bool connected;
} client_t;

void eat_line(mowgli_linebuf_t *linebuf, char *line, size_t len, void *userdata);

client_t * create_client(const char *server, const char *port, const char *nick, const char *user, const char *realname)
{
	int fd, status;
	struct addrinfo hints;
	struct addrinfo *res;
	client_t *client;
	mowgli_eventloop_io_t *io;

	/* Do name resolution */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(server, port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	client = mowgli_alloc(sizeof(client_t));

	client->connected = true;

	io = mowgli_pollable_create(base_eventloop, fd, client);
	
	client->linebuf = mowgli_linebuf_create(base_eventloop, io, eat_line);

	/* Initiate connection */
	if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}

	mowgli_pollable_set_nonblocking(io, true);

	/* Write IRC handshake */
	snprintf(buf, 512, "USER %s * 8 :%s", user, realname);
	mowgli_linebuf_write(client->linebuf, buf, strlen(buf));

	snprintf(buf, 512, "NICK %s", nick);
	mowgli_linebuf_write(client->linebuf, buf, strlen(buf));

	return client;
}

void connection_error(mowgli_linebuf_t *linebuf, mowgli_eventloop_io_dir_t dir)
{
	const char *errtype;
	client_t *client = linebuf->userdata;

	if (dir == MOWGLI_EVENTLOOP_IO_READ)
		errtype = "Read";
	else if (dir == MOWGLI_EVENTLOOP_IO_WRITE)
		errtype = "Write";
	else /* ??? */
		errtype = "Socket";

	if (linebuf->err)
		fprintf(stderr, "%s error: %s\n", errtype, strerror(linebuf->err));
	else if (linebuf->remote_hangup)
		fprintf(stderr, "Remote host closed the socket\n");
	else if (linebuf->read_buffer_full)
		fprintf(stderr, "Recieve buffer exceeded\n");
	else if (linebuf->write_buffer_full)
		fprintf(stderr, "Send buffer exceeded\n");

	client->connected = false;
	mowgli_linebuf_destroy(linebuf);

	exit(EXIT_SUCCESS);
}

void eat_line(mowgli_linebuf_t *linebuf, char *line, size_t len, void *userdata)
{
	char str[512];

	/* Avoid malicious lines -- servers shouldn't send them */
	if (linebuf->line_has_nullchar)
		return;

	strncpy(str, line, sizeof(str));
	str[len + 1] = '\0';

	printf("-> %s\n", str);

	/* Since this is just a basic example, we don't have a real dispatcher :p */
	if (strstr(str, "PING"))
	{
		char *pos = strpbrk(str, ":");
		if (pos)
		{
			char buf[512];
			snprintf(buf, 512, "PONG %s", pos);
			mowgli_linebuf_write(linebuf, buf, strlen(buf));
		}
	}

	return;
}

int main(int argc, const char *argv[])
{
	client_t *client;

	if (argc < 3)
	{
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "Usage: %s [server] [port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	base_eventloop = mowgli_eventloop_create();

	client = create_client(argv[1], argv[2], "Mowglibot", "Mowglibot", "The libmowgli example bot that does nothing useful");
	
	mowgli_eventloop_run(base_eventloop);

	mowgli_free(client);
	mowgli_eventloop_destroy(base_eventloop);

	return EXIT_SUCCESS;
}
