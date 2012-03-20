/*
 * libmowgli: A collection of useful routines for programming.
 * vio-sockets.c: Plain socket I/O routines for VIO.
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

	if (family == SOCK_STREAM)
	{
		mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);
		mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLOSED, false);
	}

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_bind(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_BIND;

	if (bind(vio->fd, (struct sockaddr *)&addr->addr, addr->addrlen) != 0)
		MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);

	memcpy(&vio->addr.addr, &addr->addr, sizeof(struct sockaddr_storage));
	vio->addr.addrlen = addr->addrlen;

	return 0;
}

int mowgli_vio_default_listen(mowgli_vio_t *vio, int backlog)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_LISTEN;

	if (listen(vio->fd, backlog) < 0)
		MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSERVER, true);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLIENT, false);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLOSED, false);

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

	if ((fd = accept(vio->fd, (struct sockaddr *)&newvio->addr.addr, &(newvio->addr.addrlen))) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			MOWGLI_VIO_RETURN_ERRCODE(newvio, strerror, errno);
		}
		else
		{
			return 0;
		}
	}

	newvio->fd = fd;

	/* The new VIO object is most certainly not a server */
	mowgli_vio_setflag(newvio, MOWGLI_VIO_FLAGS_ISCLIENT, true);
	mowgli_vio_setflag(newvio, MOWGLI_VIO_FLAGS_ISSERVER, false);

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_connect(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	if (connect(vio->fd, (struct sockaddr *)&addr->addr, addr->addrlen) < 0)
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

	/* XXX -- overwrites if we already used bind -- not terribly concerning as this is
	 * more interesting --Elizabeth */
	memcpy(&vio->addr.addr, &addr->addr, sizeof(struct sockaddr_storage));
	vio->addr.addrlen = addr->addrlen;

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLIENT, true);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSERVER, false);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, true);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLOSED, false);

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_READ;

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);

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

			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLOSED, true);

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

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);

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

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_sendto(mowgli_vio_t *vio, void *buffer, size_t len, mowgli_vio_sockaddr_t *addr)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_WRITE;

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);

	if ((ret = (int)sendto(vio->fd, buffer, len, 0, (struct sockaddr *)&addr->addr, addr->addrlen)) == -1)
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

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int mowgli_vio_default_recvfrom(mowgli_vio_t *vio, void *buffer, size_t len, mowgli_vio_sockaddr_t *addr)
{
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_READ;

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);

	if ((ret = (int)recvfrom(vio->fd, buffer, len, 0, (struct sockaddr *)&addr->addr, &addr->addrlen)) < 0)
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

			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLOSED, true);

			return mowgli_vio_error(vio);
		}
	}

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
	case MOWGLI_VIO_ERR_OP_BIND:
		errtype = "Bind";
		break;
	case MOWGLI_VIO_ERR_OP_OTHER:
		errtype = "Application";
		break;
	default:
		errtype = "Generic/Unknown";
	}

	fprintf(stderr, "%s error: %s\n", errtype, vio->error.string);

	mowgli_vio_close(vio);

	return -1;
}

int mowgli_vio_default_close(mowgli_vio_t *vio)
{
	MOWGLI_VIO_SET_CLOSED(vio);
#ifndef _WIN32
	close(vio->fd);
#else
	closesocket(vio->fd);
#endif
	return 0;
}

int mowgli_vio_default_seek(mowgli_vio_t *vio, long offset, int whence)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_SEEK;
	errno = ENOSYS;
	MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
}

int mowgli_vio_default_tell(mowgli_vio_t *vio)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_TELL;
	errno = ENOSYS;
	MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
}

/* Generate a mowgli_sockaddr_t struct */
mowgli_vio_sockaddr_t * mowgli_vio_sockaddr_create(mowgli_vio_sockaddr_t *naddr, int proto, const char *addr, int port)
{
	struct sockaddr_storage saddr;

	if (naddr == NULL)
		naddr = mowgli_alloc(sizeof(mowgli_vio_sockaddr_t));

	if (proto == AF_INET)
	{
		struct sockaddr_in *addr_in = (struct sockaddr_in *)&saddr;

		addr_in->sin_family = proto;
		addr_in->sin_port = htons(port);
		if (addr != NULL)
		{
			if (inet_pton(proto, addr, &addr_in->sin_addr) != 1)
				mowgli_log("Error with inet_pton!");
		}

		memcpy(&naddr->addr, &saddr, sizeof(struct sockaddr_in));
		naddr->addrlen = sizeof(struct sockaddr_in);
	}
	else if (proto == AF_INET6)
	{
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&saddr;

		addr_in6->sin6_family = proto;
		addr_in6->sin6_port = htons(port);
		if (addr != NULL)
		{
			if (inet_pton(proto, addr, &addr_in6->sin6_addr) != 1)
				mowgli_log("Error with inet_pton!");
		}

		memcpy(&naddr->addr, &saddr, sizeof(struct sockaddr_in6));
		naddr->addrlen = sizeof(struct sockaddr_in6);
	}
	else
		naddr = NULL;

	return naddr;
}

mowgli_vio_sockaddr_t * mowgli_vio_sockaddr_from_struct(mowgli_vio_sockaddr_t *naddr, const void *addr, socklen_t size)
{
	const struct sockaddr_storage *saddr = addr;

	return_val_if_fail(addr != NULL, NULL);
	return_val_if_fail(saddr->ss_family != AF_INET && saddr->ss_family != AF_INET6, NULL);

	if (naddr == NULL)
		naddr = mowgli_alloc(sizeof(mowgli_vio_sockaddr_t));
	memcpy(&naddr->addr, saddr, size);
	naddr->addrlen = size;

	return naddr;
}

int mowgli_vio_sockaddr_info(const mowgli_vio_sockaddr_t *addr, mowgli_vio_sockdata_t *data)
{
	const void *sockptr;
	const struct sockaddr *saddr = (const struct sockaddr *)&addr->addr;

	if (saddr->sa_family == AF_INET)
	{
		const struct sockaddr_in *saddr = (const struct sockaddr_in *)&addr->addr;
		data->port = ntohs(saddr->sin_port);
		sockptr = &saddr->sin_addr;
	}
	else if (saddr->sa_family == AF_INET6)
	{
		const struct sockaddr_in6 *saddr = (const struct sockaddr_in6 *)&addr->addr;
		data->port = ntohs(saddr->sin6_port);
		sockptr = &saddr->sin6_addr;
	}
	else
		return -1;

	if (inet_ntop(saddr->sa_family, sockptr, data->host, sizeof(data->host)) == NULL)
		return -1;

	return 0;
}
