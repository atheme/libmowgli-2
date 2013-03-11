/*
 * libmowgli: A collection of useful routines for programming.
 * null_mutexops.c: null mutex operations
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

static int
mowgli_null_mutex_create(mowgli_mutex_t *mutex)
{
	return 0;
}

static int
mowgli_null_mutex_lock(mowgli_mutex_t *mutex)
{
	return 0;
}

static int
mowgli_null_mutex_trylock(mowgli_mutex_t *mutex)
{
	return 0;
}

static int
mowgli_null_mutex_unlock(mowgli_mutex_t *mutex)
{
	return 0;
}

static int
mowgli_null_mutex_destroy(mowgli_mutex_t *mutex)
{
	return 0;
}

const mowgli_mutex_ops_t _mowgli_null_mutex_ops =
{
	.mutex_create = mowgli_null_mutex_create,
	.mutex_lock = mowgli_null_mutex_lock,
	.mutex_trylock = mowgli_null_mutex_trylock,
	.mutex_unlock = mowgli_null_mutex_unlock,
	.mutex_destroy = mowgli_null_mutex_destroy
};
