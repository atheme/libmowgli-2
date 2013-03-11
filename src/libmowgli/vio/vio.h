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

/* Error type */
typedef enum
{
	MOWGLI_VIO_ERR_NONE = 0,/* Wat, no error. */
	MOWGLI_VIO_ERR_REMOTE_HANGUP,	/* Remote end hung up on us, how rude */
	MOWGLI_VIO_ERR_ERRCODE,	/* An errno error or something like that */
	MOWGLI_VIO_ERR_API,	/* Programmer was a dumbass */
	MOWGLI_VIO_ERR_CUSTOM,	/* Use this for custom errors */
} mowgli_vio_error_type_t;

/* Errors with specific functions correspondng to the VIO op it's named
 * after */
typedef enum
{
	MOWGLI_VIO_ERR_OP_NONE = 0,	/* Wat. */
	MOWGLI_VIO_ERR_OP_SOCKET,
	MOWGLI_VIO_ERR_OP_LISTEN,
	MOWGLI_VIO_ERR_OP_ACCEPT,
	MOWGLI_VIO_ERR_OP_REUSEADDR,
	MOWGLI_VIO_ERR_OP_CONNECT,
	MOWGLI_VIO_ERR_OP_READ,
	MOWGLI_VIO_ERR_OP_WRITE,
	MOWGLI_VIO_ERR_OP_BIND,
	MOWGLI_VIO_ERR_OP_SEEK,
	MOWGLI_VIO_ERR_OP_TELL,
	MOWGLI_VIO_ERR_OP_OTHER,
} mowgli_vio_error_op_t;

typedef struct _mowgli_vio_error
{
	mowgli_vio_error_op_t op;
	mowgli_vio_error_type_t type;
	unsigned long code;	/* Unsigned long for OpenSSL fuckery */
	char string[128];	/* Friendly name for error */
} mowgli_vio_error_t;

/* Custom sockaddr member to have a uniform sockaddr as opposed to the
 * bullshit in the Berkeley sockets API with struct sockaddr/struct
 * sockaddr_storage/struct sockaddr_in/struct sockaddr_in6 and associated
 * API inconsistency and braindamage
 */
typedef struct _mowgli_vio_sockaddr
{
	struct sockaddr_storage addr;

	socklen_t addrlen;
} mowgli_vio_sockaddr_t;

#ifndef INET6_ADDRSTRLEN
# define INET6_ADDRSTRLEN 46	/* Good enough I tell you */
#endif

/* Socket data */
typedef struct _mowgli_vio_sockdata
{
	char host[INET6_ADDRSTRLEN];	/* max length of IPv6 address */
	uint16_t port;
} mowgli_vio_sockdata_t;

