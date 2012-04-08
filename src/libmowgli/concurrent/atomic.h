/*
 * libmowgli: A collection of useful routines for programming.
 * atomic.h: Wrapper for common atomic functions
 *
 * Copyright (c) 2012 Patrick McFarland <pmcfarland@adterrasperaspera.com>
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

/* This wrapper uses a lot of layers of macros to get things done. Function
 * names can come out like mowgli_atomic_load_uchar for unsigned char, or
 * mowgli_atomic_load_int8_t for int8_t or mowgli_atomic_load_pointer for
 * pointers. Almost all of the C89 and C99 basic types are supported except
 * for floating point types.
 *
 * Atomic operations currently wrapped are load, store, compare_exchange,
 * and (full barrier) synchronize.
 *
 * Hardware does not typically support atomic operations on larger than the
 * native size of the hardware (ie, 4 or 8 bytes).
 */

/* Define MOWGLI_ATOMIC_DEBUG to force it not to use compiler builtins and
 * use the mutex-based fallbacks instead. Use this consistently! Using it
 * in one .c file and not another will break it.
 */

#ifndef __MOWGLI_ATOMIC_H__
#define __MOWGLI_ATOMIC_H__

extern void mowgli_atomic_bootstrap();

#if defined MOWGLI_COMPILER_GCC_COMPAT
#if defined MOWGLI_COMPILER_GCC
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 1
#define MOWGLI_ATOMIC_GCC
#endif
#elif defined MOWGLI_COMPILER_CLANG
#if __has_builtin(__sync_swap)
#define MOWGLI_ATOMIC_GCC
#endif
#elif defined MOWGLI_COMPILER_ICC
#if defined __ICC && __ICC >= 1100
#define MOWGLI_ATOMIC_GCC
#endif
#endif
#elif defined __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L && !defined __STDC_NO_ATOMICS__
#define MOWGLI_ATOMIC_C11
#endif
#endif

#if !defined MOWGLI_ATOMIC_GCC && !defined MOWGLI_ATOMIC_C11
#define MOWGLI_ATOMIC_FALLBACK
#endif

#if !defined MOWGLI_ATOMIC_DEBUG && defined MOWGLI_ATOMIC_GCC
#define mowgli_atomic(type) volatile type

#define mowgli_atomic_load_function(type, mangle) \
static inline type mowgli_atomic_load_##mangle (mowgli_atomic(type) *atomic) \
{ \
	return (type)__sync_fetch_and_add(atomic, 0); \
}

#define mowgli_atomic_store_function(type, mangle) \
static inline type mowgli_atomic_store_##mangle (mowgli_atomic(type) *atomic, type value) \
{ \
	return (type)__sync_lock_test_and_set(atomic, value); \
}

#define mowgli_atomic_compare_exchange_function(type, mangle) \
static inline type mowgli_atomic_compare_exchange_##mangle (mowgli_atomic(type) *atomic, type expected, type desired) \
{ \
	return (type)__sync_val_compare_and_swap(atomic, expected, desired); \
}

static inline void mowgli_atomic_synchronize() {
	__sync_synchronize();
}

#elif !defined MOWGLI_ATOMIC_DEBUG && defined MOWGLI_ATOMIC_C11
#include <stdatomic.h>

#define mowgli_atomic(type) _Atomic(type)

#define mowgli_atomic_load_function(type, mangle) \
static inline type mowgli_atomic_load_##mangle (mowgli_atomic(type) *atomic) \
{ \
	return (type)atomic_load(atomic); \
}

#define mowgli_atomic_store_function(type, mangle) \
static inline type mowgli_atomic_store_##mangle (mowgli_atomic(type) *atomic, type value) \
{ \
	return (type)atomic_store(atomic, value); \
}

#define mowgli_atomic_compare_exchange_function(type, mangle) \
static inline type mowgli_atomic_compare_exchange_##mangle (mowgli_atomic(type) *atomic, type expected, type desired) \
{ \
	return (type)atomic_compare_exchange_strong(atomic, expected, desired); \
}

static inline void mowgli_atomic_synchronize() {
	atomic_thread_fence(memory_order_seq_cst);
}

#elif defined MOWGLI_ATOMIC_DEBUG || defined MOWGLI_ATOMIC_FALLBACK
#define mowgli_atomic(type) volatile type

extern mowgli_mutex_t mowgli_atomic_mutex[256];

