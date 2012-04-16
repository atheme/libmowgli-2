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
	.seek = mowgli_vio_default_seek,
	.tell = mowgli_vio_default_tell,
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
	vio->io = mowgli_pollable_create(eventloop, vio->fd, vio->userdata);
	if (vio->io != NULL)
	{
		vio->eventloop = eventloop;
		/* You're probably going to want this */
		mowgli_pollable_set_nonblocking(vio->io, true);
	}
	else
		mowgli_log("Unable to create pollable with VIO object [%p], expect problems.", vio);
}

void mowgli_vio_eventloop_detach(mowgli_vio_t *vio)
{
	return_if_fail(vio->io != NULL);
	return_if_fail(vio->eventloop != NULL);

	mowgli_pollable_destroy(vio->eventloop, vio->io);
}

void mowgli_vio_destroy(mowgli_vio_t *vio)
{
	mowgli_vio_eventloop_detach(vio);

	if (mowgli_vio_hasflag(vio, MOWGLI_VIO_FLAGS_ISONHEAP))
		mowgli_heap_free(vio_heap, vio);
}

int mowgli_vio_err_errcode(mowgli_vio_t *vio, char *(*int_to_error)(int), int errcode)
{
	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	mowgli_strlcpy(vio->error.string, int_to_error(errcode), sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

#ifdef HAVE_OPENSSL

int mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, int errcode)
{
	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	ERR_error_string_n(errcode, vio->error.string, sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

#else

int mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, int errcode)
{
	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	mowgli_strlcpy(vio->error.string, "Unknown SSL error", sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

#endif
