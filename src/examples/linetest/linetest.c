/*
 * libmowgli: A collection of useful routines for programming.
 * lineserver.c: Testing of the linebuffer
 *
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>
 * Copyright (c) 2012 Elizabeth J. Myers <elizabeth@sporksmoo.net>
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

void write_line(mowgli_linebuf_t *linebuf, char *buf, size_t len)
{
	printf("> %s\n", buf);
	mowgli_linebuf_write(linebuf, buf, len);
}

client_t * create_client(const char *server, const char *port, const char *nick, const char *user, const char *realname)
{
	client_t *client;

	client = mowgli_alloc(sizeof(client_t));
	client->linebuf = mowgli_linebuf_create(base_eventloop, eat_line, client);
	mowgli_vio_set_tcp(client->linebuf->vio);
	mowgli_linebuf_connect(client->linebuf, server, port);

	/* Write IRC handshake */
	snprintf(buf, 512, "USER %s * 8 :%s", user, realname);
	write_line(client->linebuf, buf, strlen(buf));

	snprintf(buf, 512, "NICK %s", nick);
	write_line(client->linebuf, buf, strlen(buf));

	return client;
}

void eat_line(mowgli_linebuf_t *linebuf, char *line, size_t len, void *userdata)
{
	char str[512];

	printf("Ate line\n");

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

	printf("Creating client...\n");
	client = create_client(argv[1], argv[2], "Mowglibot", "Mowglibot", "The libmowgli example bot that does nothing useful");
	printf("Client created.\n");

	mowgli_eventloop_run(base_eventloop);

	mowgli_free(client);
	mowgli_eventloop_destroy(base_eventloop);

	return EXIT_SUCCESS;
}
