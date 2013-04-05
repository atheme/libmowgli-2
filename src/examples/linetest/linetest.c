/*
 * libmowgli: A collection of useful routines for programming.
 * linetest.c: Testing of the linebuffer
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

typedef struct
{
	mowgli_linebuf_t *linebuf;
} client_t;

void eat_line(mowgli_linebuf_t *linebuf, char *line, size_t len, void *userdata);

void
write_line(mowgli_linebuf_t *linebuf, char *buf, size_t len)
{
	printf("-> %s\n", buf);
	mowgli_linebuf_write(linebuf, buf, len);
}

client_t *
create_client(const char *server, const char *port, const char *nick, const char *user, const char *realname)
{
	client_t *client;
	struct addrinfo hints, *res;

	bool use_ssl = false;
	mowgli_vio_sockaddr_t addr;
	int ret;

	mowgli_linebuf_t *linebuf;

	if (*port == '+')
	{
		port++;
		use_ssl = true;
	}

	client = mowgli_alloc(sizeof(client_t));

	linebuf = mowgli_linebuf_create(eat_line, client);
	client->linebuf = linebuf;

	/* Do name res */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(server, port, &hints, &res)) != 0)
	{
		linebuf->vio->error.op = MOWGLI_VIO_ERR_OP_OTHER;
		linebuf->vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
		linebuf->vio->error.code = ret;
		mowgli_strlcpy(linebuf->vio->error.string, gai_strerror(ret), sizeof(linebuf->vio->error.string));
		mowgli_vio_error(linebuf->vio);
		return NULL;
	}

	/* Wrap the VIO object */
	if (use_ssl)
		if (mowgli_vio_openssl_setssl(linebuf->vio, NULL, NULL) != 0)
			return NULL;

	/* We have to have a socket before starting the linebuf */

	if (mowgli_vio_socket(linebuf->vio, res->ai_family, res->ai_socktype, res->ai_protocol) != 0)
		return NULL;

	/* Attach the linebuf */
	mowgli_linebuf_attach_to_eventloop(linebuf, base_eventloop);

	/* Do the connect */
	if (mowgli_vio_connect(linebuf->vio, mowgli_vio_sockaddr_from_struct(&addr, res->ai_addr, res->ai_addrlen)) != 0)
		return NULL;

	/* Write IRC handshake */
	snprintf(buf, 512, "USER %s * 8 :%s", user, realname);
	write_line(client->linebuf, buf, strlen(buf));

	snprintf(buf, 512, "NICK %s", nick);
	write_line(client->linebuf, buf, strlen(buf));

	return client;
}

void
eat_line(mowgli_linebuf_t *linebuf, char *line, size_t len, void *userdata)
{
	char str[512];

	/* Avoid malicious lines -- servers shouldn't send them */
	if (linebuf->flags & MOWGLI_LINEBUF_LINE_HASNULLCHAR)
		return;

	strncpy(str, line, sizeof(str));
	str[len + 1] = '\0';

	printf("<- %s\n", str);

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

int
main(int argc, const char *argv[])
{
	client_t *client;
	const char *serv, *port;

	if (argc < 3)
	{
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "Usage: %s [server] [(+)port]\n", argv[0]);
		fprintf(stderr, "For SSL, put a + in front of port\n");
		return EXIT_FAILURE;
	}

	base_eventloop = mowgli_eventloop_create();

	serv = argv[1];
	port = argv[2];

	client = create_client(serv, port, "Mowglibot", "Mowglibot", "The libmowgli example bot that does nothing useful");

	if (client == NULL)
		return EXIT_FAILURE;

	mowgli_eventloop_run(base_eventloop);

	mowgli_free(client);
	mowgli_eventloop_destroy(base_eventloop);

	return EXIT_SUCCESS;
}
