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
	.bind = mowgli_vio_default_bind,
	.listen = mowgli_vio_default_listen,
	.accept = mowgli_vio_default_accept,
	.connect = mowgli_vio_default_connect,
	.read = mowgli_vio_default_read,
	.write = mowgli_vio_default_write,
	.sendto = mowgli_vio_default_sendto,
	.recvfrom = mowgli_vio_default_recvfrom,
	.error = mowgli_vio_default_error,
	.close = mowgli_vio_default_close,
};

mowgli_vio_t * mowgli_vio_create(void *userdata)
{
	mowgli_vio_t *vio;

	if (!vio_heap)
		vio_heap = mowgli_heap_create(sizeof(mowgli_vio_t), 64, BH_NOW);

	vio = mowgli_heap_alloc(vio_heap);

	mowgli_vio_init(vio, userdata);

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISONHEAP, true);

	return vio;
}

void mowgli_vio_init(mowgli_vio_t *vio, void *userdata)
{
	vio->fd = -1;

	vio->flags = 0;

	/* Default ops */
	vio->ops = mowgli_vio_default_ops;

	vio->userdata = userdata;
}

void mowgli_vio_eventloop_attach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop)
{
	return_if_fail(vio->fd > -1);

	vio->io = mowgli_pollable_create(eventloop, vio->fd, vio->userdata);

	/* Blergh */
	mowgli_node_add(eventloop, mowgli_node_create(), &(vio->eventloops));

	/* You're probably going to want this */
	mowgli_pollable_set_nonblocking(vio->io, true);
}

void mowgli_vio_eventloop_detach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop)
{
	mowgli_node_t *n, *tn;

	return_if_fail(vio->io != NULL);

	/* Remove from eventloops list */
	MOWGLI_LIST_FOREACH_SAFE(n, tn, vio->eventloops.head)
	{
		if ((mowgli_eventloop_t *)n->data == eventloop)
			mowgli_node_delete(n, &(vio->eventloops));
	}

	mowgli_pollable_destroy(eventloop, vio->io);
}

void mowgli_vio_destroy(mowgli_vio_t *vio)
{
	mowgli_node_t *n, *tn;

	/* Detach from each eventloop we're attached to */
	MOWGLI_LIST_FOREACH_SAFE(n, tn, vio->eventloops.head)
	{
		mowgli_pollable_destroy(n->data, vio->io);
		mowgli_node_delete(n, &(vio->eventloops));
	}

	if (mowgli_vio_hasflag(vio, MOWGLI_VIO_FLAGS_ISONHEAP))
		mowgli_heap_free(vio_heap, vio);
}

/* Generate a mowgli_sockaddr_t struct */
mowgli_vio_sockaddr_t * mowgli_sockaddr_create(int proto, const char *addr, int port)
{
	struct sockaddr_storage saddr;
	mowgli_vio_sockaddr_t *vsaddr = mowgli_alloc(sizeof(mowgli_vio_sockaddr_t));

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

		memcpy(&vsaddr->addr, &saddr, sizeof(struct sockaddr_in));
		vsaddr->addrlen = sizeof(struct sockaddr_in);
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

		memcpy(&vsaddr->addr, &saddr, sizeof(struct sockaddr_in6));
		vsaddr->addrlen = sizeof(struct sockaddr_in6);
	}
	else
		vsaddr = NULL;

	return vsaddr;
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
