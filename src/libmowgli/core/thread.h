/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_thread.h: Cross-platform threading helper routines.
 *
 * Copyright (c) 2011 Wilcox Technologies, LLC <awilcox -at- wilcox-tech.com>
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

#if defined(_WIN32)				/* Windows threading */

	typedef HANDLE mowgli_mutex_t;

#elif defined(__sun)				/* Solaris/UnixWare threading */

#	include <thread.h>
#	include <synch.h>
	typedef mutex_t mowgli_mutex_t;

#else						/* pthreads */

#	include <pthread.h>
	typedef pthread_mutex_t mowgli_mutex_t;

#endif


int mowgli_mutex_create(mowgli_mutex_t *mutex);
int mowgli_mutex_lock(mowgli_mutex_t *mutex);
int mowgli_mutex_trylock(mowgli_mutex_t *mutex);
int mowgli_mutex_unlock(mowgli_mutex_t *mutex);
int mowgli_mutex_destroy(mowgli_mutex_t *mutex);


#endif /* !__MOWGLI_THREAD_H__ */
