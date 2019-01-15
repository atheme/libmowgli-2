/*
 * libmowgli: A collection of useful routines for programming.
 * logger.h: Event and debugging message logging.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 * Copyright (c) 2013 Patrick McFarland <pmcfarland@adterrasperaspera.com>
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

#ifndef MOWGLI_SRC_LIBMOWGLI_CORE_LOGGER_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_CORE_LOGGER_H_INCLUDE_GUARD 1

#include "platform/attributes.h"

#if defined(MOWGLI_COMPILER_GCC_COMPAT)
#  define MOWGLI_FUNC_NAME                              __PRETTY_FUNCTION__
#else
#  if defined(MOWGLI_COMPILER_MSVC)
#    define MOWGLI_FUNC_NAME                            __FUNCTION__
#  else
#    define MOWGLI_FUNC_NAME                            __func__
#  endif
#endif

typedef void (*mowgli_log_cb_t)(const char *);

extern void mowgli_log_set_cb(mowgli_log_cb_t);

extern void mowgli_log_real(const char *file, int line, const char *func, const char *fmt, ...)
    MOWGLI_FATTR_PRINTF(4, 5);

extern void mowgli_log_prefix_real(const char *file, int line, const char *func, const char *prefix, const char *fmt, ...)
    MOWGLI_FATTR_PRINTF(5, 6);

extern void mowgli_soft_assert_log(const char *asrt, const char *file, int line, const char *function);

#define mowgli_log_prefix(prefix, ...)                  \
    mowgli_log_prefix_real(__FILE__, __LINE__, MOWGLI_FUNC_NAME, prefix, __VA_ARGS__)

#define mowgli_log(...)                                 mowgli_log_prefix("", __VA_ARGS__)
#define mowgli_log_warning(...)                         mowgli_log_prefix("warning: ", __VA_ARGS__)
#define mowgli_log_error(...)                           mowgli_log_prefix("error: ", __VA_ARGS__)

#define mowgli_log_fatal(...)                           \
    do                                                  \
    {                                                   \
        mowgli_log_prefix("fatal: ", __VA_ARGS__);      \
        abort();                                        \
    } while (0)

#endif /* MOWGLI_SRC_LIBMOWGLI_CORE_LOGGER_H_INCLUDE_GUARD */
