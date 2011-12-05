/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_thread.c: Cross-platform threading helper routines.
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

#include "mowgli.h"

#ifndef _WIN32

/*************
 * This implements native Sun/UnixWare threads.  Some other SVR4-based
 * environments attempted to make work-alikes, but those aren't guaranteed
 * to be supported.  This should work on SunOS 5.2 and UnixWare 7, and
 * anything later.
 *************/
#if defined(__sun) || defined(__sco)

int mowgli_mutex_create(mowgli_mutex_t *mutex)
{
	return mutex_init(&mutex->mutex, USYNC_THREAD, NULL);
}

int mowgli_mutex_lock(mowgli_mutex_t *mutex)
{
	return mutex_lock(&mutex->mutex);
}

int mowgli_mutex_trylock(mowgli_mutex_t *mutex)
{
	return mutex_trylock(&mutex->mutex);
}

int mowgli_mutex_unlock(mowgli_mutex_t *mutex)
{
	return mutex_unlock(&mutex->mutex);
}

int mowgli_mutex_destroy(mowgli_mutex_t *mutex)
{
	return mutex_destroy(&mutex->mutex);
}

/*************
 * This "default" implementation uses pthreads.  Care has been taken to
 * ensure it runs on POSIX 1003.4a (draft 4, aka DECthreads, aka what OSF/1,
 * Tru64, Ultrix, CMU Mach, and HP-UX use) as well as POSIX 1003.1c-1995.
 * As long as you don't try playing with the pthread_attr module or the
 * scheduler routines (which are non-standard and broken anyway, IMO) then
 * it should be relatively easy to maintian d4 compatibility without
 * sacrificing any functionality.
 *************/
#else

int mowgli_mutex_create(mowgli_mutex_t *mutex)
{
	return pthread_mutex_init(&mutex->mutex, NULL);
}

int mowgli_mutex_lock(mowgli_mutex_t *mutex)
{
	return pthread_mutex_lock(&mutex->mutex);
}

int mowgli_mutex_trylock(mowgli_mutex_t *mutex)
{
	return pthread_mutex_trylock(&mutex->mutex);
}

int mowgli_mutex_unlock(mowgli_mutex_t *mutex)
{
	return pthread_mutex_unlock(&mutex->mutex);
}

int mowgli_mutex_destroy(mowgli_mutex_t *mutex)
{
	return pthread_mutex_destroy(&mutex->mutex);
}

#endif

#endif
