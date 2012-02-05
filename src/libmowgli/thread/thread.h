/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_thread.h: Cross-platform threading helper routines.
 *
 * Copyright (c) 2011 Wilcox Technologies, LLC <awilcox -at- wilcox-tech.com>
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

#ifndef __MOWGLI_THREAD_H__
#define __MOWGLI_THREAD_H__

#if defined(__sun) || defined(__sco)
# include <thread.h>
# include <synch.h>
# define MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES
# define MOWGLI_NATIVE_MUTEX_DECL(name) mutex_t (name)
#elif defined(HAVE_LINUX_FUTEX_H) && defined(MOWGLI_FEATURE_HAVE_ATOMIC_OPS) && defined(MOWGLI_FEATURE_WANT_EXPERIMENTAL)
# include <linux/futex.h>
# define MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES
# define MOWGLI_NATIVE_MUTEX_DECL(name) void * (name)
#elif defined(_WIN32)
# define MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES
# define MOWGLI_NATIVE_MUTEX_DECL(name) HANDLE (name)
#else
# include <pthread.h>
#endif

typedef struct {
#ifdef MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES
	MOWGLI_NATIVE_MUTEX_DECL(mutex);
#else
	pthread_mutex_t mutex;
#endif
} mowgli_mutex_t;

#ifdef MOWGLI_NATIVE_MUTEX_DECL
# undef MOWGLI_NATIVE_MUTEX_DECL
#endif

typedef struct {
	int (*mutex_create)(mowgli_mutex_t *mutex);
	int (*mutex_lock)(mowgli_mutex_t *mutex);
	int (*mutex_trylock)(mowgli_mutex_t *mutex);
	int (*mutex_unlock)(mowgli_mutex_t *mutex);
	int (*mutex_destroy)(mowgli_mutex_t *mutex);
} mowgli_mutex_ops_t;

int mowgli_mutex_create(mowgli_mutex_t *mutex);
int mowgli_mutex_lock(mowgli_mutex_t *mutex);
int mowgli_mutex_trylock(mowgli_mutex_t *mutex);
int mowgli_mutex_unlock(mowgli_mutex_t *mutex);
int mowgli_mutex_destroy(mowgli_mutex_t *mutex);

typedef enum {
	MOWGLI_THREAD_POLICY_DEFAULT,
	MOWGLI_THREAD_POLICY_DISABLED,
} mowgli_thread_policy_t;

void mowgli_mutex_set_policy(mowgli_thread_policy_t policy);

/* simple dispatch function to set the ops up for the various subsystems. */
static inline void mowgli_thread_set_policy(mowgli_thread_policy_t policy)
{
	mowgli_mutex_set_policy(policy);
}

#endif /* !__MOWGLI_THREAD_H__ */
