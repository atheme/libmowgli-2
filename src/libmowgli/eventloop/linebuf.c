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

static int mowgli_linebuf_default_read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf, char *bufpos);
static int mowgli_linebuf_default_write_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf, char *bufpos);
static void mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata);
static void mowgli_linebuf_process(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf);

mowgli_linebuf_t *
mowgli_linebuf_create(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_cb_t *cb)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_linebuf_t *linebuf;

	if (linebuf_heap == NULL)
		linebuf_heap = mowgli_heap_create(sizeof(mowgli_linebuf_t), 16, BH_NOW);
	
	linebuf = mowgli_heap_alloc(linebuf_heap);

	linebuf->userdata = pollable->userdata;
	pollable->userdata = (void *)linebuf;

	linebuf->delim = "\r\n"; /* Sane default */
	linebuf->read_cb = mowgli_linebuf_default_read_cb; /* Also sane default, change if you use something weird e.g., SSL */
	linebuf->write_cb = mowgli_linebuf_default_write_cb;
	linebuf->readline_cb = cb;

	linebuf->remote_hangup = false;
	linebuf->read_buffer_full = false;
	linebuf->err = 0;

	linebuf->readbuf.buffer = NULL;
	linebuf->readbuf.buflen = linebuf->readbuf.maxbuflen = 0;
	linebuf->writebuf.buffer = NULL;
	linebuf->writebuf.buflen = linebuf->writebuf.maxbuflen = 0;

	mowgli_pollable_setselect(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ, mowgli_linebuf_read_data);
	mowgli_pollable_setselect(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE, mowgli_linebuf_write_data);

	return linebuf;
}

void mowgli_linebuf_destroy(mowgli_linebuf_t *linebuf)
{
	mowgli_free(linebuf->readbuf.buffer);
	mowgli_free(linebuf->writebuf.buffer);
	mowgli_heap_free(linebuf_heap, linebuf);
}

/* Call this before using 
 * A "one size fits all" approach is inadequate here, so you DIY
 */
void mowgli_linebuf_setbuflen(mowgli_linebuf_buf_t *buffer, size_t buflen)
{
	return_if_fail(buffer != NULL);

	if (buffer->buffer == NULL)
		buffer->buffer = mowgli_alloc(buflen);
	else
	{
		char tmpbuf[buffer->maxbuflen];
		memcpy(tmpbuf, buffer->buffer, buffer->maxbuflen); /* Copy into tmp buffer */

		/* Free old buffer and reallocate */
		mowgli_free(buffer->buffer);
		buffer->buffer = mowgli_alloc(buflen);

		/* Copy into new buffer using old buffer size */
		memcpy(buffer->buffer, tmpbuf, buffer->maxbuflen);
	}

	buffer->maxbuflen = buflen;
}

/* This will be going away after VIO is integrated */
static int mowgli_linebuf_default_read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf, char *bufpos)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	int ret;

	if (buffer->maxbuflen - buffer->buflen == 0)
	{
		linebuf->read_buffer_full = true;
		return 0; /* Ugh, buffer full :( */
	}

	if ((ret = read(pollable->fd, bufpos, buffer->maxbuflen)) == 0)
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
static int mowgli_linebuf_default_write_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf, char *bufpos)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_linebuf_buf_t *buffer = &(linebuf->writebuf);
	int ret;

	if (buffer->buflen == 0)
		return 0; /* Nothing to do */

	if ((ret = write(pollable->fd, bufpos, buffer->buflen)) == -1)
	{
		if (errno != EAGAIN)
			linebuf->err = errno;

		return 0;
	}
	else if (ret < buffer->buflen)
		memmove(bufpos, bufpos + ret, buffer->buflen - ret);

	return ret;
}

static void mowgli_linebuf_read_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_pollable_t *pollable = mowgli_eventloop_io_pollable(io);
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *)userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	int ret;

	if (dir != MOWGLI_EVENTLOOP_IO_READ)
		return; /* We're not here to do anything else you dolt */

	if ((ret = linebuf->read_cb(eventloop, io, linebuf, buffer->buffer + buffer->buflen)) == 0)
		return;

	buffer->buflen += ret;
	mowgli_linebuf_process(eventloop, pollable, linebuf);
}

static void mowgli_linebuf_write_data(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_linebuf_t *linebuf = (mowgli_linebuf_t *)userdata;
	mowgli_linebuf_buf_t *buffer = &(linebuf->writebuf);
	int ret;

	if (dir != MOWGLI_EVENTLOOP_IO_WRITE)
		return; /* We're not here to do anything else you dolt */

	if ((ret = linebuf->write_cb(eventloop, io, linebuf, buffer->buffer)) == 0)
		return;

	buffer->buflen -= ret;
}

void mowgli_linebuf_write(mowgli_linebuf_t *linebuf, const char *data, int len)
{
	char *ptr = linebuf->writebuf.buffer + linebuf->writebuf.buflen;
	int delim_len = strlen(linebuf->delim);

	return_if_fail(linebuf->writebuf.buflen + len + delim_len <= linebuf->writebuf.maxbuflen);

	memcpy((void *)ptr, data, len);
	memcpy((void *)(ptr + len), linebuf->delim, delim_len);

	linebuf->writebuf.buflen += len + delim_len;
}

static void mowgli_linebuf_process(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_linebuf_t *linebuf)
{
	mowgli_linebuf_buf_t *buffer = &(linebuf->readbuf);
	size_t delim_len = strlen(linebuf->delim);
	char *line_start = buffer->buffer;
	char *buf_end = buffer->buffer + buffer->buflen;
	char *cptr;

	printf("Debug: buflen %d\n", buffer->buflen);

	return_if_fail(buffer->buflen > 0);

	for (cptr = line_start; cptr < buf_end; cptr++)
	{
		int c = memcmp((void *)cptr, linebuf->delim, delim_len);
		if (c != 0)
			continue;

		linebuf->readline_cb(eventloop, io, line_start, cptr - line_start, linebuf->userdata);
		cptr += delim_len;
		line_start = cptr;
	}

	if (cptr - line_start > 0)
	{
		memmove(buffer->buffer, cptr, cptr - line_start);
		buffer->buflen = cptr - line_start;
	}
	else
		buffer->buflen = 0;
}

