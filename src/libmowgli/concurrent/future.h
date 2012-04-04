/*
 * libmowgli: A collection of useful routines for programming.
 * future.h: Result of an asynchronous computation
 *
 * Copyright (c) 2012 Patrick McFarland <pmcfarland@adterrasperaspera.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
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

#ifndef __MOWGLI_FUTURE_H__
#define __MOWGLI_FUTURE_H__

typedef enum {
	MOWGLI_FUTURE_STATE_WAITING,
	MOWGLI_FUTURE_STATE_RUNNING,
	MOWGLI_FUTURE_STATE_FINISHED,
	MOWGLI_FUTURE_STATE_CANCELED,
	MOWGLI_FUTURE_STATE_ERROR = -1,
	MOWGLI_FUTURE_STATE_CONSISTENCY_FAILURE = -2
} mowgli_future_state_t;

typedef struct _mowgli_future mowgli_future_t;

extern mowgli_future_t *mowgli_future_create();
extern mowgli_future_t *mowgli_future_destroy(mowgli_future_t *future);
extern int mowgli_future_init(mowgli_future_t *future);
extern mowgli_future_state_t mowgli_future_finish(mowgli_future_t *future, void *result);
extern mowgli_future_state_t mowgli_future_cancel(mowgli_future_t *future);
extern mowgli_future_state_t mowgli_future_state(mowgli_future_t *future);
extern void *mowgli_future_result(mowgli_future_t *future);

#endif
