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

/* Note these routines are just defaults for clients -- if you have more
 * specific needs, you should write your own implementation (that is the
 * whole point of vio)
 */

#ifdef HAVE_OPENSSL

typedef struct
{
	SSL *ssl_handle;
	SSL_CTX *ssl_context;
	mowgli_vio_ssl_settings_t settings;
} mowgli_ssl_connection_t;

static int mowgli_vio_openssl_client_handshake(mowgli_vio_t *vio, mowgli_ssl_connection_t *connection);
static int mowgli_openssl_read_or_write(bool read, mowgli_vio_t *vio, void *readbuf, const void *writebuf, size_t len);

static mowgli_heap_t *ssl_heap = NULL;

static bool openssl_init = false;

static mowgli_vio_ops_t *openssl_ops = NULL;

int
mowgli_vio_openssl_setssl(mowgli_vio_t *vio, mowgli_vio_ssl_settings_t *settings, mowgli_vio_ops_t *ops)
{
	mowgli_ssl_connection_t *connection;

	return_val_if_fail(vio, -255);

	if (!ssl_heap)
		ssl_heap = mowgli_heap_create(sizeof(mowgli_ssl_connection_t), 64, BH_NOW);

	connection = mowgli_heap_alloc(ssl_heap);
	vio->privdata = connection;

	if (settings)
		memcpy(&connection->settings, settings, sizeof(mowgli_vio_ssl_settings_t));
	else
		/* Greatest compat without being terribly insecure */
		connection->settings.ssl_version = MOWGLI_VIO_SSLFLAGS_SSLV3;

	if (ops == NULL)
	{
		if (!openssl_ops)
		{
			openssl_ops = mowgli_alloc(sizeof(mowgli_vio_ops_t));
			memcpy(openssl_ops, &mowgli_vio_default_ops, sizeof(mowgli_vio_ops_t));
		}

		vio->ops = openssl_ops;
	}
	else
	{
		vio->ops = ops;
	}

	/* Change ops */
	mowgli_vio_ops_set_op(vio->ops, connect, mowgli_vio_openssl_default_connect);
	mowgli_vio_ops_set_op(vio->ops, read, mowgli_vio_openssl_default_read);
	mowgli_vio_ops_set_op(vio->ops, write, mowgli_vio_openssl_default_write);
	mowgli_vio_ops_set_op(vio->ops, close, mowgli_vio_openssl_default_close);
	mowgli_vio_ops_set_op(vio->ops, accept, mowgli_vio_openssl_default_accept);
	mowgli_vio_ops_set_op(vio->ops, listen, mowgli_vio_openssl_default_listen);

	/* SSL setup */
	if (!openssl_init)
	{
		openssl_init = true;
		SSL_library_init();
		SSL_load_error_strings();
		ERR_load_BIO_strings();
		OpenSSL_add_all_algorithms();
	}

	return 0;
}

/* Returns void so they can be stubs */
void *
mowgli_vio_openssl_getsslhandle(mowgli_vio_t *vio)
{
	return_val_if_fail(vio, NULL);
	mowgli_ssl_connection_t *connection = vio->privdata;
	return connection->ssl_handle;
}

void *
mowgli_vio_openssl_getsslcontext(mowgli_vio_t *vio)
{
	return_val_if_fail(vio, NULL);
	mowgli_ssl_connection_t *connection = vio->privdata;
	return connection->ssl_context;
}

int
mowgli_vio_openssl_default_connect(mowgli_vio_t *vio, mowgli_vio_sockaddr_t *addr)
{
	const int fd = mowgli_vio_getfd(vio);

	return_val_if_fail(fd != -1, -255);

	mowgli_ssl_connection_t *connection = vio->privdata;

	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	if (connect(fd, (struct sockaddr *) &addr->addr, addr->addrlen) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
		{
			return mowgli_vio_err_errcode(vio, strerror, errno);
		}
		else
		{
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, true);
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, true);
			vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
			return 0;
		}
	}

	memcpy(&vio->addr.addr, &addr->addr, addr->addrlen);
	vio->addr.addrlen = addr->addrlen;

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCLIENT, true);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSERVER, false);

	/* Non-blocking socket, begin handshake */
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);
	return mowgli_vio_openssl_client_handshake(vio, connection);
}

