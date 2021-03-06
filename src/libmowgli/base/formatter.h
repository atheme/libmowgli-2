/*
 * libmowgli: A collection of useful routines for programming.
 * formatter.h: Reusable formatting.
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

#ifndef MOWGLI_SRC_LIBMOWGLI_BASE_FORMATTER_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_BASE_FORMATTER_H_INCLUDE_GUARD 1

#include "base/argstack.h"
#include "core/stdinc.h"

extern void mowgli_formatter_format(char *buf, size_t bufstr, const char *fmtstr, const char *descstr, ...);
extern void mowgli_formatter_print(const char *fmtstr, const char *descstr, ...);
extern void mowgli_formatter_format_from_argstack(char *buf, size_t bufstr, const char *fmtstr, const char *descstr, mowgli_argstack_t *stack);

#endif /* MOWGLI_SRC_LIBMOWGLI_BASE_FORMATTER_H_INCLUDE_GUARD */
