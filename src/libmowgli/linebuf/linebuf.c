/*
 * libmowgli: A collection of useful routines for programming.
 * linebuf.c: Line buffering for the event loop system
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

static mowgli_heap_t *linebuf_heap = NULL;

static void mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_process(mowgli_linebuf_t *linebuf);
static void mowgli_linebuf_do_shutdown(mowgli_linebuf_t *linebuf);

static int mowgli_linebuf_error(mowgli_vio_t *vio);

#ifdef NOTYET
static mowgli_vio_evops_t linebuf_evops =
{
	.read_cb = mowgli_linebuf_read_data,
	.write_cb = mowgli_linebuf_write_data
};
#endif

mowgli_linebuf_t *
mowgli_linebuf_create(mowgli_linebuf_readline_cb_t *cb, void *userdata)
{
	mowgli_linebuf_t *linebuf;

	if (linebuf_heap == NULL)
		linebuf_heap = mowgli_heap_create(sizeof(mowgli_linebuf_t), 16, BH_NOW);

	linebuf = mowgli_heap_alloc(linebuf_heap);

	/* Sane default */
	mowgli_linebuf_delim(linebuf, "\r\n", "\r\n");
	linebuf->readline_cb = cb;

	linebuf->flags = 0;

	linebuf->readbuf.buffer = NULL;
	linebuf->writebuf.buffer = NULL;
	mowgli_linebuf_setbuflen(&(linebuf->readbuf), 65536);
	mowgli_linebuf_setbuflen(&(linebuf->writebuf), 65536);

	linebuf->eventloop = NULL;

	linebuf->return_normal_strings = true;	/* This is generally what you want, but beware of malicious \0's in input data! */

	linebuf->userdata = userdata;

	linebuf->vio = mowgli_vio_create(linebuf);

	return linebuf;
}

/* Attach the linebuf instance to the eventloop -- socket must be created first with VIO instance! */
void
mowgli_linebuf_attach_to_eventloop(mowgli_linebuf_t *linebuf, mowgli_eventloop_t *eventloop)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(linebuf != NULL);
	return_if_fail(linebuf->vio != NULL);
	return_if_fail((linebuf->vio->flags & MOWGLI_VIO_FLAGS_ISCLOSED) == 0);

#ifdef NOTYET
	mowgli_vio_eventloop_attach(linebuf->vio, eventloop, &linebuf_evops);
#else
	mowgli_vio_eventloop_attach(linebuf->vio, eventloop, NULL);
#endif
	mowgli_pollable_setselect(eventloop, linebuf->vio->io.e, MOWGLI_EVENTLOOP_IO_READ, mowgli_linebuf_read_data);
	mowgli_pollable_setselect(eventloop, linebuf->vio->io.e, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);

	linebuf->eventloop = eventloop;
}

