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
 * - Return 0 in your error callback if you have nothing to report (non-fatal error or no error)
 * - Return -1 in your callback if you have a fatal error
 * - Return the length of bytes written or read, similar to the semantics of
 *   the read(3) or write(3) calls, if you are a read/write callback.
 *
 *  These are just default implementations, you can change them to suit your needs.
 */

static mowgli_heap_t *vio_heap = NULL;

/* Change these to suit your needs for new VIO objects */
mowgli_vio_ops_t mowgli_vio_default_ops = {
	.socket = mowgli_vio_default_socket,
	.listen = mowgli_vio_default_listen,
	.accept = mowgli_vio_default_accept,
	.connect = mowgli_vio_default_connect,
	.read = mowgli_vio_default_read,
	.write = mowgli_vio_default_write,
	.error = mowgli_vio_default_error,
	.close = mowgli_vio_default_close,
};

mowgli_vio_t * mowgli_vio_create(void *userdata)
{
	mowgli_vio_t *vio;

	if (!vio_heap)
		vio_heap = mowgli_heap_create(sizeof(mowgli_vio_t), 16, BH_NOW);

	vio = mowgli_heap_alloc(vio_heap);

	vio->fd = -1;

	vio->flags = 0;

	/* Default ops */
	vio->ops = mowgli_vio_default_ops;

	vio->userdata = userdata;

	return vio;
}

int mowgli_vio_pollable_create(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop)
{
	return_val_if_fail(vio->fd >= 0, -1);

	vio->io = mowgli_pollable_create(eventloop, vio->fd, vio->userdata);
	vio->eventloop = eventloop;

	/* You're probably going to want this */
	mowgli_pollable_set_nonblocking(vio->io, true);

	return 0;
}

void mowgli_vio_pollable_destroy(mowgli_vio_t *vio)
{
	return_if_fail(vio->io != NULL);

	mowgli_pollable_destroy(vio->eventloop, vio->io);
}

void mowgli_vio_destroy(mowgli_vio_t *vio)
{
	mowgli_vio_pollable_destroy(vio);
	mowgli_heap_free(vio_heap, vio);
}

int mowgli_vio_default_socket(mowgli_vio_t *vio, int family, int type, int proto)
{
	int fd;

	vio->error.op = MOWGLI_VIO_ERR_OP_SOCKET;

	/* We can't call socket with AF_UNSPEC on most platforms >_> */
	if (family == AF_UNSPEC)
		family = AF_INET6;	/* This is fine, IPv4 will still work via a 6to4 mapping */

	if ((fd = socket(family, type, proto)) == -1)
		MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);

	vio->fd = fd;

	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCONNECTING;
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCLOSED;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_listen(mowgli_vio_t *vio, int backlog)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_LISTEN;

	if (listen(vio->fd, backlog) < 0)
		MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
	
	vio->flags |= MOWGLI_VIO_FLAGS_ISSERVER;
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCLIENT;
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCLOSED;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio)
{
	int fd;

	vio->error.op = MOWGLI_VIO_ERR_OP_ACCEPT;

	if (!newvio)
	{
		const char errstr[] = "accept not called with valid new VIO object";
		vio->error.type = MOWGLI_VIO_ERR_API;
		mowgli_strlcpy(vio->error.string, errstr, sizeof(errstr));
		return mowgli_vio_error(vio);
	}

	if ((fd = accept(vio->fd, newvio->addr, &(newvio->addrlen))) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
		}
		else
		{
			return 0;
		}
	}

	newvio->fd = fd;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_connect(mowgli_vio_t *vio)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	if (connect(vio->fd, vio->addr, vio->addrlen) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
		}
		else
		{
			return 0;
		}
	}

	vio->flags |= MOWGLI_VIO_FLAGS_ISCLIENT;
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISSERVER;
	vio->flags |= MOWGLI_VIO_FLAGS_ISCONNECTING;
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCLOSED;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_READ;

	if ((ret = (int)recv(vio->fd, buffer, len, 0)) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
		}
		else if (errno != 0)
		{
			return 0;
		}

		if (ret == 0)
		{
			vio->error.type = MOWGLI_VIO_ERR_REMOTE_HANGUP;
			mowgli_strlcpy(vio->error.string, "Remote host closed the socket", sizeof(vio->error.string));

			vio->flags &= ~MOWGLI_VIO_FLAGS_ISCONNECTING;
			vio->flags |= MOWGLI_VIO_FLAGS_ISCLOSED;

			return mowgli_vio_error(vio);
		}
	}

	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCONNECTING;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_write(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_WRITE;

	if ((ret = (int)send(vio->fd, buffer, len, 0)) == -1)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
		}
		else
		{
			return 0;
		}
	}

	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCONNECTING;

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_error(mowgli_vio_t *vio)
{
	const char *errtype;

	switch (vio->error.op)
	{
	case MOWGLI_VIO_ERR_OP_READ:
		errtype = "Read";
		break;
	case MOWGLI_VIO_ERR_OP_WRITE:
		errtype = "Write";
		break;
	case MOWGLI_VIO_ERR_OP_LISTEN:
		errtype = "Listen";
		break;
	case MOWGLI_VIO_ERR_OP_ACCEPT:
		errtype = "Accept";
		break;
	case MOWGLI_VIO_ERR_OP_CONNECT:
		errtype = "Connect";
		break;
	case MOWGLI_VIO_ERR_OP_SOCKET:
		errtype = "Socket";
		break;
	case MOWGLI_VIO_ERR_OP_OTHER:
		errtype = "Application";
		break;
	default:
		errtype = "Generic";
	}

	fprintf(stderr, "%s error: %s\n", errtype, vio->error.string);

	mowgli_vio_close(vio);

	return -1;
}

int mowgli_vio_default_close(mowgli_vio_t *vio)
{
	vio->flags &= ~MOWGLI_VIO_FLAGS_ISCONNECTING;
	vio->flags |= MOWGLI_VIO_FLAGS_ISCLOSED;
	close(vio->fd);
	return 0;
}

