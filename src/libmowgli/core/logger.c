/*
 * libmowgli: A collection of useful routines for programming.
 * logger.c: Event and debugging message logging.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice is present in all copies.
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

#define MOWGLI_LOG_BUF_SIZE 4096

char _mowgli_log_buf[MOWGLI_LOG_BUF_SIZE];
mowgli_log_cb_t _mowgli_log_cb;

void
mowgli_log_cb_default(const char *buf)
{
	fprintf(stderr, "%s\n", buf);
}

void
mowgli_log_bootstrap()
{
	_mowgli_log_cb = mowgli_log_cb_default;
}

void
mowgli_log_set_cb(mowgli_log_cb_t callback)
{
	return_if_fail(callback != NULL);

	_mowgli_log_cb = callback;
}

void
mowgli_log_prefix_real(const char *file, int line, const char *func, const char *prefix, const char *fmt, ...)
{
	int len = snprintf(_mowgli_log_buf, MOWGLI_LOG_BUF_SIZE, "(%s:%d %s): %s",
			   file, line, func, prefix);

	char *buf = &_mowgli_log_buf[len];

	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, MOWGLI_LOG_BUF_SIZE - len, fmt, va);
	va_end(va);

	_mowgli_log_cb(_mowgli_log_buf);
}

/* TODO: remove next time there is a LIB_MAJOR bump */
void
mowgli_log_real(const char *file, int line, const char *func, const char *fmt, ...)
{
	int len = snprintf(_mowgli_log_buf, 4095, "(%s:%d %s): ", file, line,
			   func);

	char *buf = &_mowgli_log_buf[len];

	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, 4095 - len, fmt, va);
	va_end(va);

	_mowgli_log_cb(_mowgli_log_buf);
}

/* TODO: remove next time there is a LIB_MAJOR bump */
void
mowgli_soft_assert_log(const char *asrt, const char *file, int line, const char *function)
{
	snprintf(_mowgli_log_buf, 4095,
		 "(%s:%d %s): critical: Assertion '%s' failed.", file, line,
		 function, asrt);

	_mowgli_log_cb(_mowgli_log_buf);
}
