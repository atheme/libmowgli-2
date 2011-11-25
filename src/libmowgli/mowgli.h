/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli.h: Base header for libmowgli. Includes everything.
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

#ifndef __MOWGLI_STAND_H__
#define __MOWGLI_STAND_H__

#ifdef __cplusplus
# define MOWGLI_DECLS_START extern "C" {
# define MOWGLI_DECLS_END   }
#else
# define MOWGLI_DECLS_START
# define MOWGLI_DECLS_END
#endif

#ifdef MOWGLI_CORE
# include "platform/autoconf.h"
#endif
#ifdef _WIN32
# include "platform/win32/win32_stdinc.h"
#endif

#include "core/stdinc.h"

MOWGLI_DECLS_START

#include "core/logger.h"
#include "core/assert.h"
#include "core/exception.h"
#include "core/iterator.h"

#include "core/alloc.h"
#include "atomic/spinlock.h"
#include "container/list.h"
#include "object/class.h"
#include "object/object.h"
#include "core/allocation_policy.h"
#include "container/patricia.h"
#include "base/mempool.h"
#include "module/module.h"
#include "container/queue.h"
#include "base/hash.h"
#include "core/heap.h"
#include "core/init.h"
#include "base/bitvector.h"
#include "base/hook.h"
#include "base/mowgli_signal.h"
#include "ext/error_backtrace.h"
#include "base/random.h"
#include "base/argstack.h"
#include "object/message.h"
#include "object/metadata.h"
#include "ext/global_storage.h"
#include "core/mowgli_string.h"
#include "core/allocator.h"
#include "base/formatter.h"
#include "container/index.h"
#include "core/thread.h"
#include "ext/confparse.h"
#include "eventloop/eventloop.h"

MOWGLI_DECLS_END

#endif

