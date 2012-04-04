/*
 * libmowgli: A collection of useful routines for programming.
 * future.c: Result of an asynchronous computation
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

/* Please note: You /must/ check the return values from these operations.
 * Future is meant to be used on concurrent operations, failure to use
 * this properly will, at the very least, result in leaked memory, at
 * the very worst, a crashing program.
 */

#include "mowgli.h"

typedef struct _mowgli_future {
	mowgli_atomic(mowgli_future_state_t) state;
	mowgli_atomic(void *) result;
} mowgli_future_t;

mowgli_future_t *mowgli_future_create() {
	mowgli_future_t *future = mowgli_alloc(sizeof(mowgli_future_t));

	if(mowgli_future_init(future) == 0) {
		return future;
	} else {
		mowgli_free(future);
		return NULL;
	}
}

int mowgli_future_init(mowgli_future_t *future) {
	return_val_if_fail(future != NULL, -1);

	mowgli_atomic_store_int(&future->state, MOWGLI_FUTURE_STATE_WAITING);
	mowgli_atomic_store_pointer(&future->result, NULL);

	return 0;
}

/* Given a valid future object, finish will either return FINISHED if it
 * successfully finished, CANCELED if it was canceled before you called
 * finish (and you must clean up your result yourself), or
 * CONSISTENCY_FAILURE if you have accidently tried to finish a future more
 * than once or something else has gone horribly wrong.
 */

mowgli_future_state_t mowgli_future_finish(mowgli_future_t *future, void *result) {
	return_val_if_fail(future != NULL, MOWGLI_FUTURE_STATE_ERROR);

	mowgli_future_state_t oldstate = mowgli_atomic_compare_exchange_int(&future->state,
			MOWGLI_FUTURE_STATE_WAITING, MOWGLI_FUTURE_STATE_RUNNING);

	if(oldstate == MOWGLI_FUTURE_STATE_WAITING) {
			void *oldresult = mowgli_atomic_compare_exchange_pointer(&future->result, NULL, result);

			if(oldresult == NULL) {
				oldstate = mowgli_atomic_compare_exchange_int(&future->state, MOWGLI_FUTURE_STATE_RUNNING,
						MOWGLI_FUTURE_STATE_FINISHED);

				if(oldstate == MOWGLI_FUTURE_STATE_RUNNING)
					return MOWGLI_FUTURE_STATE_FINISHED;
				else
					return MOWGLI_FUTURE_STATE_CONSISTENCY_FAILURE;
			}
			else
				return MOWGLI_FUTURE_STATE_CONSISTENCY_FAILURE;
	} else if(oldstate == MOWGLI_FUTURE_STATE_CANCELED) {
		return MOWGLI_FUTURE_STATE_CANCELED;
	} else if(oldstate == MOWGLI_FUTURE_STATE_FINISHED || oldstate == MOWGLI_FUTURE_STATE_RUNNING) {
		return MOWGLI_FUTURE_STATE_CONSISTENCY_FAILURE;
	} else {
		return oldstate;
	}
}

/* Given a valid future object, cancel will either return CANCELED and it was
 * successfully canceled before it could finish, FINISHED if it already
 * finished (and you now must clean up the result), or ERROR or
 * CONSISTENCY_FAILURE if something went wrong before you tried to cancel.
 */
mowgli_future_state_t mowgli_future_cancel(mowgli_future_t *future) {
	return_val_if_fail(future != NULL, MOWGLI_FUTURE_STATE_ERROR);

	mowgli_future_state_t state = mowgli_atomic_compare_exchange_int(&future->state,
			MOWGLI_FUTURE_STATE_WAITING, MOWGLI_FUTURE_STATE_CANCELED);

	if(state != MOWGLI_FUTURE_STATE_WAITING)
		return state;
	else
		return mowgli_atomic_load_int(&future->state);
}

/* Given a valid future object, return the current state */
mowgli_future_state_t mowgli_future_state(mowgli_future_t *future) {
	return_val_if_fail(future != NULL, MOWGLI_FUTURE_STATE_ERROR);

	return mowgli_atomic_load_int(&future->state);
}

/* Given a valid future object, result will return the result WITHOUT
 * checking the state. If this future has a state of CONSISTENCY_FAILURE,
 * ERRORED, CANCELED, or WAITING, it will still return the result.
 */
void *mowgli_future_result(mowgli_future_t *future) {
	return_val_if_fail(future != NULL, NULL);

	return mowgli_atomic_load_pointer(&future->result);
}
