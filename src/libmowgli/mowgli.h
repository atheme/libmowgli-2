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

#ifndef MOWGLI_SRC_LIBMOWGLI_MOWGLI_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_MOWGLI_H_INCLUDE_GUARD 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MOWGLI_CORE
#  include "platform/autoconf.h"
#endif

#include "base/argstack.h"
#include "base/bitvector.h"
#include "base/formatter.h"
#include "base/hash.h"
#include "base/hook.h"
#include "base/memslice.h"
#include "base/mowgli_signal.h"
#include "base/random.h"
#include "container/dictionary.h"
#include "container/index.h"
#include "container/list.h"
#include "container/patricia.h"
#include "container/queue.h"
#include "core/allocation_policy.h"
#include "core/allocator.h"
#include "core/alloc.h"
#include "core/assert.h"
#include "core/bootstrap.h"
#include "core/heap.h"
#include "core/iterator.h"
#include "core/logger.h"
#include "core/mowgli_string.h"
#include "core/process.h"
#include "dns/dns.h"
#include "dns/evloop_res.h"
#include "dns/evloop_reslib.h"
#include "eventloop/eventloop.h"
#include "ext/confparse.h"
#include "ext/error_backtrace.h"
#include "ext/getopt_long.h"
#include "ext/global_storage.h"
#include "ext/json.h"
#include "ext/json-inline.h"
#include "ext/proctitle.h"
#include "ext/program_opts.h"
#include "linebuf/linebuf.h"
#include "module/module.h"
#include "object/class.h"
#include "object/message.h"
#include "object/metadata.h"
#include "object/object.h"
#include "platform/attributes.h"
#include "platform/cacheline.h"
#include "platform/constructor.h"
#include "platform/machine.h"
#include "thread/mutex.h"
#include "thread/thread.h"
#include "vio/vio.h"

#ifdef __cplusplus
}
#endif

#endif /* MOWGLI_SRC_LIBMOWGLI_MOWGLI_H_INCLUDE_GUARD */
