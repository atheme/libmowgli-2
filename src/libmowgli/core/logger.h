/*
 * libmowgli: A collection of useful routines for programming.
 * logger.h: Event and debugging message logging.
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

#ifndef __MOWGLI_LOGGER_H__
#define __MOWGLI_LOGGER_H__

#define mowgli_log_warning(...) \
	mowgli_log_prefix("warning: ", __VA_ARGS__)

#define mowgli_log_error(...) \
 	mowgli_log_prefix("error: ", __VA_ARGS__)

#define mowgli_log_fatal(...) \
	do { \
		mowgli_log_prefix("fatal: ", __VA_ARGS__); \
		abort(); \
	} while(0)

#if defined MOWGLI_COMPILER_GCC_COMPAT
#define _FUNCARG __PRETTY_FUNCTION__
#elif defined MOWGLI_COMPILER_MSVC
#define _FUNCARG __FUNCTION__
#else
#define _FUNCARG __func__
#endif

#define mowgli_log(...) \
	mowgli_log_prefix("", __VA_ARGS__);

#define mowgli_log_prefix(prefix, ...) \
	mowgli_log_prefix_real(__FILE__, __LINE__, _FUNCARG, prefix, __VA_ARGS__);

typedef void (*mowgli_log_cb_t)(const char *);

extern char _mowgli_log_buf[4096];
extern mowgli_log_cb_t _mowgli_log_cb;

static inline void mowgli_log_prefix_real(const char *file, int line,
		const char *func, const char *prefix, const char *fmt, ...) {

	int len = snprintf(_mowgli_log_buf, 4095, "(%s:%d %s): %s", file, line,
			func, prefix);

	char *buf = &_mowgli_log_buf[len];

	va_list va;

	va_start(va, fmt);
	vsnprintf(buf, 4095 - len, fmt, va);
	va_end(va);

	_mowgli_log_cb(_mowgli_log_buf);
}

extern void mowgli_log_set_cb(mowgli_log_cb_t callback);

#endif