typedef union {
	mowgli_atomic(void) *address;
	struct {
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
#ifdef MOWGLI_CPU_BITS_64
		uint8_t e;
		uint8_t f;
		uint8_t g;
		uint8_t h;
#endif
	} part;
} mowgli_atomic_address_mangle_t;

static inline mowgli_mutex_t *mowgli_atomic_mutex_lookup(mowgli_atomic(void) *address) {
	mowgli_atomic_address_mangle_t mangle;

	mangle.address = address;

	uint8_t ab = mangle.part.a ^ mangle.part.b;
	uint8_t cd = mangle.part.c ^ mangle.part.d;

#ifdef MOWGLI_CPU_BITS_64
	uint8_t ef = mangle.part.e ^ mangle.part.f;
	uint8_t gh = mangle.part.g ^ mangle.part.h;
#endif

	uint8_t abcd = ab ^ cd;

#ifdef MOWGLI_CPU_BITS_64
	uint8_t efgh = ef ^ gh;

	uint8_t abcdefgh = abcd ^ efgh;

	return &mowgli_atomic_mutex[abcdefgh];
#else
	return &mowgli_atomic_mutex[abcd];
#endif
}

#define mowgli_atomic_load_function(type, mangle) \
static inline type mowgli_atomic_load_##mangle (mowgli_atomic(type) *atomic) \
{ \
	type result; \
\
	mowgli_mutex_lock(mowgli_atomic_mutex_lookup(atomic)); \
	result = (type) *atomic; \
	mowgli_mutex_unlock(mowgli_atomic_mutex_lookup(atomic)); \
\
	return result; \
}

#define mowgli_atomic_store_function(type, mangle) \
static inline type mowgli_atomic_store_##mangle (mowgli_atomic(type) *atomic, type value) \
{ \
	type result; \
\
	mowgli_mutex_lock(mowgli_atomic_mutex_lookup(atomic)); \
	result = (type) *atomic; \
	*atomic = value; \
	mowgli_mutex_unlock(mowgli_atomic_mutex_lookup(atomic)); \
\
	return result; \
}

#define mowgli_atomic_compare_exchange_function(type, mangle) \
static inline type mowgli_atomic_compare_exchange_##mangle (mowgli_atomic(type) *atomic, type expected, type desired) \
{ \
	type result; \
\
	mowgli_mutex_lock(mowgli_atomic_mutex_lookup(atomic)); \
	result = (type) *atomic; \
\
	if(*atomic == expected) \
		*atomic = desired; \
\
	mowgli_mutex_unlock(mowgli_atomic_mutex_lookup(atomic)); \
\
	return result; \
}

static inline void mowgli_atomic_synchronize() { }

#endif

#define mowgli_atomic_type(type, mangle) \
mowgli_atomic_load_function(type, mangle) \
mowgli_atomic_store_function(type, mangle) \
mowgli_atomic_compare_exchange_function(type, mangle)

#define mowgli_atomic_signed_type(type, mangle) \
mowgli_atomic_type(unsigned type, u##mangle) \
mowgli_atomic_type(signed type, mangle)

#define mowgli_atomic_c99_t_type(type) \
mowgli_atomic_type(type, type) \
mowgli_atomic_type(u##type, u##type)

#define mowgli_atomic_c99_width_type(bitwidth) \
mowgli_atomic_c99_t_type(int##bitwidth##_t) \
mowgli_atomic_c99_t_type(int_least##bitwidth##_t) \
mowgli_atomic_c99_t_type(int_fast##bitwidth##_t)

mowgli_atomic_signed_type(char, char)
mowgli_atomic_signed_type(int, int)
mowgli_atomic_signed_type(long, long)
mowgli_atomic_signed_type(long long, longlong)
mowgli_atomic_type(bool, bool)
mowgli_atomic_type(void *, pointer)

mowgli_atomic_type(size_t, size_t)
mowgli_atomic_type(ptrdiff_t, ptrdiff_t)
mowgli_atomic_type(wchar_t, wchar_t)

mowgli_atomic_c99_width_type(8)
mowgli_atomic_c99_width_type(16)
mowgli_atomic_c99_width_type(32)
mowgli_atomic_c99_width_type(64)
mowgli_atomic_c99_t_type(intptr_t)
mowgli_atomic_c99_t_type(intmax_t)

#undef mowgli_atomic_load_function
#undef mowgli_atomic_store_function
#undef mowgli_atomic_compare_exchange_function
#undef mowgli_atomic_type
#undef mowgli_atomic_signed_type
#undef mowgli_atomic_c99_t_type
#undef mowgli_atomic_c99_width_type

#endif

