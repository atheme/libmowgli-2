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

static int mowgli_linebuf_default_read_cb(mowgli_linebuf_t *linebuf, mowgli_eventloop_io_dir_t dir);
static int mowgli_linebuf_default_write_cb(mowgli_linebuf_t *linebuf, mowgli_eventloop_io_dir_t dir);
static void mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_process(mowgli_linebuf_t *linebuf);

mowgli_linebuf_t *
mowgli_linebuf_create(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_readline_cb_t *cb)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_linebuf_t *linebuf;

	if (linebuf_heap == NULL)
		linebuf_heap = mowgli_heap_create(sizeof(mowgli_linebuf_t), 16, BH_NOW);
	
	linebuf = mowgli_heap_alloc(linebuf_heap);

	linebuf->eventloop = eventloop;
	linebuf->io = io;

	linebuf->userdata = pollable->userdata;
	pollable->userdata = (void *)linebuf;

	linebuf->delim = "\r\n"; /* Sane default */
	linebuf->read_cb = mowgli_linebuf_default_read_cb; /* Also sane default, change if you use something weird e.g., SSL */
	linebuf->write_cb = mowgli_linebuf_default_write_cb;
	linebuf->readline_cb = cb;

	linebuf->remote_hangup = false;
	linebuf->read_buffer_full = false;
	linebuf->write_buffer_full = false;
	linebuf->err = 0;

	linebuf->readbuf.buffer = NULL;
	linebuf->writebuf.buffer = NULL;
	mowgli_linebuf_setbuflen(&(linebuf->readbuf), 65536);
	mowgli_linebuf_setbuflen(&(linebuf->writebuf), 65536);

	linebuf->return_normal_strings = true; /* This is generally what you want, but beware of malicious \0's in input data! */

	mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_READ, mowgli_linebuf_read_data);

	return linebuf;
}

void mowgli_linebuf_destroy(mowgli_linebuf_t *linebuf)
{
	mowgli_free(linebuf->readbuf.buffer);
	mowgli_free(linebuf->writebuf.buffer);
	mowgli_heap_free(linebuf_heap, linebuf);
}

void mowgli_linebuf_setbuflen(mowgli_linebuf_buf_t *buffer, size_t buflen)
{
	return_if_fail(buffer != NULL);

	if (buffer->buffer == NULL)
		buffer->buffer = mowgli_alloc(buflen);
	else
	{
		char tmpbuf[buffer->maxbuflen];

		if (buffer->buflen > 0)
			memcpy(tmpbuf, buffer->buffer, buffer->maxbuflen); /* Copy into tmp buffer */

		/* Free old buffer and reallocate */
		mowgli_free(buffer->buffer);
		buffer->buffer = mowgli_alloc(buflen);

		if (buffer->buflen > 0)
			/* Copy into new buffer using old buffer size */
			memcpy(buffer->buffer, tmpbuf, buffer->maxbuflen);
	}

	buffer->maxbuflen = buflen;
}

/* This will be going away after VIO is integrated */
static int mowgli_linebuf_default_read_cb(mowgli_linebuf_t *linebuf, mowgli_eventloop_io_dir_t dir)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(linebuf->io);
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	int ret;

	if (buffer->maxbuflen - buffer->buflen == 0)
	{
		linebuf->read_buffer_full = true;
		return 0; /* Ugh, buffer full :( */
	}

	if ((ret = recv(pollable->fd, buffer->buffer + buffer->buflen, buffer->maxbuflen - buffer->buflen + 1, 0)) == 0)
	{
		linebuf->remote_hangup = true;
		return 0; /* Connection reset by peer */
	}
	else if(ret < 0)
	{
		if (errno != EAGAIN)  /* Try again (can happen on Linux) */
			linebuf->err = errno;

		return 0;
	}

	return ret;
}

