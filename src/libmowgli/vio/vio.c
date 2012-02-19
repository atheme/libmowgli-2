/*
 * libmowgli: A collection of useful routines for programming.
 * vio.c: Virtual I/O subsystem
 *
 * Copyright (c) 2012 Elizabeth J. Myers. All rights reserved.
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

#include "mowgli.h"

/* How the VIO API works:
 *
 * - Return 0 in your error callback if you have a non-fatal error
 * - Return -1 in your callback if you have a fatal error
 * - Return the length of bytes written or read, similar to the semantics of
 *   the read(3) or write(3) calls.
 */

static mowgli_heap_t *vio_heap = NULL;

static inline int mowgli_vio_errcode(mowgli_vio_t *vio, const char *errstr, int errcode)
{
	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	mowgli_strlcpy(vio->error.string, errstr, sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

mowgli_vio_t * mowgli_vio_create(mowgli_descriptor_t fd, void *userdata)
{
	mowgli_vio_t *vio;

	return_val_if_fail(fd >= 0, NULL);

	if (!vio_heap)
		vio_heap = mowgli_heap_create(sizeof(mowgli_vio_t), 16, BH_NOW);

	vio = mowgli_heap_alloc(vio_heap);

	vio->fd = fd;
	vio->userdata = userdata;

	/* Default ops */
	vio->ops.resolve = mowgli_vio_default_resolve;
	vio->ops.connect = mowgli_vio_default_connect;
	vio->ops.read = mowgli_vio_default_read;
	vio->ops.write = mowgli_vio_default_write;
	vio->ops.error = mowgli_vio_default_error;
	vio->ops.close = mowgli_vio_default_close;

	return vio;
}
int mowgli_vio_default_resolve(mowgli_vio_t *vio, char *addr, char *service, void *data)
{
	int ret;
	struct addrinfo hints;
	struct addrinfo *res = data;

	vio->error.op = MOWGLI_VIO_ERR_OP_RESOLVE;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(addr, service, &hints, &res)) != 0)
		return mowgli_vio_errcode(vio, gai_strerror(ret), ret);

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_connect(mowgli_vio_t *vio, char *addr, char *service)
{
	struct addrinfo *res = NULL;
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	if ((ret = mowgli_vio_resolve(vio, addr, service, res)) != 0)
		return ret;

	if ((ret = connect(vio->fd, res->ai_addr, res->ai_addrlen)) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
			return mowgli_vio_errcode(vio, strerror(errno), errno);
	}

	return 0;
}

int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_READ;

	while ((ret = (int)recv(vio->fd, buffer, len, 0)) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			return mowgli_vio_errcode(vio, strerror(errno), errno);
		}
		else if (ret == 0)
		{
			vio->error.type = MOWGLI_VIO_ERR_REMOTE_HANGUP;
			mowgli_strlcpy(vio->error.string, "Remote host closed the socket", sizeof(vio->error.string));
			return mowgli_vio_error(vio);
		}
	}

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_write(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;
	vio->error.op = MOWGLI_VIO_ERR_OP_WRITE;

	/* Interruptable syscalls suck */
	while ((ret = (int)send(vio->fd, buffer, len, 0)) == -1)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
			return mowgli_vio_errcode(vio, strerror(errno), errno);
	}

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_error(mowgli_vio_t *vio)
{
	const char *errtype;

	if (vio->error.op == MOWGLI_VIO_ERR_OP_READ)
		errtype = "Read";
	else if (vio->error.op == MOWGLI_VIO_ERR_OP_WRITE)
		errtype = "Write";
	else /* ??? */
		errtype = "Socket";

	fprintf(stderr, "%s error: %s\n", errtype, vio->error.string);

	mowgli_vio_close(vio);

	return -1;
}

int mowgli_vio_default_close(mowgli_vio_t *vio)
{
	close(vio->fd);
	return 0;
}

