/*
 * libmowgli: A collection of useful routines for programming.
 * mutex.h: Cross-platform mutexes.
 *
 * Copyright (c) 2011 William Pitcock <nenolod@dereferenced.org>
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

#ifndef MOWGLI_SRC_LIBMOWGLI_THREAD_MUTEX_INTERNAL_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_THREAD_MUTEX_INTERNAL_H_INCLUDE_GUARD 1

#include "platform/autoconf.h"
#include "thread/mutex.h"

#ifdef _WIN32
extern const mowgli_mutex_ops_t _mowgli_win32_mutex_ops;
#else
extern const mowgli_mutex_ops_t _mowgli_posix_mutex_ops;
#endif

extern const mowgli_mutex_ops_t _mowgli_null_mutex_ops;

#endif /* MOWGLI_SRC_LIBMOWGLI_THREAD_MUTEX_INTERNAL_H_INCLUDE_GUARD */