/* Detach the linebuf instance from the eventloop */
void
mowgli_linebuf_detach_from_eventloop(mowgli_linebuf_t *linebuf)
{
	return_if_fail(linebuf != NULL);
	return_if_fail(linebuf->eventloop != NULL);
	mowgli_pollable_setselect(linebuf->eventloop, linebuf->vio->io.e, MOWGLI_EVENTLOOP_IO_READ, NULL);
	mowgli_pollable_setselect(linebuf->eventloop, linebuf->vio->io.e, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
	mowgli_vio_eventloop_detach(linebuf->vio);
	linebuf->eventloop = NULL;
}

void
mowgli_linebuf_destroy(mowgli_linebuf_t *linebuf)
{
	if (linebuf->eventloop != NULL)
		mowgli_linebuf_detach_from_eventloop(linebuf);

	mowgli_vio_destroy(linebuf->vio);

	mowgli_free(linebuf->readbuf.buffer);
	mowgli_free(linebuf->writebuf.buffer);
	mowgli_heap_free(linebuf_heap, linebuf);
}

void
mowgli_linebuf_setbuflen(mowgli_linebuf_buf_t *buffer, size_t buflen)
{
	return_if_fail(buffer != NULL);

	if (buffer->buffer == NULL)
	{
		buffer->buffer = mowgli_alloc(buflen);
	}
	else
	{
		char tmpbuf[buffer->maxbuflen];

		if (buffer->buflen > 0)
			memcpy(tmpbuf, buffer->buffer, buffer->maxbuflen);	/* Copy into tmp buffer */

		/* Free old buffer and reallocate */
		mowgli_free(buffer->buffer);
		buffer->buffer = mowgli_alloc(buflen);

		if (buffer->buflen > 0)
			/* Copy into new buffer using old buffer size */
			memcpy(buffer->buffer, tmpbuf, buffer->maxbuflen);
	}

	buffer->maxbuflen = buflen;
}

void
mowgli_linebuf_delim(mowgli_linebuf_t *linebuf, const char *delim, const char *endl)
{
	return_if_fail(linebuf != NULL);
	return_if_fail(delim != NULL && *delim != '\0');
	return_if_fail(endl != NULL && *endl != '\0');

	linebuf->delim = delim;
	linebuf->endl = endl;
	linebuf->endl_len = strlen(endl);
}

static void
mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *) userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	void *bufpos;
	size_t offset;
	int ret;

	if (buffer->maxbuflen - buffer->buflen == 0)
	{
		linebuf->flags |= MOWGLI_LINEBUF_ERR_READBUF_FULL;
		mowgli_linebuf_error(linebuf->vio);
		return;
	}

	bufpos = buffer->buffer + buffer->buflen;
	offset = buffer->maxbuflen - buffer->buflen + 1;

	if ((ret = mowgli_vio_read(linebuf->vio, bufpos, offset)) <= 0)
	{
		if (linebuf->vio->error.type == MOWGLI_VIO_ERR_NONE)
			return;

		/* Let's never come back here */
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_READ, NULL);
		mowgli_linebuf_do_shutdown(linebuf);
		return;
	}

	/* Le sigh -- stupid edge-triggered interfaces */
	if (mowgli_vio_hasflag(linebuf->vio, MOWGLI_VIO_FLAGS_NEEDREAD))
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_READ, mowgli_linebuf_read_data);

	/* Do we want a write for SSL? */
	if (mowgli_vio_hasflag(linebuf->vio, MOWGLI_VIO_FLAGS_NEEDWRITE))
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);

	buffer->buflen += ret;
	mowgli_linebuf_process(linebuf);
}

static void
mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *) userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->writebuf);
	int ret;

	if ((ret = mowgli_vio_write(linebuf->vio, buffer->buffer, buffer->buflen)) <= 0)
		if (linebuf->vio->error.code != MOWGLI_VIO_ERR_NONE)
			/* If we have a genuine error, we shouldn't come back to this func
			 * Otherwise we'll try again. */
			if (ret != 0)
			{
				mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
				mowgli_log("mowgli_vio_write returned error [%ld]: %s", linebuf->vio->error.code, linebuf->vio->error.string);
				return;
			}

	buffer->buflen -= ret;

	/* Anything else to write? */
	if (buffer->buflen == 0)
	{
		if (!mowgli_vio_hasflag(linebuf->vio, MOWGLI_VIO_FLAGS_NEEDWRITE))
			mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);

		if ((linebuf->flags & MOWGLI_LINEBUF_SHUTTING_DOWN) != 0)
			mowgli_linebuf_do_shutdown(linebuf);
	}
	else
	{
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);
	}
}

void
mowgli_linebuf_writef(mowgli_linebuf_t *linebuf, const char *format, ...)
{
	char buf[linebuf->writebuf.maxbuflen];
	size_t len;
	va_list va;

	va_start(va, format);
	len = vsnprintf(buf, linebuf->writebuf.maxbuflen - 1, format, va);
	va_end(va);

	mowgli_linebuf_write(linebuf, buf, len);
}