/* Various typedefs bleh */
typedef int mowgli_vio_func_t (mowgli_vio_t *);
typedef int mowgli_vio_bind_connect_func_t (mowgli_vio_t *, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_read_func_t (mowgli_vio_t *, void *, size_t);
typedef int mowgli_vio_write_func_t (mowgli_vio_t *, const void *, size_t);
typedef int mowgli_vio_sendto_func_t (mowgli_vio_t *, const void *, size_t, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_recvfrom_func_t (mowgli_vio_t *, void *, size_t, mowgli_vio_sockaddr_t *);
typedef int mowgli_vio_connect_func_t (mowgli_vio_t *);
typedef int mowgli_vio_accept_func_t (mowgli_vio_t *, mowgli_vio_t *);
typedef int mowgli_vio_listen_func_t (mowgli_vio_t *, int);
typedef int mowgli_vio_socket_func_t (mowgli_vio_t *, int, int, int);
typedef int mowgli_vio_seek_func_t (mowgli_vio_t *, long, int);

/* These are workalikes vis-a-vis the Berkeley sockets API */
typedef struct
{
	mowgli_vio_socket_func_t *socket;
	mowgli_vio_bind_connect_func_t *bind;
	mowgli_vio_bind_connect_func_t *connect;
	mowgli_vio_listen_func_t *listen;
	mowgli_vio_accept_func_t *accept;
	mowgli_vio_func_t *reuseaddr;
	mowgli_vio_read_func_t *read;
	mowgli_vio_write_func_t *write;
	mowgli_vio_sendto_func_t *sendto;
	mowgli_vio_recvfrom_func_t *recvfrom;
	mowgli_vio_func_t *error;
	mowgli_vio_func_t *close;
	mowgli_vio_seek_func_t *seek;
	mowgli_vio_func_t *tell;
} mowgli_vio_ops_t;

/* Callbacks for eventloop stuff */
typedef struct
{
	mowgli_eventloop_io_cb_t *read_cb;
	mowgli_eventloop_io_cb_t *write_cb;
} mowgli_vio_evops_t;

struct _mowgli_vio
{
	mowgli_vio_ops_t *ops;	/* VIO operations */
	mowgli_vio_evops_t *evops;	/* Eventloop operations */

	/* eventloop IO object or descriptor
	 * If the eventloop member is non-null use io
	 * else use fd
	 */
	union
	{
		mowgli_eventloop_io_t *e;
		mowgli_descriptor_t fd;
	} io;

	/* Eventloop object we're attached to */
	mowgli_eventloop_t *eventloop;

	/* struct sockaddr portable workalike -- usage is
	 * context specific.
	 *
	 * For accept()'ed VIO objects, this is the client's
	 * struct sockaddr stuff, for connect()'ed objects it
	 * is the remote end's sockaddr (even with previous call
	 * to bind, for bind()'ed objects it is the sockaddr passed
	 * to bind
	 */
	mowgli_vio_sockaddr_t addr;

	mowgli_vio_error_t error;	/* Error information lives here */

	unsigned int flags;	/* Connection flags */

	void *userdata;	/* User data for VIO object */
	void *privdata;	/* Private data for stuff like SSL */
};

/* SSL settings... members subject to change */
typedef struct _mowgli_vio_ssl_settings
{
	const char *cert_path;
	const char *privatekey_path;
	int ssl_version;
	int (*password_func)(char *, int, int, void *);
	int (*verify_func)(int, void *);
} mowgli_vio_ssl_settings_t;

/* Flags */
#define MOWGLI_VIO_FLAGS_ISCONNECTING 0x00001
#define MOWGLI_VIO_FLAGS_ISSSLCONNECTING 0x00002
#define MOWGLI_VIO_FLAGS_ISCLOSED 0x00004

#define MOWGLI_VIO_FLAGS_ISCLIENT 0x00008
#define MOWGLI_VIO_FLAGS_ISSERVER 0x00010

#define MOWGLI_VIO_FLAGS_ISONHEAP 0x00020

#define MOWGLI_VIO_FLAGS_NEEDREAD 0x00040
#define MOWGLI_VIO_FLAGS_NEEDWRITE 0x00080

/* SSL flags */
#define MOWGLI_VIO_SSLFLAGS_SSLV2 0x00001
#define MOWGLI_VIO_SSLFLAGS_SSLV3 0x00002
#define MOWGLI_VIO_SSLFLAGS_TLSV10 0x00004
#define MOWGLI_VIO_SSLFLAGS_TLSV11 0x00008
#define MOWGLI_VIO_SSLFLAGS_TLSV12 0x00010

/* Flag setting/getting */
static inline bool
mowgli_vio_hasflag(mowgli_vio_t *vio, int flag)
{
	return (vio->flags & flag) != 0 ? true : false;
}

static inline void
mowgli_vio_setflag(mowgli_vio_t *vio, int flag, bool setting)
{
	if (setting)
		vio->flags |= flag;
	else
		vio->flags &= ~flag;
}

/* Get file descriptor */
static inline mowgli_descriptor_t
mowgli_vio_getfd(mowgli_vio_t *vio)
{
	return_val_if_fail(vio, -1);

	if (vio->eventloop)
	{
		mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(vio->io.e);

		if (pollable)
			return pollable->fd;
	}

	return vio->io.fd;
}

/* Macros */
#define MOWGLI_VIO_SET_CLOSED(v) \
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCONNECTING, false); \
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISCLOSED, true);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, false);	\
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_NEEDREAD, false); \
	mowgli_vio_setflag(v, MOWGLI_VIO_FLAGS_NEEDWRITE, false)

#define MOWGLI_VIO_IS_CLOSED(v) mowgli_vio_hasflag(v, MOWGLI_VIO_FLAGS_ISCLOSED)

#define MOWGLI_VIO_SETREAD(vio)	\
	if (vio->eventloop && vio->io.e && vio->evops && vio->evops->read_cb) \
	{ \
		mowgli_pollable_setselect(vio->eventloop, vio->io.e, MOWGLI_EVENTLOOP_IO_READ, vio->evops->read_cb); \
	}

#define MOWGLI_VIO_SETWRITE(vio) \
	if (vio->eventloop && vio->io.e && vio->evops && vio->evops->write_cb) \
	{ \
		mowgli_pollable_setselect(vio->eventloop, vio->io.e, MOWGLI_EVENTLOOP_IO_WRITE, vio->evops->write_cb); \
	}

#define MOWGLI_VIO_UNSETWRITE(vio) \
	if (vio->eventloop && vio->io.e) \
	{ \
		mowgli_pollable_setselect(vio->eventloop, vio->io.e, MOWGLI_EVENTLOOP_IO_WRITE, NULL); \
	}

/* Decls */
extern mowgli_vio_t *mowgli_vio_create(void *userdata);
extern void mowgli_vio_init(mowgli_vio_t *vio, void *userdata);
extern void mowgli_vio_destroy(mowgli_vio_t *vio);

