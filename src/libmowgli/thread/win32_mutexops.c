/*
 * libmowgli: A collection of useful routines for programming.
 * win32_mutexops.c: Windows mutex operations
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

/*************
* This Windows implementation is guaranteed to work on Windows 95,
* Windows NT 4, and anything later.
*************/
#if defined(_WIN32)

static int
mowgli_win32_mutex_create(mowgli_mutex_t *mutex)
{
	mutex->mutex = CreateMutex(NULL, FALSE, NULL);

	if (mutex->mutex == NULL)
		return GetLastError();

	return 0;
}

static int
mowgli_win32_mutex_lock(mowgli_mutex_t *mutex)
{
	return WaitForSingleObject(mutex->mutex, INFINITE);
}

static int
mowgli_win32_mutex_trylock(mowgli_mutex_t *mutex)
{
	return WaitForSingleObject(mutex->mutex, 0);
}

static int
mowgli_win32_mutex_unlock(mowgli_mutex_t *mutex)
{
	if (ReleaseMutex(mutex->mutex) != 0)
		return 0;

	return GetLastError();
}

static int
mowgli_win32_mutex_destroy(mowgli_mutex_t *mutex)
{
	CloseHandle(mutex->mutex);
	return 0;
}

const mowgli_mutex_ops_t _mowgli_win32_mutex_ops =
{
	.mutex_create = mowgli_win32_mutex_create,
	.mutex_lock = mowgli_win32_mutex_lock,
	.mutex_trylock = mowgli_win32_mutex_trylock,
	.mutex_unlock = mowgli_win32_mutex_unlock,
	.mutex_destroy = mowgli_win32_mutex_destroy
};

#endif
