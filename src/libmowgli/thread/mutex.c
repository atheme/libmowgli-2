/*
 * libmowgli: A collection of useful routines for programming.
 * mutex.c: Cross-platform mutexes.
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

#include "mowgli.h"

#if defined(_WIN32)
extern const mowgli_mutex_ops_t _mowgli_win32_mutex_ops;
#else
extern const mowgli_mutex_ops_t _mowgli_posix_mutex_ops;
#endif

extern const mowgli_mutex_ops_t _mowgli_null_mutex_ops;

static const mowgli_mutex_ops_t *_mowgli_mutex_ops = NULL;

static inline const mowgli_mutex_ops_t *
get_mutex_platform(void)
{
	/* allow for threading policy to set custom mutex ops */
	if (_mowgli_mutex_ops != NULL)
		return _mowgli_mutex_ops;

#if defined(_WIN32)
	return &_mowgli_win32_mutex_ops;
#endif

#if !defined(MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES)
	return &_mowgli_posix_mutex_ops;
#endif

	return &_mowgli_null_mutex_ops;
}

mowgli_mutex_t *
mowgli_mutex_create(void)
{
	mowgli_mutex_t *mutex = mowgli_alloc(sizeof(mowgli_mutex_t));

	return_val_if_fail(mutex != NULL, NULL);

	if (mowgli_mutex_init(mutex))
	{
		return mutex;
	}
	else
	{
		mowgli_free(mutex);
		return NULL;
	}
}

int
mowgli_mutex_init(mowgli_mutex_t *mutex)
{
	return_val_if_fail(mutex != NULL, -1);

	mutex->ops = get_mutex_platform();

	return mutex->ops->mutex_create(mutex);
}

int
mowgli_mutex_lock(mowgli_mutex_t *mutex)
{
	return_val_if_fail(mutex != NULL, -1);
	return_val_if_fail(mutex->ops != NULL, -1);

	return mutex->ops->mutex_lock(mutex);
}

int
mowgli_mutex_trylock(mowgli_mutex_t *mutex)
{
	return_val_if_fail(mutex != NULL, -1);
	return_val_if_fail(mutex->ops != NULL, -1);

	return mutex->ops->mutex_trylock(mutex);
}

int
mowgli_mutex_unlock(mowgli_mutex_t *mutex)
{
	return_val_if_fail(mutex != NULL, -1);
	return_val_if_fail(mutex->ops != NULL, -1);

	return mutex->ops->mutex_unlock(mutex);
}

int
mowgli_mutex_uninit(mowgli_mutex_t *mutex)
{
	return_val_if_fail(mutex != NULL, -1);
	return_val_if_fail(mutex->ops != NULL, -1);

	return mutex->ops->mutex_destroy(mutex);
}

void
mowgli_mutex_destroy(mowgli_mutex_t *mutex)
{
	return_if_fail(mutex != NULL);

	mowgli_mutex_uninit(mutex);
	mowgli_free(mutex);
}

void
mowgli_mutex_set_policy(mowgli_thread_policy_t policy)
{
	switch (policy)
	{
	case MOWGLI_THREAD_POLICY_DISABLED:
		_mowgli_mutex_ops = &_mowgli_null_mutex_ops;
		break;
	case MOWGLI_THREAD_POLICY_DEFAULT:
	default:
		_mowgli_mutex_ops = NULL;
	}
}