/* This will be going away after VIO is integrated */
static int mowgli_linebuf_default_write_cb(mowgli_linebuf_t *linebuf, mowgli_eventloop_io_dir_t dir)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(linebuf->io);
	mowgli_linebuf_buf_t *buffer = &(linebuf->writebuf);
	int ret;

	if (buffer->buflen == 0)
		return 0; /* Nothing to do */

	if ((ret = send(pollable->fd, buffer->buffer, buffer->buflen, 0)) == -1)
	{
		if (errno != EAGAIN)
		{
			linebuf->err = errno;
			linebuf->error_cb(linebuf, MOWGLI_EVENTLOOP_IO_WRITE);
		}

		return 0;
	}
	else if (ret < buffer->buflen)
		memmove(buffer->buffer, buffer->buffer + ret, buffer->buflen - ret);

	return ret;
}

static void mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *)userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	int ret;

	if ((ret = linebuf->read_cb(linebuf, MOWGLI_EVENTLOOP_IO_READ)) == 0)
	{
		linebuf->error_cb(linebuf, MOWGLI_EVENTLOOP_IO_READ);
		return;
	}

	buffer->buflen += ret;
	mowgli_linebuf_process(linebuf);
}

static void mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *)userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->writebuf);
	int ret;

	if (buffer->buflen == 0)
	{
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
		return;
	}

	if ((ret = linebuf->write_cb(linebuf, MOWGLI_EVENTLOOP_IO_WRITE)) == 0)
	{
		if (linebuf->err != 0)
			mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
		return;
	}

	buffer->buflen -= ret;

	if (buffer->buflen == 0)
		mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, NULL);
}

void mowgli_linebuf_write(mowgli_linebuf_t *linebuf, const char *data, int len)
{
	char *ptr = linebuf->writebuf.buffer + linebuf->writebuf.buflen;
	int delim_len = strlen(linebuf->delim);

	return_if_fail(len > 0);
	return_if_fail(data != NULL);

	if (linebuf->writebuf.buflen + len + delim_len > linebuf->writebuf.maxbuflen)
	{
		linebuf->write_buffer_full = true;
		linebuf->error_cb(linebuf, MOWGLI_EVENTLOOP_IO_WRITE);
		return;
	}

	memcpy((void *)ptr, data, len);
	memcpy((void *)(ptr + len), linebuf->delim, delim_len);

	linebuf->writebuf.buflen += len + delim_len;

	mowgli_pollable_setselect(eventloop, io, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);
}

static void mowgli_linebuf_process(mowgli_linebuf_t *linebuf)
{
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	size_t delim_len = strlen(linebuf->delim);

	char *line_start;
	char *cptr;
	int len = 0;
	int linecount = 0;

	line_start = cptr = buffer->buffer;

	/* Initalise */
	linebuf->line_has_nullchar = false;

	while (len < buffer->buflen)
	{
		if (memcmp((void *)cptr, linebuf->delim, delim_len) != 0)
		{
			if (*cptr == '\0')
				/* Warn about unexpected null chars in the string */
				linebuf->line_has_nullchar = true;
			cptr++;
			len++;
			continue;
		}

		linecount++;

		/* We now have a line */
		if (linebuf->return_normal_strings)
			*cptr = '\0';

		linebuf->readline_cb(linebuf, line_start, cptr - line_start, linebuf->userdata);

		/* Next line starts here; begin scanning and set the start of it */
		len += delim_len;
		cptr += delim_len;
		line_start = cptr;

		/* Reset this for next line */
		linebuf->line_has_nullchar = false;
	}

	if (linecount == 0 && (buffer->buflen == buffer->maxbuflen))
	{
		/* No more chars will fit in the buffer and we don't have a line 
		 * We're really screwed, let's trigger an error. */
		linebuf->read_buffer_full = true;
		linebuf->error_cb(linebuf, MOWGLI_EVENTLOOP_IO_READ);
		return;
	}

	if (line_start != cptr)
	{
		buffer->buflen = cptr - line_start;
		memmove(buffer->buffer, line_start, cptr - line_start);
	}
	else
		buffer->buflen = 0;
}

