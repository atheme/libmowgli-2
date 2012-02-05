/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>.
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

#ifndef __MOWGLI_ATOMIC_H__
#define __MOWGLI_ATOMIC_H__

typedef struct {
	volatile int counter;
} mowgli_atomic_t;

/* basic ops */
#define mowgli_atomic_init(start) { .counter = (start) }
#define mowgli_atomic_get(a)	  ((a)->counter)
#define mowgli_atomic_put(a, b)	  ((a)->counter = (b))

/* arch-specific stuff */

#if defined(__i386__) || defined(__x86_64__)

#define MOWGLI_FEATURE_HAVE_ATOMIC_OPS

static inline void mowgli_atomic_add(mowgli_atomic_t *a, int b)
{
	return_if_fail(a != NULL);

	asm volatile("lock; addl %1, %0" : "=m" (a->counter) : "ir" (b), "m" (a->counter));
}

static inline int mowgli_atomic_add_and_test(mowgli_atomic_t *a, int b)
{
	unsigned char c;

	return_val_if_fail(a != NULL, 0);

	asm volatile("lock; addl %2, %0; sete %1" : "=m" (a->counter), "=qm" (c) : "ir" (b), "m" (a->counter) : "memory");

	return c;
}

static inline void mowgli_atomic_sub(mowgli_atomic_t *a, int b)
{
	return_if_fail(a != NULL);

	asm volatile("lock; subl %1, %0" : "=m" (a->counter) : "ir" (b), "m" (a->counter));
}

static inline int mowgli_atomic_sub_and_test(mowgli_atomic_t *a, int b)
{
	unsigned char c;

	return_val_if_fail(a != NULL, 0);

	asm volatile("lock; subl %2, %0; sete %1" : "=m" (a->counter), "=qm" (c) : "ir" (b), "m" (a->counter) : "memory");

	return c;
}

static inline void mowgli_atomic_inc(mowgli_atomic_t *a)
{
	return_if_fail(a != NULL);

	asm volatile("lock; incl %0" : "=m" (a->counter) : "m" (a->counter));
}

static inline int mowgli_atomic_inc_and_test(mowgli_atomic_t *a)
{
	unsigned char c;

	return_val_if_fail(a != NULL, 0);

	asm volatile("lock; incl %0; sete %1" : "=m" (a->counter), "=qm" (c) : "m" (a->counter) : "memory");

	return c;
}

static inline void mowgli_atomic_dec(mowgli_atomic_t *a)
{
	return_if_fail(a != NULL);

	asm volatile("lock; decl %0" : "=m" (a->counter) : "m" (a->counter));
}

static inline int mowgli_atomic_dec_and_test(mowgli_atomic_t *a)
{
	unsigned char c;

	return_val_if_fail(a != NULL, 0);

	asm volatile("lock; decl %0; sete %1" : "=m" (a->counter), "=qm" (c) : "m" (a->counter) : "memory");

	return c;
}

#else

#error mowgli_atomic_t is not ported to your platform yet.

#endif

#endif