void
mowgli_linebuf_write(mowgli_linebuf_t *linebuf, const char *data, int len)
{
	char *ptr = linebuf->writebuf.buffer + linebuf->writebuf.buflen;

	return_if_fail(len > 0);
	return_if_fail(data != NULL);

	if (linebuf->flags & MOWGLI_LINEBUF_SHUTTING_DOWN)
		return;

	if (linebuf->writebuf.buflen + len + linebuf->endl_len > linebuf->writebuf.maxbuflen)
	{
		linebuf->flags |= MOWGLI_LINEBUF_ERR_WRITEBUF_FULL;
		mowgli_linebuf_error(linebuf->vio);
		return;
	}

	memcpy((void *) ptr, data, len);
	memcpy((void *) (ptr + len), linebuf->endl, linebuf->endl_len);

	linebuf->writebuf.buflen += len + linebuf->endl_len;

	/* Schedule our write */
	mowgli_pollable_setselect(linebuf->eventloop, linebuf->vio->io.e, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);
}

void
mowgli_linebuf_shut_down(mowgli_linebuf_t *linebuf)
{
	return_if_fail(linebuf != NULL);

	linebuf->flags |= MOWGLI_LINEBUF_SHUTTING_DOWN;

	if (linebuf->writebuf.buflen == 0)
		mowgli_linebuf_do_shutdown(linebuf);
}

static void
mowgli_linebuf_process(mowgli_linebuf_t *linebuf)
{
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);

	char *line_start;
	char *cptr;
	size_t len = 0;
	int linecount = 0;

	line_start = cptr = buffer->buffer;

	/* Initalise */
	linebuf->flags &= ~MOWGLI_LINEBUF_LINE_HASNULLCHAR;

	while (len < buffer->buflen)
	{
		if (!strchr(linebuf->delim, *cptr))
		{
			if (*cptr == '\0')
				/* Warn about unexpected null chars in the string */
				linebuf->flags |= MOWGLI_LINEBUF_LINE_HASNULLCHAR;

			cptr++;
			len++;
			continue;
		}

		linecount++;

		/* We now have a line */
		if (linebuf->return_normal_strings)
			*cptr = '\0';

		if ((linebuf->flags & MOWGLI_LINEBUF_SHUTTING_DOWN) == 0)
			linebuf->readline_cb(linebuf, line_start, cptr - line_start, linebuf->userdata);

		/* Next line starts here; begin scanning and set the start of it */
		while (strchr(linebuf->delim, *cptr))
		{
			len++;
			cptr++;
		}

		line_start = cptr;

		/* Reset this for next line */
		linebuf->flags &= ~MOWGLI_LINEBUF_LINE_HASNULLCHAR;
	}

	if ((linecount == 0) && (buffer->buflen == buffer->maxbuflen))
	{
		/* No more chars will fit in the buffer and we don't have a line
		 * We're really screwed, let's trigger an error. */
		linebuf->flags |= MOWGLI_LINEBUF_ERR_READBUF_FULL;
		mowgli_linebuf_error(linebuf->vio);
		return;
	}

	if (line_start != cptr)
	{
		buffer->buflen = cptr - line_start;
		memmove(buffer->buffer, line_start, cptr - line_start);
	}
	else
	{
		buffer->buflen = 0;
	}
}

static void
mowgli_linebuf_do_shutdown(mowgli_linebuf_t *linebuf)
{
	if (linebuf && linebuf->shutdown_cb)
		linebuf->shutdown_cb(linebuf, linebuf->userdata);
}

static int
mowgli_linebuf_error(mowgli_vio_t *vio)
{
	mowgli_linebuf_t *linebuf = vio->userdata;
	mowgli_vio_error_t *error = &(linebuf->vio->error);

	if (linebuf->flags & MOWGLI_LINEBUF_ERR_READBUF_FULL)
	{
		error->op = MOWGLI_VIO_ERR_OP_READ;
		error->type = MOWGLI_VIO_ERR_CUSTOM;
		mowgli_strlcpy(error->string, "Read buffer full", sizeof(error->string));
	}
	else if (linebuf->flags & MOWGLI_LINEBUF_ERR_WRITEBUF_FULL)
	{
		error->op = MOWGLI_VIO_ERR_OP_WRITE;
		error->type = MOWGLI_VIO_ERR_CUSTOM;
		mowgli_strlcpy(error->string, "Write buffer full", sizeof(error->string));
	}

	/* Pass this up to higher callback */
	return mowgli_vio_error(vio);
}