int
mowgli_vio_openssl_default_listen(mowgli_vio_t *vio, int backlog)
{
	return_val_if_fail(vio, -255);

	mowgli_ssl_connection_t *connection = vio->privdata;
	const SSL_METHOD *method;
	const int fd = mowgli_vio_getfd(vio);

	vio->error.op = MOWGLI_VIO_ERR_OP_LISTEN;

	switch (connection->settings.ssl_version)
	{
	case MOWGLI_VIO_SSLFLAGS_SSLV2:
		method = SSLv23_server_method();
		break;
	case MOWGLI_VIO_SSLFLAGS_SSLV3:
		method = SSLv3_server_method();
		break;
	case MOWGLI_VIO_SSLFLAGS_TLSV10:
	case MOWGLI_VIO_SSLFLAGS_TLSV11:
	case MOWGLI_VIO_SSLFLAGS_TLSV12:
		method = TLSv1_server_method();
		break;
	default:

		/* Compat method */
		method = SSLv23_server_method();
	}

	connection->ssl_context = SSL_CTX_new((SSL_METHOD *) method);

	if (connection->ssl_context == NULL)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	connection->ssl_handle = SSL_new(connection->ssl_context);

	if (connection->ssl_handle == NULL)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	SSL_set_accept_state(connection->ssl_handle);
	SSL_CTX_set_options(connection->ssl_context, SSL_OP_SINGLE_DH_USE);

	if (connection->settings.password_func)
	{
		SSL_CTX_set_default_passwd_cb(connection->ssl_context, connection->settings.password_func);
		SSL_CTX_set_default_passwd_cb_userdata(connection->ssl_context, vio->userdata);
	}

	if (SSL_CTX_use_certificate_file(connection->ssl_context, connection->settings.cert_path, SSL_FILETYPE_PEM) != 1)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	if (SSL_CTX_use_PrivateKey_file(connection->ssl_context, connection->settings.privatekey_path, SSL_FILETYPE_PEM) != 1)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	if (listen(fd, backlog) != 0)
		return mowgli_vio_err_errcode(vio, strerror, errno);

	if (!SSL_set_fd(connection->ssl_handle, fd))
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSERVER, true);
	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;

	return 0;
}

int
mowgli_vio_openssl_default_accept(mowgli_vio_t *vio, mowgli_vio_t *newvio)
{
	const int fd = mowgli_vio_getfd(vio);
	int afd;
	int ret;

	return_val_if_fail(fd != -1, -255);

	mowgli_ssl_connection_t *connection = vio->privdata;
	mowgli_ssl_connection_t *newconnection;

	vio->error.op = MOWGLI_VIO_ERR_OP_ACCEPT;

	if (!newvio)
	{
		const char errstr[] = "accept not called with valid new VIO object";
		vio->error.type = MOWGLI_VIO_ERR_API;
		mowgli_strlcpy(vio->error.string, errstr, sizeof(errstr));
		return mowgli_vio_error(vio);
	}

	if ((afd = accept(fd, (struct sockaddr *) &newvio->addr.addr, &(newvio->addr.addrlen))) < 0)
	{
		if (!mowgli_eventloop_ignore_errno(errno))
			return mowgli_vio_err_errcode(vio, strerror, errno);
		else
			return 0;
	}

	newvio->io.fd = afd;

	mowgli_vio_openssl_setssl(newvio, &connection->settings, vio->ops);
	newconnection = newvio->privdata;
	newconnection->ssl_context = connection->ssl_context;
	newconnection->ssl_handle = SSL_new(newconnection->ssl_context);

	if (!SSL_set_fd(newconnection->ssl_handle, afd))
		return mowgli_vio_err_sslerrcode(newvio, ERR_get_error());

	if ((ret = SSL_accept(newconnection->ssl_handle)) != 1)
	{
		unsigned long err;

		switch (SSL_get_error(newconnection->ssl_handle, ret))
		{
		case SSL_ERROR_WANT_READ:
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDREAD, true);
			MOWGLI_VIO_SETREAD(vio)
			return 0;
		case SSL_ERROR_WANT_WRITE:
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDWRITE, true);
			MOWGLI_VIO_SETWRITE(vio)
			return 0;
		case SSL_ERROR_ZERO_RETURN:
			return 0;
		case SSL_ERROR_SYSCALL:
			return mowgli_vio_err_errcode(newvio, strerror, errno);
		default:
			err = ERR_get_error();
			break;
		}

		if (err > 0)
		{
			errno = EIO;
			return mowgli_vio_err_errcode(vio, strerror, errno);
		}

		return -1;
	}

	mowgli_vio_setflag(newvio, MOWGLI_VIO_FLAGS_ISCLIENT, true);
	mowgli_vio_setflag(newvio, MOWGLI_VIO_FLAGS_ISSERVER, false);

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

