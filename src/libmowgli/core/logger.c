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

char buf1[65535];
char buf2[65535];

void mowgli_log_cb_default(const char *buf) {
	fprintf(stderr, "%s\n", buf);
}

static mowgli_log_cb_t mowgli_log_cb = mowgli_log_cb_default;

/* TODO: remove next time there is a LIB_MAJOR bump */
void mowgli_log_real(const char *file, int line, const char *func,
		const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf1, 65534, fmt, va);
	va_end(va);

	snprintf(buf2, 65534, "(%s:%d) [%s]: %s", file, line, func, buf1);

	mowgli_log_cb(buf2);
}

void mowgli_log_prefix_real(const char *file, int line, const char *func,
		const char *prefix, const char *fmt, ...) {
	va_list va;

	va_start(va, fmt);
	vsnprintf(buf1, 65534, fmt, va);
	va_end(va);

	snprintf(buf2, 65534, "(%s:%d) [%s]: %s%s", file, line, func,
			prefix, buf1);

	mowgli_log_cb(buf2);
}

void mowgli_log_set_cb(mowgli_log_cb_t callback) {
	return_if_fail(callback != NULL);

	mowgli_log_cb = callback;
}

/* TODO: remove next time there is a LIB_MAJOR bump */
void mowgli_soft_assert_log(const char *asrt, const char *file, int line,
		const char *function) {
	char buf[65535];

	snprintf(buf, sizeof buf, "(%s:%d) [%s]: critical: Assertion '%s' failed.",
			file, line, function, asrt);

	mowgli_log_cb(buf);
}
