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

static mowgli_list_t mutex_list;

#if defined(_WIN32)
extern mowgli_mutex_ops_t _mowgli_win32_mutex_ops;
#elif defined(_sun) || defined(_sco)
extern mowgli_mutex_ops_t _mowgli_sun_mutex_ops;
#elif defined(HAVE_LINUX_FUTEX_H) && defined(MOWGLI_FEATURE_HAVE_ATOMIC_OPS) && defined(MOWGLI_FEATURE_WANT_EXPERIMENTAL)
extern mowgli_mutex_ops_t _mowgli_linux_mutex_ops;
#else
extern mowgli_mutex_ops_t _mowgli_posix_mutex_ops;
#endif

extern mowgli_mutex_ops_t _mowgli_null_mutex_ops;

static mowgli_mutex_ops_t *_mowgli_mutex_ops = NULL;

void mowgli_mutex_lock_all(void)
{
	mowgli_node_t *iter;

#if !defined(_WIN32) && defined(DEBUG)
	mowgli_log("Locking all mutexes in PID %d", getpid());
#endif

	MOWGLI_ITER_FOREACH(iter, mutex_list.head)
	{
		mowgli_mutex_t *mutex = iter->data;

		mowgli_mutex_lock(mutex);
	}
}

void mowgli_mutex_unlock_all(void)
{
	mowgli_node_t *iter;

#if !defined(_WIN32) && defined(DEBUG)
	mowgli_log("Unlocking all mutexes in PID %d", getpid());
#endif

	MOWGLI_ITER_FOREACH(iter, mutex_list.head)
	{
		mowgli_mutex_t *mutex = iter->data;

		mowgli_mutex_unlock(mutex);
	}
}

static inline mowgli_mutex_ops_t *get_mutex_platform(void)
{
	/* allow for threading policy to set custom mutex ops */
	if (_mowgli_mutex_ops != NULL)
		return _mowgli_mutex_ops;

#if defined(_WIN32)
	return &_mowgli_win32_mutex_ops;
#endif

#if defined(_sun) || defined(_sco)
	return &_mowgli_sun_mutex_ops;
#endif

#if defined(HAVE_LINUX_FUTEX_H) && defined(MOWGLI_FEATURE_HAVE_ATOMIC_OPS) && defined(MOWGLI_FEATURE_WANT_EXPERIMENTAL)
	return &_mowgli_linux_mutex_ops;
#endif

#if !defined(MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES)
	return &_mowgli_posix_mutex_ops;
#endif

	return &_mowgli_null_mutex_ops;
}

int mowgli_mutex_create(mowgli_mutex_t *mutex)
{
	static bool initialized = false;
	mowgli_mutex_ops_t *mutex_ops = get_mutex_platform();

	return_val_if_fail(mutex != NULL, -1);

	if (!initialized)
	{
		if (mutex_ops->setup_fork_safety != NULL)
			mutex_ops->setup_fork_safety();

		initialized = true;
	}

	mowgli_node_add(mutex, &mutex->node, &mutex_list);

	return mutex_ops->mutex_create(mutex);
}

int mowgli_mutex_lock(mowgli_mutex_t *mutex)
{
	mowgli_mutex_ops_t *mutex_ops = get_mutex_platform();

	return_val_if_fail(mutex != NULL, -1);

	return mutex_ops->mutex_lock(mutex);
}

int mowgli_mutex_trylock(mowgli_mutex_t *mutex)
{
	mowgli_mutex_ops_t *mutex_ops = get_mutex_platform();

	return_val_if_fail(mutex != NULL, -1);

	return mutex_ops->mutex_trylock(mutex);
}

int mowgli_mutex_unlock(mowgli_mutex_t *mutex)
{
	mowgli_mutex_ops_t *mutex_ops = get_mutex_platform();

	return_val_if_fail(mutex != NULL, -1);

	return mutex_ops->mutex_unlock(mutex);
}

int mowgli_mutex_destroy(mowgli_mutex_t *mutex)
{
	mowgli_mutex_ops_t *mutex_ops = get_mutex_platform();

	return_val_if_fail(mutex != NULL, -1);

	mowgli_node_delete(&mutex->node, &mutex_list);

	return mutex_ops->mutex_destroy(mutex);
}

void mowgli_mutex_set_policy(mowgli_thread_policy_t policy)
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
