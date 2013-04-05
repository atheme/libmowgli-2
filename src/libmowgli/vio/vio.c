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
mowgli_vio_ops_t mowgli_vio_default_ops =
{
	.socket = mowgli_vio_default_socket,
	.bind = mowgli_vio_default_bind,
	.listen = mowgli_vio_default_listen,
	.accept = mowgli_vio_default_accept,
	.reuseaddr = mowgli_vio_default_reuseaddr,
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

/* Null ops */
mowgli_vio_evops_t mowgli_vio_default_evops =
{
	.read_cb = NULL,
	.write_cb = NULL
};

/* mowgli_vio_create - create a VIO object on the heap
 *
 * inputs - userdata for the VIO object
 * outputs - a VIO object
 */
mowgli_vio_t *
mowgli_vio_create(void *userdata)
{
	mowgli_vio_t *vio;

	if (!vio_heap)
		vio_heap = mowgli_heap_create(sizeof(mowgli_vio_t), 64, BH_NOW);

	vio = mowgli_heap_alloc(vio_heap);

	mowgli_vio_init(vio, userdata);

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISONHEAP, true);

	return vio;
}

/* mowgli_vio_init - initalise a VIO object previously allocated
 * only use this if you know what you're doing, otherwise use mowgli_vio_create
 *
 * inputs - VIO object, userdata
 * outputs - None
 */
void
mowgli_vio_init(mowgli_vio_t *vio, void *userdata)
{
	return_if_fail(vio);

	vio->io.fd = -1;

	vio->flags = 0;

	/* Default ops */
	vio->ops = &mowgli_vio_default_ops;

	vio->userdata = userdata;
}

/* mowgli_vio_eventloop_attach - attach a VIO object to an eventloop
 *
 * inputs - VIO object, eventloop, ops to use for the eventloop (optional but recommended)
 * outputs - None
 */
void
mowgli_vio_eventloop_attach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop, mowgli_vio_evops_t *evops)
{
	return_if_fail(vio);
	return_if_fail(eventloop);

	const int fd = vio->io.fd;

	/* Check for previous attachment */
	if (vio->eventloop)
	{
		mowgli_log("VIO object [%p] is already attached to eventloop [%p]; attempted to attach new eventloop [%p]", (void *) vio, (void *) vio->eventloop, (void *) eventloop);
		return;
	}

	if ((vio->io.e = mowgli_pollable_create(eventloop, fd, vio->userdata)) != NULL)
	{
		vio->eventloop = eventloop;

		/* You're probably going to want this */
		mowgli_pollable_set_nonblocking(vio->io.e, true);

		if (evops)
			vio->evops = evops;
		else
			/* Default NULL ops */
			vio->evops = &mowgli_vio_default_evops;
	}
	else
	{
		mowgli_log("Unable to create pollable with VIO object [%p], expect problems.", (void *) vio);
		vio->io.fd = fd;/* May have been clobbered */
	}
}

/* mowgli_vio_eventloop_detach - detach VIO object from eventloop
 *
 * inputs - VIO object
 * output - None
 */
void
mowgli_vio_eventloop_detach(mowgli_vio_t *vio)
{
	const int fd = mowgli_vio_getfd(vio);

	return_if_fail(fd != -1);

	return_if_fail(vio != NULL);
	return_if_fail(vio->io.e != NULL);
	return_if_fail(vio->eventloop != NULL);

	mowgli_pollable_destroy(vio->eventloop, vio->io.e);

	vio->eventloop = NULL;
	vio->io.fd = fd;
}

/* mowgli_vio_destroy - eliminate a VIO object
 *
 * inputs - VIO object
 * output - None
 */
void
mowgli_vio_destroy(mowgli_vio_t *vio)
{
	return_if_fail(vio);

	if (vio->eventloop != NULL)
		mowgli_vio_eventloop_detach(vio);

	if (!MOWGLI_VIO_IS_CLOSED(vio))
		mowgli_vio_close(vio);

	if (mowgli_vio_hasflag(vio, MOWGLI_VIO_FLAGS_ISONHEAP))
		mowgli_heap_free(vio_heap, vio);
}

/* mowgli_vio_err_errcode - Signal an error using the specified char *foo(int err) callback
 * Usually the callback can be strerror, but it can use anything else that fits the mould.
 *
 * inputs - VIO object, callback, error code.
 * outputs - Error code from mowgli_vio_error function, any output to terminal/log files/etc.
 */
int
mowgli_vio_err_errcode(mowgli_vio_t *vio, char *(*int_to_error)(int), int errcode)
{
	return_val_if_fail(vio, -255);

	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	mowgli_strlcpy(vio->error.string, int_to_error(errcode), sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

/* mowgli_vio_err_sslerrcode - Signal an SSL error
 *
 * inputs - VIO object, error code
 * outputs - Error code from mowgli_vio_error function, any output to terminal/log files/etc.
 */
#ifdef HAVE_OPENSSL

int
mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, unsigned long int errcode)
{
	return_val_if_fail(vio, -255);

	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	ERR_error_string_n(errcode, vio->error.string, sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

#else

int
mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, unsigned long int errcode)
{
	return_if_fail(vio);

	vio->error.type = MOWGLI_VIO_ERR_ERRCODE;
	vio->error.code = errcode;
	mowgli_strlcpy(vio->error.string, "Unknown SSL error", sizeof(vio->error.string));
	return mowgli_vio_error(vio);
}

#endif
