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

/* vio.c */
typedef struct _mowgli_vio mowgli_vio_t;

typedef int mowgli_vio_func_t(mowgli_vio_t *);
typedef int mowgli_vio_socket_func_t(mowgli_vio_t *, int, int);
typedef int mowgli_vio_rw_func_t(mowgli_vio_t *, void *, size_t);
typedef int mowgli_vio_connect_func_t(mowgli_vio_t *, char *, char *);

typedef enum {
	MOWGLI_VIO_ERR_NONE,
	MOWGLI_VIO_ERR_REMOTE_HANGUP,
	MOWGLI_VIO_ERR_BUFFER_FULL,
	MOWGLI_VIO_ERR_ERRCODE,
} mowgli_vio_error_type_t;

typedef enum {
	MOWGLI_VIO_ERR_OP_NONE,
	MOWGLI_VIO_ERR_OP_SOCKET,
	MOWGLI_VIO_ERR_OP_CONNECT,
	MOWGLI_VIO_ERR_OP_READ,
	MOWGLI_VIO_ERR_OP_WRITE,
	MOWGLI_VIO_ERR_OP_OTHER,
} mowgli_vio_error_op_t;

typedef struct _mowgli_vio_error {
	mowgli_vio_error_op_t op;
	mowgli_vio_error_type_t type;
	int code;
	char string[128];
} mowgli_vio_error_t;

#define MOWGLI_VIO_RETURN_ERRCODE(v, s, e) {    \
		v->error.type = MOWGLI_VIO_ERR_ERRCODE; \
		v->error.code = e;		      \
		mowgli_strlcpy(v->error.string, s(e), sizeof(vio->error.string)); \
		return mowgli_vio_error(vio); }

typedef struct _mowgli_vio_ops {
	mowgli_vio_socket_func_t *socket;
	mowgli_vio_connect_func_t *connect;
	mowgli_vio_rw_func_t *read;
	mowgli_vio_rw_func_t *write;
	mowgli_vio_func_t *error;
	mowgli_vio_func_t *close;
} mowgli_vio_ops_t;

typedef struct _mowgli_vio {
	mowgli_vio_ops_t ops;

	mowgli_descriptor_t fd;
	int sock_family;
	int sock_type;
	int sock_proto;

	mowgli_vio_error_t error;

	void *privdata;
	void *userdata;
} mowgli_vio_t;

extern mowgli_vio_t * mowgli_vio_create(void *userdata);
extern void mowgli_vio_destroy(mowgli_vio_t *vio);

extern int mowgli_vio_default_socket(mowgli_vio_t *vio, int domain, int type);
extern int mowgli_vio_default_connect(mowgli_vio_t *vio, char *addr, char *service);
extern int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_default_write(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_default_error(mowgli_vio_t *vio);
extern int mowgli_vio_default_close(mowgli_vio_t *vio);

extern void mowgli_vio_openssl_setssl(mowgli_vio_t *vio, int flags);

#define mowgli_vio_set_op(vio, operation, func) (vio)->op.operation = func;

static inline int mowgli_vio_socket(mowgli_vio_t *vio, int family, int type)
{
	return vio->ops.socket(vio, family, type);
}

static inline int mowgli_vio_connect(mowgli_vio_t *vio, char *addr, char *service)
{
	return vio->ops.connect(vio, addr, service);
}

static inline int mowgli_vio_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	return vio->ops.read(vio, buffer, len);
}

static inline int mowgli_vio_write(mowgli_vio_t *vio, void *buffer, size_t len)
{
	return vio->ops.write(vio, buffer, len);
}

static inline int mowgli_vio_error(mowgli_vio_t *vio)
{
	return vio->ops.error(vio);
}

static inline int mowgli_vio_close(mowgli_vio_t *vio)
{
	return vio->ops.close(vio);
}

#endif
