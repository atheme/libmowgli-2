/*
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

typedef struct
{
	char *path;
	char **argv;
} mowgli_process_execv_req_t;

static void
mowgli_process_cloned_execv(mowgli_process_execv_req_t *execv_req)
{
#ifndef _WIN32
	return_if_fail(execv_req != NULL);
	return_if_fail(execv_req->path != NULL);
	return_if_fail(execv_req->argv != NULL);

	/* Do best to set proctitle if below hack don't work */
	mowgli_proctitle_set("%s", execv_req->argv[0]);
	execv(execv_req->path, execv_req->argv);

	mowgli_free(execv_req->argv);
	mowgli_free(execv_req->path);
	mowgli_free(execv_req);
#else
# warning implement me :(
#endif
}

mowgli_process_t *
mowgli_process_clone(mowgli_process_start_fn_t start_fn, const char *procname, void *userdata)
{
#ifndef _WIN32
	mowgli_process_t *out;

	return_val_if_fail(start_fn != NULL, NULL);

	out = mowgli_alloc(sizeof(mowgli_process_t));
	out->userdata = userdata;

	out->pid = fork();

	switch (out->pid)
	{
	default:
		break;

	case 0:

		/* Do our best to set this... */
		mowgli_proctitle_set("%s", procname);
		start_fn(out->userdata);
		_exit(255);

		return NULL;
		break;

	case -1:
		mowgli_free(out);
		return NULL;
		break;
	}

	return out;
#else
# warning implement me :(
	return NULL;
#endif
}

mowgli_process_t *
mowgli_process_spawn(const char *path, char *const argv[])
{
	size_t i;
	mowgli_process_execv_req_t *req;

	return_val_if_fail(path != NULL, NULL);
	return_val_if_fail(argv != NULL, NULL);

	req = mowgli_alloc(sizeof(mowgli_process_execv_req_t));
	req->path = mowgli_strdup(path);

	for (i = 0; argv[i] != NULL; i++)
		;

	req->argv = mowgli_alloc_array(sizeof(char *), i + 1);

	for (i = 0; argv[i] != NULL; i++)
		req->argv[i] = argv[i];

	return mowgli_process_clone((mowgli_process_start_fn_t) mowgli_process_cloned_execv, req->argv[0], req);
}

void
mowgli_process_kill(mowgli_process_t *process)
{
#ifndef _WIN32
	return_if_fail(process != NULL);

	kill(process->pid, SIGKILL);
#else
# warning implement me :(
#endif
}
