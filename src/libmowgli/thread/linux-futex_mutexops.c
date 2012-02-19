/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_thread.c: Cross-platform threading helper routines.
 *
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

#include "mowgli.h"

#if defined(HAVE_LINUX_FUTEX_H) && defined(MOWGLI_FEATURE_HAVE_ATOMIC_OPS) && defined(MOWGLI_FEATURE_WANT_EXPERIMENTAL)
#warning the futex implementation of mowgli_mutex_t is experimental

#include <sys/syscall.h>

typedef struct {
	mowgli_atomic_t atom;
} futex_mutex_t;

static int mowgli_linux_mutex_create(mowgli_mutex_t *mutex)
{
	futex_mutex_t *fm;

	fm = mowgli_alloc(sizeof(futex_mutex_t));
	if (fm == NULL)
		return -1;

	mowgli_atomic_put(&fm->atom, 0);

	mutex->mutex = fm;

	return 0;
}

static int mowgli_linux_mutex_lock(mowgli_mutex_t *mutex)
{
	futex_mutex_t *fm;
	int value;

	fm = mutex->mutex;

	for (value = mowgli_atomic_add_and_test(&fm->atom, 2); value != 0; value = mowgli_atomic_add_and_test(&fm->atom, 2))
		syscall(__NR_futex, &fm->atom, FUTEX_WAIT_PRIVATE, 2, NULL);

	return 0;
}

static int mowgli_linux_mutex_trylock(mowgli_mutex_t *mutex)
{
	futex_mutex_t *fm = mutex->mutex;

	return mowgli_atomic_get(&fm->atom) != 2 ? 0 : EBUSY;
}

static int mowgli_linux_mutex_unlock(mowgli_mutex_t *mutex)
{
	futex_mutex_t *fm = mutex->mutex;

	if (!mowgli_atomic_dec_and_test(&fm->atom))
	{
		mowgli_atomic_sub(&fm->atom, 1);
		syscall(__NR_futex, &fm->atom, FUTEX_WAKE_PRIVATE, 1, NULL);
	}

	return 0;
}

static int mowgli_linux_mutex_destroy(mowgli_mutex_t *mutex)
{
	futex_mutex_t *fm = mutex->mutex;

	mowgli_free(fm);

	return 0;
}

mowgli_mutex_ops_t _mowgli_linux_mutex_ops = {
	.mutex_create = mowgli_linux_mutex_create,
	.mutex_lock = mowgli_linux_mutex_lock,
	.mutex_trylock = mowgli_linux_mutex_trylock,
	.mutex_unlock = mowgli_linux_mutex_unlock,
	.mutex_destroy = mowgli_linux_mutex_destroy,
};

#endif