static int
mowgli_vio_openssl_client_handshake(mowgli_vio_t *vio, mowgli_ssl_connection_t *connection)
{
	const int fd = mowgli_vio_getfd(vio);
	int ret;
	const SSL_METHOD *method;

	vio->error.op = MOWGLI_VIO_ERR_OP_CONNECT;

	switch (connection->settings.ssl_version)
	{
	case MOWGLI_VIO_SSLFLAGS_SSLV2:
		method = SSLv23_client_method();
		break;
	case MOWGLI_VIO_SSLFLAGS_SSLV3:
		method = SSLv3_client_method();
		break;
	case MOWGLI_VIO_SSLFLAGS_TLSV10:
	case MOWGLI_VIO_SSLFLAGS_TLSV11:
	case MOWGLI_VIO_SSLFLAGS_TLSV12:
		method = TLSv1_client_method();
		break;
	default:

		/* Compat method */
		method = SSLv23_client_method();
	}

	/* Cast is to eliminate an excessively bogus warning on old OpenSSL --Elizacat */
	connection->ssl_context = SSL_CTX_new((SSL_METHOD *) method);

	if (connection->ssl_context == NULL)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	connection->ssl_handle = SSL_new(connection->ssl_context);

	if (connection->ssl_handle == NULL)
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	SSL_set_connect_state(connection->ssl_handle);

	if (!SSL_set_fd(connection->ssl_handle, fd))
		return mowgli_vio_err_sslerrcode(vio, ERR_get_error());

	if (vio->eventloop)
		SSL_CTX_set_mode(connection->ssl_context, SSL_MODE_ENABLE_PARTIAL_WRITE);

	if ((ret = SSL_connect(connection->ssl_handle)) != 1)
	{
		unsigned long err = SSL_get_error(connection->ssl_handle, ret);

		if (err == SSL_ERROR_WANT_READ)
		{
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDREAD, true);
			MOWGLI_VIO_SETREAD(vio)
		}
		else if (err == SSL_ERROR_WANT_WRITE)
		{
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDWRITE, true);
			MOWGLI_VIO_SETWRITE(vio)
		}
		else if (err == SSL_ERROR_WANT_CONNECT)
		{
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, true);
			return 0;
		}
		else
		{
			return mowgli_vio_err_sslerrcode(vio, err);
		}

		mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, false);
		return 0;
	}

	/* Connected */
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISSSLCONNECTING, false);

	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return 0;
}

# define MOWGLI_VIO_SSL_DOREAD true
# define MOWGLI_VIO_SSL_DOWRITE false

int
mowgli_vio_openssl_default_read(mowgli_vio_t *vio, void *buffer, size_t len)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_READ;
	return mowgli_openssl_read_or_write(MOWGLI_VIO_SSL_DOREAD, vio, buffer, NULL, len);
}

