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

#ifndef __MOWGLI_VIO_VIO_H__
#define __MOWGLI_VIO_VIO_H__

/* Types and structs */
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
	MOWGLI_VIO_ERR_OP_SEEK,
	MOWGLI_VIO_ERR_OP_TELL,
	MOWGLI_VIO_ERR_OP_OTHER,
} mowgli_vio_error_op_t;

typedef struct _mowgli_vio_error {
	mowgli_vio_error_op_t op;
	mowgli_vio_error_type_t type;
	int code;
	char string[128];
} mowgli_vio_error_t;

typedef struct _mowgli_vio_sockaddr {
	struct sockaddr_storage addr;
	socklen_t addrlen;
} mowgli_vio_sockaddr_t;

typedef struct _mowgli_vio_sockdata {
	char host[39];	/* max length of IPv6 address */
	uint16_t port;
} mowgli_vio_sockdata_t;

typedef int mowgli_vio_func_t(mowgli_vio_t *);
typedef int mowgli_vio_bind_connect_func_t(mowgli_vio_t *, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_read_func_t(mowgli_vio_t *, void *, size_t);
typedef int mowgli_vio_write_func_t(mowgli_vio_t *, const void *, size_t);
typedef int mowgli_vio_sendto_func_t(mowgli_vio_t *, const void *, size_t, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_recvfrom_func_t(mowgli_vio_t *, void *, size_t, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_connect_func_t(mowgli_vio_t *);
typedef int mowgli_vio_accept_func_t(mowgli_vio_t *, mowgli_vio_t *);
typedef int mowgli_vio_listen_func_t(mowgli_vio_t *, int);
typedef int mowgli_vio_socket_func_t(mowgli_vio_t *, int, int, int);
typedef int mowgli_vio_seek_func_t(mowgli_vio_t *, long, int);

typedef struct {
	mowgli_vio_socket_func_t *socket;
	mowgli_vio_bind_connect_func_t *bind;
	mowgli_vio_bind_connect_func_t *connect;
	mowgli_vio_listen_func_t *listen;
	mowgli_vio_accept_func_t *accept;
	mowgli_vio_read_func_t *read;
	mowgli_vio_write_func_t *write;
	mowgli_vio_sendto_func_t *sendto;
	mowgli_vio_recvfrom_func_t *recvfrom;
	mowgli_vio_func_t *error;
	mowgli_vio_func_t *close;
	mowgli_vio_seek_func_t *seek;
	mowgli_vio_func_t *tell;
} mowgli_vio_ops_t;

struct _mowgli_vio {
	mowgli_vio_ops_t ops;

	mowgli_eventloop_io_t *io;
	mowgli_descriptor_t fd;

	mowgli_eventloop_t *eventloop;

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


/* Flags */
#define MOWGLI_VIO_FLAGS_ISCONNECTING		0x00001
#define MOWGLI_VIO_FLAGS_ISSSLCONNECTING	0x00002
#define MOWGLI_VIO_FLAGS_ISCLOSED		0x00004

#define MOWGLI_VIO_FLAGS_ISCLIENT		0x00008
#define MOWGLI_VIO_FLAGS_ISSERVER		0x00010

#define MOWGLI_VIO_FLAGS_ISONHEAP		0x00020

#define MOWGLI_VIO_FLAGS_NEEDREAD		0x00040
#define MOWGLI_VIO_FLAGS_NEEDWRITE		0x00080

/* SSL flags */
#define MOWGLI_VIO_SSLFLAGS_SSLV2		0x00001
#define MOWGLI_VIO_SSLFLAGS_SSLV3		0x00002
#define MOWGLI_VIO_SSLFLAGS_TLSV10		0x00004
#define MOWGLI_VIO_SSLFLAGS_TLSV11		0x00008
#define MOWGLI_VIO_SSLFLAGS_TLSV12		0x00010


/* Flag setting/getting */
static inline bool mowgli_vio_hasflag(mowgli_vio_t *vio, int flag)
{
	return (vio->flags & flag) != 0 ? true : false;
}

static inline void mowgli_vio_setflag(mowgli_vio_t *vio, int flag, bool setting)
{
	if (setting)
		vio->flags |= flag;
	else
		vio->flags &= ~flag;
}


/* Macros */
#define MOWGLI_VIO_SET_CLOSED(v)					\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCONNECTING, false);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCLOSED, true);		\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, false);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_NEEDREAD, false);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_NEEDWRITE, false);


/* Decls */
extern mowgli_vio_t * mowgli_vio_create(void *userdata);
extern void mowgli_vio_init(mowgli_vio_t *vio, void *userdata);
extern void mowgli_vio_destroy(mowgli_vio_t *vio);

extern void mowgli_vio_eventloop_attach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop);
extern void mowgli_vio_eventloop_detach(mowgli_vio_t *vio);

extern mowgli_vio_sockaddr_t * mowgli_vio_sockaddr_create(mowgli_vio_sockaddr_t *naddr, int proto, const char *addr, int port);
extern mowgli_vio_sockaddr_t * mowgli_vio_sockaddr_from_struct(mowgli_vio_sockaddr_t *naddr, const void *addr, socklen_t size);
extern int mowgli_vio_sockaddr_info(const mowgli_vio_sockaddr_t *addr, mowgli_vio_sockdata_t *data);

extern int mowgli_vio_default_socket(mowgli_vio_t *vio, int family, int type, int proto);
extern int mowgli_vio_default_bind(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_listen(mowgli_vio_t *vio, int backlog);
extern int mowgli_vio_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio);
extern int mowgli_vio_default_connect(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_read(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_default_write(mowgli_vio_t *vio, const void *buffer, size_t len);
extern int mowgli_vio_default_sendto(mowgli_vio_t *vio, const void *buffer, size_t len, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_recvfrom(mowgli_vio_t *vio, void *buffer, size_t len, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_error(mowgli_vio_t *vio);
extern int mowgli_vio_default_close(mowgli_vio_t *vio);
extern int mowgli_vio_default_seek(mowgli_vio_t *vio, long offset, int whence);
extern int mowgli_vio_default_tell(mowgli_vio_t *vio);

extern int mowgli_vio_err_errcode(mowgli_vio_t *vio, char *(*int_to_error)(int), int errcode);
extern int mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, int errcode);

extern int mowgli_vio_openssl_setssl(mowgli_vio_t *vio, mowgli_vio_ssl_settings_t *settings);
/* These are void ptr's so they can be null ops if SSL isn't available */
extern void * mowgli_vio_openssl_getsslhandle(mowgli_vio_t *vio);
extern void * mowgli_vio_openssl_getsslcontext(mowgli_vio_t *vio);

extern mowgli_vio_ops_t mowgli_vio_default_ops;


/* Sundry operations on vio functables */
#define mowgli_vio_set_op(vio, op, func) vio->ops.op = func;

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
#define mowgli_vio_seek(vio, ...)	vio->ops.seek(vio, __VA_ARGS__)
#define mowgli_vio_tell(vio)		vio->ops.tell(vio)

#endif