extern void mowgli_vio_eventloop_attach(mowgli_vio_t *vio, mowgli_eventloop_t *eventloop, mowgli_vio_evops_t *evops);
extern void mowgli_vio_eventloop_detach(mowgli_vio_t *vio);

extern mowgli_vio_sockaddr_t *mowgli_vio_sockaddr_create(mowgli_vio_sockaddr_t *naddr, int proto, const char *addr, int port);
extern mowgli_vio_sockaddr_t *mowgli_vio_sockaddr_from_struct(mowgli_vio_sockaddr_t *naddr, const void *addr, socklen_t size);
extern int mowgli_vio_sockaddr_info(const mowgli_vio_sockaddr_t *addr, mowgli_vio_sockdata_t *data);

extern int mowgli_vio_default_socket(mowgli_vio_t *vio, int family, int type, int proto);
extern int mowgli_vio_default_bind(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_default_listen(mowgli_vio_t *vio, int backlog);
extern int mowgli_vio_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio);
extern int mowgli_vio_default_reuseaddr(mowgli_vio_t *vio);
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
extern int mowgli_vio_err_sslerrcode(mowgli_vio_t *vio, unsigned long int errcode);

extern int mowgli_vio_openssl_setssl(mowgli_vio_t *vio, mowgli_vio_ssl_settings_t *settings, mowgli_vio_ops_t *ops);

/* These are void ptr's so they can be null ops if SSL isn't available */
extern void *mowgli_vio_openssl_getsslhandle(mowgli_vio_t *vio);
extern void *mowgli_vio_openssl_getsslcontext(mowgli_vio_t *vio);

#ifdef HAVE_OPENSSL
extern int mowgli_vio_openssl_default_connect(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr);
extern int mowgli_vio_openssl_default_listen(mowgli_vio_t *vio, int backlog);
extern int mowgli_vio_openssl_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio);
extern int mowgli_vio_openssl_default_read(mowgli_vio_t *vio, void *buffer, size_t len);
extern int mowgli_vio_openssl_default_write(mowgli_vio_t *vio, const void *buffer, size_t len);
extern int mowgli_vio_openssl_default_close(mowgli_vio_t *vio);

#else
# define NOSSLSUPPORT { mowgli_log("Attempting to use default OpenSSL op with no SSL support; this will not work!"); return -255; }
static inline int mowgli_vio_openssl_default_connect(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr) NOSSLSUPPORT
static inline int mowgli_vio_openssl_default_listen(mowgli_vio_t *vio, int backlog) NOSSLSUPPORT
static inline int mowgli_vio_openssl_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio) NOSSLSUPPORT
static inline int mowgli_vio_openssl_default_read(mowgli_vio_t *vio, void *buffer, size_t len) NOSSLSUPPORT
static inline int mowgli_vio_openssl_default_write(mowgli_vio_t *vio, const void *buffer, size_t len) NOSSLSUPPORT
static inline int mowgli_vio_openssl_default_close(mowgli_vio_t *vio) NOSSLSUPPORT
#endif

/* Default ops -- change these if you want something besides the default */
extern mowgli_vio_ops_t mowgli_vio_default_ops;

/* Default evops -- they do nothing unless you change them */
extern mowgli_vio_evops_t mowgli_vio_default_evops;

/* Sundry operations on vio functables */
#define mowgli_vio_ops_set_op(ops, op, func) ops->op = func

/* Wrappers for the VIO ops */
#define mowgli_vio_socket(vio, ...) vio->ops->socket(vio, __VA_ARGS__)
#define mowgli_vio_listen(vio, ...) vio->ops->listen(vio, __VA_ARGS__)
#define mowgli_vio_bind(vio, ...) vio->ops->bind(vio, __VA_ARGS__)
#define mowgli_vio_accept(vio, ...) vio->ops->accept(vio, __VA_ARGS__)
#define mowgli_vio_reuseaddr(vio) vio->ops->reuseaddr(vio)
#define mowgli_vio_connect(vio, ...) vio->ops->connect(vio, __VA_ARGS__)
#define mowgli_vio_read(vio, ...) vio->ops->read(vio, __VA_ARGS__)
#define mowgli_vio_write(vio, ...) vio->ops->write(vio, __VA_ARGS__)
#define mowgli_vio_sendto(vio, ...) vio->ops->sendto(vio, __VA_ARGS__)
#define mowgli_vio_recvfrom(vio, ...) vio->ops->recvfrom(vio, __VA_ARGS__)
#define mowgli_vio_error(vio) vio->ops->error(vio)
#define mowgli_vio_close(vio) vio->ops->close(vio)
#define mowgli_vio_seek(vio, ...) vio->ops->seek(vio, __VA_ARGS__)
#define mowgli_vio_tell(vio) vio->ops->tell(vio)

#endif
