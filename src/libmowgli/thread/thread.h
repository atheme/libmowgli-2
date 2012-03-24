/*
 * libmowgli: A collection of useful routines for programming.
 * thread.h: Cross-platform threading helper routines.
 *
 * Copyright (c) 2011 Wilcox Technologies, LLC <awilcox -at- wilcox-tech.com>
 * Copyright (c) 2011, 2012 William Pitcock <nenolod@dereferenced.org>
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
# define MOWGLI_FEATURE_HAVE_NATIVE_THREADS
# define MOWGLI_NATIVE_MUTEX_DECL(name) 	mutex_t (name)
# define MOWGLI_NATIVE_THREAD_DECL(name)	thread_t (name)
#elif defined(_WIN32)
# define MOWGLI_FEATURE_HAVE_NATIVE_MUTEXES
# define MOWGLI_FEATURE_HAVE_NATIVE_THREADS
# define MOWGLI_NATIVE_MUTEX_DECL(name) 	HANDLE (name)
# define MOWGLI_NATIVE_THREAD_DECL(name) 	HANDLE (name)
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

typedef struct {
#ifdef MOWGLI_FEATURE_HAVE_NATIVE_THREADS
	MOWGLI_NATIVE_THREAD_DECL(thread);
#else
	pthread_t thread;
#endif
} mowgli_thread_t;

#ifdef MOWGLI_NATIVE_THREAD_DECL
# undef MOWGLI_NATIVE_THREAD_DECL
#endif

typedef void *(*mowgli_thread_start_fn_t)(mowgli_thread_t *thread, void *userdata);

/*
 * Note: we should keep our thread interface light and minimal for best possible
 * portability.  Creating, ending, killing and cleanup functions are presently implemented,
 * and cover approximately 99.999% of uses of thread APIs.  --nenolod
 */
typedef struct {
	int (*thread_create)(mowgli_thread_t *thread, mowgli_thread_start_fn_t start_fn, void *userdata);
	void (*thread_exit)(mowgli_thread_t *thread);
	void *(*thread_join)(mowgli_thread_t *thread);
	void (*thread_kill)(mowgli_thread_t *thread);
	void (*thread_destroy)(mowgli_thread_t *thread);
} mowgli_thread_ops_t;

int mowgli_thread_create(mowgli_thread_t *thread, mowgli_thread_start_fn_t start_fn, void *userdata);
void mowgli_thread_exit(mowgli_thread_t *thread);
void *mowgli_thread_join(mowgli_thread_t *thread);
void mowgli_thread_kill(mowgli_thread_t *thread);
void mowgli_thread_destroy(mowgli_thread_t *thread);

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
