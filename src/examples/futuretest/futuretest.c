/*
 * libmowgli: A collection of useful routines for programming.
 * futuretest: Combustable lemons
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

#include <string.h>

#include <mowgli.h>

int main(int argc, char *argv[]) {
	char *text = "hello world";

	printf("create future: ");
	mowgli_future_t *future = mowgli_future_create();
	if(future != NULL)
		printf("correctly created future\n");
	else
		printf("error: abandon all hope\n");

	printf("get state manually of waiting future: ");
	if(mowgli_future_state(future) == MOWGLI_FUTURE_STATE_WAITING)
		printf("correctly waiting\n");
	else
		printf("error: %i\n", mowgli_future_state(future));

	printf("finish future: ");
	if(mowgli_future_finish(future, text) == MOWGLI_FUTURE_STATE_FINISHED)
		printf("correctly finished\n");
	else
		printf("error: %i\n", mowgli_future_state(future));

	printf("get result of finished future: ");
	if(mowgli_future_result(future) == text)
		printf("correct: %s\n", text);
	else
		printf("error: %s\n", (char *)mowgli_future_result(future));

	printf("get state of finished future: ");
	if(mowgli_future_state(future) == MOWGLI_FUTURE_STATE_FINISHED)
		printf("correctly finished\n");
	else
		printf("error: %i\n", mowgli_future_state(future));

	printf("reinit then cancel: ");
	if(mowgli_future_init(future) == 0) {
		if(mowgli_future_cancel(future) == MOWGLI_FUTURE_STATE_CANCELED)
			printf("correctly canceled\n");
		else
			printf("error: failed to cancel: %i\n", mowgli_future_state(future));

		printf("try to finish on canceled future: ");
		if(mowgli_future_finish(future, text) == MOWGLI_FUTURE_STATE_CANCELED)
			printf("correctly caught cancel\n");
		else
			printf("error: failed to cancel: %s\n", text);
	} else {
		printf("error: failed to reinit\n");
	}

	printf("reinit then finish twice: ");
	if(mowgli_future_init(future) == 0) {
		mowgli_future_finish(future, text);

		if(mowgli_future_finish(future, text) == MOWGLI_FUTURE_STATE_CONSISTENCY_FAILURE)
			printf("correctly raised consistency failure\n");
		else
			printf("error: finished twice: %s\n", text);
	} else {
		printf("error: failed to reinit\n");
	}
}
