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

/* Define MOWGLI_ATOMIC_USE_FALLBACK to force it not to use compiler
 * builtins. Use this consistently! Using it in one .c file and not
 * another will break it.
 */

#ifndef __MOWGLI_ATOMIC_H__
#define __MOWGLI_ATOMIC_H__

#if !defined MOWGLI_ATOMIC_USE_FALLBACK && defined HAVE_ATOMIC_BUILTINS_GCC
#define mowgli_atomic(type) volatile type

#define mowgli_atomic_load(type, atomic) (type)__sync_fetch_and_add(&atomic, 0)
#define mowgli_atomic_store(type, atomic, value) (type)__sync_lock_test_and_set(&atomic, value)
#define mowgli_atomic_compare_exchange(type, atomic, expected, desired) (type)__sync_val_compare_and_swap(&atomic, expected, desired)
#endif

#if !defined MOWGLI_ATOMIC_USE_FALLBACK && defined HAVE_ATOMIC_BUILTINS_C11
#define mowgli_atomic(type) _Atomic type

#define mowgli_atomic_load(type, atomic) (type)atomic_load(&atomic)
#define mowgli_atomic_store(type, atomic, value) (type)atomic_store(&atomic, value)
#define mowgli_atomic_compare_exchange(type, atomic, expected, desired) (type)atomic_compare_exchange_strong(&atomic, expected, desired)
#endif

#if defined MOWGLI_ATOMIC_USE_FALLBACK || !defined HAVE_ATOMIC_BUILTINS
typedef unsigned long mowgli_atomic_value_t;

typedef struct {
	mowgli_atomic_value_t value;
	mowgli_mutex_t mutex;
} mowgli_atomic_t;

#define mowgli_atomic(type) mowgli_atomic_t

#define mowgli_atomic_load(type, atomic) (type)mowgli_atomic_load_fallback(&atomic)
static inline mowgli_atomic_value_t mowgli_atomic_load_fallback(mowgli_atomic_t *atomic)
{
	mowgli_atomic_value_t value;

	mowgli_mutex_lock(&atomic->mutex);
	value = atomic->value;
	mowgli_mutex_unlock(&atomic->mutex);

	return value;
}

#define mowgli_atomic_store(type, atomic, value) (type)mowgli_atomic_store_fallback(&atomic, (mowgli_atomic_value_t)value)
static inline mowgli_atomic_value_t mowgli_atomic_store_fallback(mowgli_atomic_t *atomic, mowgli_atomic_value_t value)
{
	mowgli_atomic_value_t oldvalue;

	mowgli_mutex_lock(&atomic->mutex);
	oldvalue = atomic->value;
	atomic->value = value;
	mowgli_mutex_unlock(&atomic->mutex);

	return oldvalue;
}

#define mowgli_atomic_compare_exchange(type, atomic, expected, desired) (type)mowgli_atomic_compare_exchange_fallback(&atomic, (mowgli_atomic_value_t)expected, (mowgli_atomic_value_t)desired)
static inline mowgli_atomic_value_t mowgli_atomic_compare_exchange_fallback(mowgli_atomic_t *atomic, mowgli_atomic_value_t expected, mowgli_atomic_value_t desired)
{
	mowgli_atomic_value_t oldvalue;

	mowgli_mutex_lock(&atomic->mutex);
	oldvalue = atomic->value;

	if(atomic->value == expected)
		atomic->value = desired;
	mowgli_mutex_unlock(&atomic->mutex);

	return oldvalue;
}
#endif

#endif

