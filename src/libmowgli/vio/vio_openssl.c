/*
 * libmowgli: A collection of useful routines for programming.
 * vio_openssl.c: OpenSSL routines built on VIO
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

#ifdef HAVE_OPENSSL

#define MOWGLI_VIO_RETURN_SSLERR_ERRCODE(v, e) {	\
	(v)->error.type = MOWGLI_VIO_ERR_ERRCODE;	\
	(v)->error.code = e;			\
	ERR_error_string_n(e, (v)->error.string, sizeof((v)->error.string)); \
	return mowgli_vio_error(v); }

typedef struct {
	SSL *ssl_handle;
	SSL_CTX *ssl_context;
	int flags;
} mowgli_ssl_connection_t;

static int mowgli_vio_openssl_connect(mowgli_vio_t *vio, char *addr, char *service);
static int mowgli_vio_openssl_read(mowgli_vio_t *vio, void *buffer, size_t len);
static int mowgli_vio_openssl_write(mowgli_vio_t *vio, void *buffer, size_t len);
static int mowgli_vio_openssl_close(mowgli_vio_t *vio);

int mowgli_vio_openssl_setssl(mowgli_vio_t *vio, int flags)
{
	/* Allocate */
	vio->privdata = mowgli_alloc(sizeof(mowgli_ssl_connection_t));

	/* Change ops */
	vio->ops.connect = mowgli_vio_openssl_connect;
	vio->ops.read = mowgli_vio_openssl_read;
	vio->ops.write = mowgli_vio_openssl_write;
	vio->ops.close = mowgli_vio_openssl_close;
}

static int mowgli_vio_openssl_connect(mowgli_vio_t *vio, char *addr, char *service)
{
	mowgli_ssl_connection_t *connection = mowgli_alloc(sizeof(mowgli_ssl_connection_t));
	struct addrinfo *res = NULL;
	int ret;

	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	if ((ret = mowgli_vio_resolve(vio, addr, service, res)) != 0)
		return ret;

	if (vio->fd < 0)
	{
		if ((ret = mowgli_vio_socket(vio, vio->sock_family, vio->sock_type)) != 0)
			return ret;
	}

	if ((ret = connect(vio->fd, res->ai_addr, res->ai_addrlen)) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
			MOWGLI_VIO_RETURN_ERRCODE(vio, strerror, errno);
	}

	SSL_load_error_strings();
	SSL_library_init();

	/* Good default; most stuff isn't using TLSv1 or above yet (unfortunately)
	 * The library will negotiate something better if supported.
	 */
	connection->ssl_context = SSL_CTX_new(SSLv3_client_method());
	if (connection->ssl_context == NULL)
		MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, ERR_get_error())

	connection->ssl_handle = SSL_new(connection->ssl_context);
	if (connection->ssl_handle == NULL)
		MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, ERR_get_error())

	if (!SSL_set_fd(connection->ssl_handle, vio->fd))
		MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, ERR_get_error())

	if (SSL_connect(connection->ssl_handle) != 1)
		MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, ERR_get_error())

	vio->privdata = connection;

	return 0;
}

static int mowgli_vio_openssl_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;
	mowgli_ssl_connection_t *connection = vio->privdata;

	vio->error.op = MOWGLI_VIO_ERR_OP_READ;

	if ((ret = (int)SSL_read(connection->ssl_handle, buffer, len)) < 0)
	{
		unsigned long err = ERR_get_error();

		if (err != SSL_ERROR_WANT_READ)
			MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, err)

		if (ret == 0)
		{
			vio->error.type = MOWGLI_VIO_ERR_REMOTE_HANGUP;
			mowgli_strlcpy(vio->error.string, "Remote host closed the socket", sizeof(vio->error.string));
			return mowgli_vio_error(vio);
		}
	}

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

static int mowgli_vio_openssl_write(mowgli_vio_t *vio, void *buffer, size_t len)
{
	int ret;
	mowgli_ssl_connection_t *connection = vio->privdata;

	vio->error.op = MOWGLI_VIO_ERR_OP_WRITE;

	if ((ret = (int)SSL_write(connection->ssl_handle, buffer, len)) == -1)
	{
		unsigned long err = ERR_get_error();
		
		if (err != SSL_ERROR_WANT_WRITE)
			MOWGLI_VIO_RETURN_SSLERR_ERRCODE(vio, err)
	}

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

static int mowgli_vio_openssl_close(mowgli_vio_t *vio)
{
	mowgli_ssl_connection_t *connection = vio->privdata;

	SSL_shutdown(connection->ssl_handle);
	SSL_free(connection->ssl_handle);
	SSL_CTX_free(connection->ssl_context);

	mowgli_free(connection);

	close(vio->fd);
	return 0;
}

#else

int mowgli_vio_openssl_setssl(mowgli_vio_t *vio, int flags)
{
	mowgli_log("OpenSSL requested on a VIO object, but mowgli was built without OpenSSL support...");
	return -1;
}

#endif