int
mowgli_vio_openssl_default_write(mowgli_vio_t *vio, const void *buffer, size_t len)
{
	vio->error.op = MOWGLI_VIO_ERR_OP_WRITE;
	return mowgli_openssl_read_or_write(MOWGLI_VIO_SSL_DOWRITE, vio, NULL, buffer, len);
}

static int
mowgli_openssl_read_or_write(bool read, mowgli_vio_t *vio, void *readbuf, const void *writebuf, size_t len)
{
	mowgli_ssl_connection_t *connection = vio->privdata;
	int ret;
	unsigned long err;

	/* We are connected */
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_ISCONNECTING, false);

	if (mowgli_vio_hasflag(vio, MOWGLI_VIO_FLAGS_ISSSLCONNECTING))
		return mowgli_vio_openssl_client_handshake(vio, connection);

	return_val_if_fail(connection->ssl_handle != NULL, -1);

	if (read)
	{
		ret = (int) SSL_read(connection->ssl_handle, readbuf, len);
	}
	else
	{
		ret = (int) SSL_write(connection->ssl_handle, writebuf, len);
		MOWGLI_VIO_UNSETWRITE(vio)
	}

	if (ret < 0)
	{
		switch (SSL_get_error(connection->ssl_handle, ret))
		{
		case SSL_ERROR_WANT_READ:
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDREAD, true);
			MOWGLI_VIO_SETREAD(vio)
			return 0;
		case SSL_ERROR_WANT_WRITE:
			mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDWRITE, true);
			MOWGLI_VIO_SETWRITE(vio)
			return 0;
		case SSL_ERROR_ZERO_RETURN:
			return 0;
		case SSL_ERROR_SYSCALL:

			if ((err = ERR_get_error()) == 0)
			{
				vio->error.type = MOWGLI_VIO_ERR_REMOTE_HANGUP;
				mowgli_strlcpy(vio->error.string, "Remote host closed the socket", sizeof(vio->error.string));

				MOWGLI_VIO_SET_CLOSED(vio);

				return mowgli_vio_error(vio);
			}

			break;

		default:
			err = ERR_get_error();
			break;
		}

		if (err > 0)
		{
			errno = EIO;
			return mowgli_vio_err_errcode(vio, strerror, errno);
		}

		/* idk lol */
		return -1;
	}

	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDREAD, false);
	mowgli_vio_setflag(vio, MOWGLI_VIO_FLAGS_NEEDWRITE, false);
	vio->error.op = MOWGLI_VIO_ERR_OP_NONE;
	return ret;
}

int
mowgli_vio_openssl_default_close(mowgli_vio_t *vio)
{
	const int fd = mowgli_vio_getfd(vio);
	mowgli_ssl_connection_t *connection = vio->privdata;

	return_val_if_fail(connection->ssl_handle != NULL, -1);

	SSL_shutdown(connection->ssl_handle);
	SSL_free(connection->ssl_handle);
	SSL_CTX_free(connection->ssl_context);

	mowgli_heap_free(ssl_heap, connection);

	MOWGLI_VIO_SET_CLOSED(vio);

	/* FIXME - doesn't verify a proper SSL shutdown! */
	close(fd);
	return 0;
}

#else

int
mowgli_vio_openssl_setssl(mowgli_vio_t *vio, mowgli_vio_ssl_settings_t *settings, mowgli_vio_ops_t *ops)
{
	mowgli_log("OpenSSL requested on a VIO object, but mowgli was built without OpenSSL support...");
	return -1;
}

void *
mowgli_vio_openssl_getsslhandle(mowgli_vio_t *vio)
{
	mowgli_log("Cannot get VIO SSL handle as libmowgli was built without OpenSSL support");
	return NULL;
}

void *
mowgli_vio_openssl_getsslcontext(mowgli_vio_t *vio)
{
	mowgli_log("Cannot get VIO SSL context as libmowgli was built without OpenSSL support");
	return NULL;
}

#endif
