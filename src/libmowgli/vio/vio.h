/*
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
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MOWGLI_VIO_H__
#define __MOWGLI_VIO_H__

#include "vio/vio-types.h"

/* Flags */
#define MOWGLI_VIO_FLAGS_ISCONNECTING		0x00001
#define MOWGLI_VIO_FLAGS_ISSSLCONNECTING	0x00002
#define MOWGLI_VIO_FLAGS_ISCLOSED		0x00004

#define MOWGLI_VIO_FLAGS_ISCLIENT		0x00008
#define MOWGLI_VIO_FLAGS_ISSERVER		0x00010

#define MOWGLI_VIO_FLAGS_ISONHEAP		0x00020

/* SSL flags */
#define MOWGLI_VIO_SSLFLAGS_SSLV2		0x00001
#define MOWGLI_VIO_SSLFLAGS_SSLV3		0x00002
#define MOWGLI_VIO_SSLFLAGS_TLSV10		0x00004
#define MOWGLI_VIO_SSLFLAGS_TLSV11		0x00008
#define MOWGLI_VIO_SSLFLAGS_TLSV12		0x00010

static inline bool mowgli_vio_hasflag(mowgli_vio_t *vio, int flag)
{
	return vio->flags & flag ? true : false;
}

static inline void mowgli_vio_setflag(mowgli_vio_t *vio, int flag, bool setting)
{
	if (setting)
		vio->flags |= flag;
	else
		vio->flags &= flag;
}

/* Macros */
#define MOWGLI_VIO_SET_CLOSED(v)					\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCONNECTING, false);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCLOSED, true);		\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, false);

#define MOWGLI_VIO_RETURN_ERRCODE(v, s, e) 					\
{										\
	v->error.type = MOWGLI_VIO_ERR_ERRCODE; 				\
	v->error.code = e;							\
	mowgli_strlcpy(v->error.string, s(e), sizeof((v)->error.string));	\
	return mowgli_vio_error((v)); 						\
}

#ifdef HAVE_OPENSSL

#define MOWGLI_VIO_RETURN_SSLERR_ERRCODE(v, e)					\
{										\
	(v)->error.type = MOWGLI_VIO_ERR_ERRCODE;				\
	(v)->error.code = e;							\
	ERR_error_string_n(e, (v)->error.string, sizeof((v)->error.string));	\
	return mowgli_vio_error(v);						\
}

#else
#	define MOWGLI_VIO_RETURN_SSL_ERRCODE(v, e) MOWGLI_VIO_RETURN_ERRCODE(v, strerror, e)
#endif

/* Decls */
extern mowgli_vio_t * mowgli_vio_create(void *userdata);
extern void mowgli_vio_init(mowgli_vio_t *vio, void *userdata);
extern void mowgli_vio_destroy(mowgli_vio_t *vio);

extern void mowgli_vio_eventloop_attach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop);
extern void mowgli_vio_eventloop_detach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop);

extern mowgli_vio_sockaddr_t * mowgli_sockaddr_create(int proto, const char *addr, int port);
extern int mowgli_vio_sockaddr_info(const mowgli_vio_sockaddr_t *addr, mowgli_vio_sockdata_t *data);

extern int mowgli_vio_default_socket(mowgli_vio_t *vio, int family, int type, int proto);
extern int mowgli_vio_default_bind(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_listen(mowgli_vio_t *vio, int backlog);
extern int mowgli_vio_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio);
extern int mowgli_vio_default_connect(mowgli_vio_t *vio);
extern int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_default_write(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_default_sendto(mowgli_vio_t *vio, void *buffer, size_t len, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_recvfrom(mowgli_vio_t *vio, void *buffer, size_t len, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_error(mowgli_vio_t *vio);
extern int mowgli_vio_default_close(mowgli_vio_t *vio);

extern int mowgli_vio_openssl_setssl(mowgli_vio_t *vio, mowgli_vio_ssl_settings_t *settings);
/* These are void ptr's so they can be null ops if SSL isn't available */
extern void * mowgli_vio_openssl_getsslhandle(mowgli_vio_t *vio);
extern void * mowgli_vio_openssl_getsslcontext(mowgli_vio_t *vio);

extern mowgli_vio_ops_t mowgli_vio_default_ops;

#define mowgli_vio_set_op(vio, op, func) vio->ops.op = func;

/* A tad inelegant... */
#define mowgli_vio_socket(vio, ...)	vio->ops.socket(vio, __VA_ARGS__)
#define mowgli_vio_listen(vio, ...)	vio->ops.listen(vio, __VA_ARGS__)
#define mowgli_vio_bind(vio, ...)	vio->ops.bind(vio, __VA_ARGS__)
#define mowgli_vio_accept(vio, ...)	vio->ops.accept(vio, __VA_ARGS__)
#define mowgli_vio_connect(vio, ...)	vio->ops.connect(vio, __VA_ARGS__)
#define mowgli_vio_read(vio, ...)	vio->ops.read(vio, __VA_ARGS__)
#define mowgli_vio_write(vio, ...)	vio->ops.write(vio, __VA_ARGS__)
#define mowgli_vio_sendto(vio, ...)	vio->ops.sendto(vio, __VA_ARGS__)
#define mowgli_vio_recvfrom(vio, ...)	vio->ops.recvfrom(vio, __VA_ARGS__)
#define mowgli_vio_error(vio)		vio->ops.error(vio);
#define mowgli_vio_close(vio)		vio->ops.close(vio);

#endif

