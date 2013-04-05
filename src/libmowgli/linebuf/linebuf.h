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
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MOWGLI_LINEBUF_LINEBUF_H__
#define __MOWGLI_LINEBUF_LINEBUF_H__

typedef struct _mowgli_linebuf_buf mowgli_linebuf_buf_t;

typedef void mowgli_linebuf_readline_cb_t (mowgli_linebuf_t *, char *, size_t, void *);
typedef void mowgli_linebuf_shutdown_cb_t (mowgli_linebuf_t *, void *);

extern mowgli_linebuf_t *mowgli_linebuf_create(mowgli_linebuf_readline_cb_t *cb, void *userdata);

/* XXX these are unfortunately named and will change */
extern void mowgli_linebuf_attach_to_eventloop(mowgli_linebuf_t *linebuf, mowgli_eventloop_t *eventloop);
extern void mowgli_linebuf_detach_from_eventloop(mowgli_linebuf_t *linebuf);
extern void mowgli_linebuf_destroy(mowgli_linebuf_t *linebuf);

extern void mowgli_linebuf_setbuflen(mowgli_linebuf_buf_t *buffer, size_t buflen);
extern void mowgli_linebuf_delim(mowgli_linebuf_t *linebuf, const char *delim, const char *endl);
extern void mowgli_linebuf_write(mowgli_linebuf_t *linebuf, const char *data, int len);
extern void mowgli_linebuf_writef(mowgli_linebuf_t *linebuf, const char *format, ...);
extern void mowgli_linebuf_shut_down(mowgli_linebuf_t *linebuf);

struct _mowgli_linebuf_buf
{
	char *buffer;
	size_t buflen;
	size_t maxbuflen;
};

/* Errors */
#define MOWGLI_LINEBUF_ERR_NONE 0x0000
#define MOWGLI_LINEBUF_ERR_READBUF_FULL 0x0001
#define MOWGLI_LINEBUF_ERR_WRITEBUF_FULL 0x0002

/* Informative */
#define MOWGLI_LINEBUF_LINE_HASNULLCHAR 0x0004

/* State */
#define MOWGLI_LINEBUF_SHUTTING_DOWN 0x0100

struct _mowgli_linebuf
{
	mowgli_linebuf_readline_cb_t *readline_cb;
	mowgli_linebuf_shutdown_cb_t *shutdown_cb;

	mowgli_vio_t *vio;

	const char *delim;
	const char *endl;
	size_t endl_len;

	int flags;

	mowgli_linebuf_buf_t readbuf;
	mowgli_linebuf_buf_t writebuf;

	mowgli_eventloop_t *eventloop;

	bool return_normal_strings;

	void *userdata;
};

static inline mowgli_vio_t *
mowgli_linebuf_get_vio(mowgli_linebuf_t *linebuf)
{
	return_val_if_fail(linebuf != NULL, NULL);
	return linebuf->vio;
}

#endif
