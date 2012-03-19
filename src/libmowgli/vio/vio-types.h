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

#ifndef __MOWGLI_VIO_TYPES_H__
#define __MOWGLI_VIO_TYPES_H__

/* vio.c */
typedef struct _mowgli_vio mowgli_vio_t;

typedef enum {
	MOWGLI_VIO_ERR_NONE,
	MOWGLI_VIO_ERR_REMOTE_HANGUP,
	MOWGLI_VIO_ERR_ERRCODE,
	MOWGLI_VIO_ERR_API,
	MOWGLI_VIO_ERR_CUSTOM,
} mowgli_vio_error_type_t;

typedef enum {
	MOWGLI_VIO_ERR_OP_NONE,
	MOWGLI_VIO_ERR_OP_SOCKET,
	MOWGLI_VIO_ERR_OP_LISTEN,
	MOWGLI_VIO_ERR_OP_ACCEPT,
	MOWGLI_VIO_ERR_OP_CONNECT,
	MOWGLI_VIO_ERR_OP_READ,
	MOWGLI_VIO_ERR_OP_WRITE,
	MOWGLI_VIO_ERR_OP_BIND,
	MOWGLI_VIO_ERR_OP_OTHER,
} mowgli_vio_error_op_t;

typedef struct _mowgli_vio_error {
	mowgli_vio_error_op_t op;
	mowgli_vio_error_type_t type;
	int code;
	char string[128];
} mowgli_vio_error_t;

typedef struct _mowgli_vio_sockaddr {
	struct sockaddr *addr;
	socklen_t addrlen;
} mowgli_vio_sockaddr_t;

typedef int mowgli_vio_func_t(mowgli_vio_t *);
typedef int mowgli_vio_bind_func_t(mowgli_vio_t *, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_rw_func_t(mowgli_vio_t *, void *, size_t);
typedef int mowgli_vio_sr_func_t(mowgli_vio_t *, void *, size_t, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_connect_func_t(mowgli_vio_t *);
typedef int mowgli_vio_accept_func_t(mowgli_vio_t *, mowgli_vio_t *);
typedef int mowgli_vio_listen_func_t(mowgli_vio_t *, int);
typedef int mowgli_vio_socket_func_t(mowgli_vio_t *, int, int, int);

typedef struct {
	mowgli_vio_socket_func_t *socket;
	mowgli_vio_bind_func_t *bind;
	mowgli_vio_connect_func_t *connect;
	mowgli_vio_listen_func_t *listen;
	mowgli_vio_accept_func_t *accept;
	mowgli_vio_rw_func_t *read;
	mowgli_vio_rw_func_t *write;
	mowgli_vio_sr_func_t *sendto;
	mowgli_vio_sr_func_t *recvfrom;
	mowgli_vio_func_t *error;
	mowgli_vio_func_t *close;
} mowgli_vio_ops_t;

struct _mowgli_vio {
	mowgli_vio_ops_t ops;

	mowgli_eventloop_io_t *io;
	mowgli_descriptor_t fd;

	/* Some jackass could attach us to multiple event loops I guess */
	mowgli_list_t eventloops;

	mowgli_vio_sockaddr_t addr;

	mowgli_vio_error_t error;

	int flags;

	void *userdata;
	void *privdata;
};

typedef struct _mowgli_vio_ssl_settings {
	char cert_path[FILENAME_MAX];
	int ssl_version;
	bool strict_checking;
} mowgli_vio_ssl_settings_t;

#endif

