/*
 * Copyright (c) 2011, 2012 William Pitcock <nenolod@dereferenced.org>.
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

#ifndef MOWGLI_SRC_LIBMOWGLI_EVENTLOOP_EVENTLOOP_INTERNAL_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_EVENTLOOP_EVENTLOOP_INTERNAL_H_INCLUDE_GUARD 1

#include "eventloop/eventloop.h"
#include "platform/autoconf.h"

extern mowgli_eventloop_ops_t _mowgli_null_pollops;

#ifdef HAVE_PORT_CREATE
extern mowgli_eventloop_ops_t _mowgli_ports_pollops;
#endif

#ifdef HAVE_DISPATCH_BLOCK
extern mowgli_eventloop_ops_t _mowgli_qnx_pollops;
#endif

#ifdef HAVE_SELECT
extern mowgli_eventloop_ops_t _mowgli_select_pollops;
#endif

#ifdef HAVE_POLL_H
extern mowgli_eventloop_ops_t _mowgli_poll_pollops;
#endif

#ifdef HAVE_SYS_EPOLL_H
extern mowgli_eventloop_ops_t _mowgli_epoll_pollops;
#endif

#ifdef HAVE_KQUEUE
extern mowgli_eventloop_ops_t _mowgli_kqueue_pollops;
#endif

#if 0
extern mowgli_eventloop_ops_t _mowgli_winsock_pollops;
#endif

#endif /* MOWGLI_SRC_LIBMOWGLI_EVENTLOOP_EVENTLOOP_INTERNAL_H_INCLUDE_GUARD */
