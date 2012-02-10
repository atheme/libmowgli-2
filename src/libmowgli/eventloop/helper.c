/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>.
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

#include "mowgli.h"

typedef struct {
	mowgli_eventloop_helper_start_fn_t *start_fn;
	void *userdata;
	mowgli_descriptor_t in_fd, out_fd;
} mowgli_helper_create_req_t;

static void
mowgli_helper_trampoline(mowgli_helper_create_req_t *req)
{
	mowgli_eventloop_helper_proc_t *helper;
#ifndef _WIN32
	int i, x;
#endif

	return_if_fail(req != NULL);
	return_if_fail(req->start_fn != NULL);

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->in_fd = req->in_fd;
	helper->out_fd = req->out_fd;

#ifndef _WIN32
	for (i = 0; i < 1024; i++)
	{
		if (i != req->in_fd && i != req->out_fd)
			close(i);
	}

	x = open("/dev/null", O_RDWR);

	for (i = 0; i < 2; i++)
	{
		if (req->in_fd != i && req->out_fd != i)
			dup2(x, i);
	}

	if (x > 2)
		close(x);
#endif

	helper->eventloop = mowgli_eventloop_create();
	helper->in_pfd = mowgli_pollable_create(helper->eventloop, helper->in_fd, helper);
	helper->out_pfd = mowgli_pollable_create(helper->eventloop, helper->out_fd, helper);
	helper->userdata = req->userdata;

	mowgli_pollable_set_nonblocking(helper->in_pfd, true);
	mowgli_pollable_set_nonblocking(helper->out_pfd, true);

	req->start_fn(helper, helper->userdata);
}

mowgli_eventloop_helper_proc_t *
mowgli_helper_create(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_start_fn_t *start_fn, void *userdata)
{
	mowgli_eventloop_helper_proc_t *helper;
	mowgli_helper_create_req_t child;
	int in_fd[2], out_fd[2];

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(start_fn != NULL, NULL);

	child.start_fn = start_fn;
	child.userdata = userdata;

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->eventloop = eventloop;

	pipe(in_fd);
	pipe(out_fd);

	/* set up helper/child fd mapping */
	helper->in_fd = in_fd[0];
	helper->out_fd = out_fd[1];
	child.in_fd = in_fd[1];
	child.out_fd = out_fd[0];

	/* make pollables and make them non-blocking */
	helper->in_pfd = mowgli_pollable_create(eventloop, helper->in_fd, helper);
	helper->out_pfd = mowgli_pollable_create(eventloop, helper->out_fd, helper);
	mowgli_pollable_set_nonblocking(helper->in_pfd, true);
	mowgli_pollable_set_nonblocking(helper->out_pfd, true);

	/* spawn helper process using mowgli_process_clone() */
	helper->child = mowgli_process_clone((mowgli_process_start_fn_t) mowgli_helper_trampoline, &child);

	if (helper->child == NULL)
	{
		mowgli_pollable_destroy(eventloop, helper->in_pfd);
		mowgli_pollable_destroy(eventloop, helper->out_pfd);

		close(in_fd[0]);
		close(in_fd[1]);
		close(out_fd[0]);
		close(out_fd[1]);

		mowgli_free(helper);
		return NULL;
	}

	close(child.in_fd);
	close(child.out_fd);

	return helper;
}

mowgli_eventloop_helper_proc_t *
mowgli_helper_spawn(mowgli_eventloop_t *eventloop, const char *path, char *const argv[])
{
	mowgli_eventloop_helper_proc_t *helper;
	int in_fd[2], out_fd[2];
	char buf[64];

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(path != NULL, NULL);

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->eventloop = eventloop;

	pipe(in_fd);
	pipe(out_fd);

	/* set up helper/child fd mapping */
	helper->in_fd = in_fd[0];
	helper->out_fd = out_fd[1];

	/* make pollables and make them non-blocking */
	helper->in_pfd = mowgli_pollable_create(eventloop, helper->in_fd, helper);
	helper->out_pfd = mowgli_pollable_create(eventloop, helper->out_fd, helper);
	mowgli_pollable_set_nonblocking(helper->in_pfd, true);
	mowgli_pollable_set_nonblocking(helper->out_pfd, true);

	snprintf(buf, sizeof buf, "%d", in_fd[1]);
	setenv("IN_FD", buf, 1);

	snprintf(buf, sizeof buf, "%d", out_fd[0]);
	setenv("OUT_FD", buf, 1);

	/* Spawn helper process using mowgli_process_spawn(), helper will get
	 * IN_FD/OUT_FD mapping from getenv().  Ugly hack, but it works...
	 *     --nenolod
	 */
	helper->child = mowgli_process_spawn(path, argv);

	if (helper->child == NULL)
	{
		mowgli_pollable_destroy(eventloop, helper->in_pfd);
		mowgli_pollable_destroy(eventloop, helper->out_pfd);

		close(in_fd[0]);
		close(in_fd[1]);
		close(out_fd[0]);
		close(out_fd[1]);

		mowgli_free(helper);
		return NULL;
	}

	close(in_fd[1]);
	close(out_fd[0]);

	return helper;
}

/* Set up a helper connection to parent using getenv() */
mowgli_eventloop_helper_proc_t *
mowgli_helper_setup(mowgli_eventloop_t *eventloop)
{
	mowgli_eventloop_helper_proc_t *helper;
	const char *env_in_fd, *env_out_fd;

	env_in_fd = getenv("IN_FD");
	env_out_fd = getenv("IN_FD");

	/* this shouldn't be a hard-fail because some idiot may run the helper from
	 * the cmdline.  allow the helper to error out gracefully if not spawned as
	 * a helper.
	 */
	if (env_in_fd == NULL)
		return NULL;

	if (env_out_fd == NULL)
		return NULL;

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->eventloop = mowgli_eventloop_create();
	helper->in_fd = atoi(env_in_fd);
	helper->out_fd = atoi(env_out_fd);
	helper->in_pfd = mowgli_pollable_create(helper->eventloop, helper->in_fd, helper);
	helper->out_pfd = mowgli_pollable_create(helper->eventloop, helper->out_fd, helper);

	mowgli_pollable_set_nonblocking(helper->in_pfd, true);
	mowgli_pollable_set_nonblocking(helper->out_pfd, true);

	return helper;
}

static void
mowgli_helper_io_trampoline(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_pollable_dir_t dir, void *userdata)
{
	mowgli_eventloop_helper_proc_t *helper = userdata;

	switch (dir) {
	case MOWGLI_EVENTLOOP_POLL_READ:
		if (helper->read_function != NULL)
			return helper->read_function(eventloop, helper, helper->userdata);
	default:
		break;
	}
}

void
mowgli_helper_set_read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper, mowgli_eventloop_helper_cb_t *read_fn)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(helper != NULL);

	if (read_fn == NULL)
		mowgli_pollable_setselect(eventloop, helper->in_pfd, MOWGLI_EVENTLOOP_POLL_READ, NULL);

	helper->read_function = read_fn;
	mowgli_pollable_setselect(eventloop, helper->in_pfd, MOWGLI_EVENTLOOP_POLL_READ, mowgli_helper_io_trampoline);
}

void
mowgli_helper_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(helper != NULL);

	mowgli_process_kill(helper->child);
	mowgli_pollable_destroy(eventloop, helper->in_pfd);
	mowgli_pollable_destroy(eventloop, helper->out_pfd);
	close(helper->in_fd);
	close(helper->out_fd);

	mowgli_free(helper);
}
