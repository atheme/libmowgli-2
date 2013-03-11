/*
 * libmowgli: A collection of useful routines for programming.
 * attributes.h: Compiler attributes to help write better code
 *
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

#ifndef __MOWGLI_PLATFORM_ATTRIBUTES_H__
#define __MOWGLI_PLATFORM_ATTRIBUTES_H__

/* Deprecated */
#if defined MOWGLI_COMPILER_GCC_COMPAT
# define MOWGLI_DEPRECATED(proto, msg) proto __attribute__((deprecated(msg)))
#elif defined MOWGLI_COMPILER_MSVC
# define MOWGLI_DEPRECATED(proto, msg) __declspec(deprecated(msg)) proto
#else
# define MOWGLI_DEPRECATED(proto, msg) proto
#endif

/* printf, n is number of args to skip to get to fmt */
#if defined MOWGLI_COMPILER_GCC_COMPAT
# define MOWGLI_PRINTF(proto, n) proto __attribute__((format(printf, n, n + 1)))
#else
# define MOWGLI_PRINTF(proto) proto
#endif

/* Hot and cold paths */
#if MOWGLI_COMPILER_GCC_VERSION > 403000
# define MOWGLI_HOT(proto) proto __attribute__((hot))
# define MOWGLI_COLD(proto) proto __attribute__((cold))
#else
# define MOWGLI_HOT(proto) proto
# define MOWGLI_COLD(proto) proto
#endif

#if MOWGLI_COMPILER_GCC_VERSION > 408000
# define MOWGLI_HOT_LABEL(label) label __attribute__((hot))
# define MOWGLI_COLD_LABEL(label) label __attribute__((cold))
#else
# define MOWGLI_HOT_LABEL(label) label
# define MOWGLI_COLD_LABEL(label) label
#endif

/* malloc, n is the arg used for allocation size */
#ifdef MOWGLI_COMPILER_GCC_COMPAT
# define MOWGLI_MALLOC(proto, n) \
	proto __attribute__((malloc, alloc_size(n), warn_unused_result))
#else
# define MOWGLI_MALLOC(proto, n) \
	proto
#endif

#endif
