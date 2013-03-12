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

typedef struct
{
	mowgli_eventloop_helper_start_fn_t *start_fn;
	void *userdata;
	mowgli_descriptor_t fd;
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
	helper->type.type = MOWGLI_EVENTLOOP_TYPE_HELPER;
	helper->fd = req->fd;

#ifndef _WIN32

	for (i = 0; i < 1024; i++)
		if (i != req->fd)
			close(i);

	x = open("/dev/null", O_RDWR);

	for (i = 0; i < 2; i++)
		if (req->fd != i)
			dup2(x, i);

	if (x > 2)
		close(x);

#endif

	helper->eventloop = mowgli_eventloop_create();
	helper->pfd = mowgli_pollable_create(helper->eventloop, helper->fd, helper);
	helper->userdata = req->userdata;

	mowgli_pollable_set_nonblocking(helper->pfd, true);

	req->start_fn(helper, helper->userdata);
}

mowgli_eventloop_helper_proc_t *
mowgli_helper_create(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_start_fn_t *start_fn, const char *helpername, void *userdata)
{
	mowgli_eventloop_helper_proc_t *helper;
	mowgli_helper_create_req_t child;
	int io_fd[2];

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(start_fn != NULL, NULL);

	child.start_fn = start_fn;
	child.userdata = userdata;

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->type.type = MOWGLI_EVENTLOOP_TYPE_HELPER;
	helper->eventloop = eventloop;

	socketpair(AF_UNIX, SOCK_STREAM, 0, io_fd);

	/* set up helper/child fd mapping */
	helper->fd = io_fd[0];
	child.fd = io_fd[1];

	/* make pollables and make them non-blocking */
	helper->pfd = mowgli_pollable_create(eventloop, helper->fd, helper);
	mowgli_pollable_set_nonblocking(helper->pfd, true);

	/* spawn helper process using mowgli_process_clone() */
	helper->child = mowgli_process_clone((mowgli_process_start_fn_t) mowgli_helper_trampoline, helpername, &child);

	if (helper->child == NULL)
	{
		mowgli_pollable_destroy(eventloop, helper->pfd);

		close(io_fd[0]);
		close(io_fd[1]);

		mowgli_free(helper);
		return NULL;
	}

	close(child.fd);

	return helper;
}

mowgli_eventloop_helper_proc_t *
mowgli_helper_spawn(mowgli_eventloop_t *eventloop, const char *path, char *const argv[])
{
	mowgli_eventloop_helper_proc_t *helper;
	int io_fd[2];
	char buf[64];

	return_val_if_fail(eventloop != NULL, NULL);
	return_val_if_fail(path != NULL, NULL);

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->type.type = MOWGLI_EVENTLOOP_TYPE_HELPER;
	helper->eventloop = eventloop;

	socketpair(AF_UNIX, SOCK_STREAM, 0, io_fd);

	/* set up helper/child fd mapping */
	helper->fd = io_fd[0];

	/* make pollables and make them non-blocking */
	helper->pfd = mowgli_pollable_create(eventloop, helper->fd, helper);

	snprintf(buf, sizeof buf, "%d", io_fd[1]);
	setenv("IO_FD", buf, 1);

	/* Spawn helper process using mowgli_process_spawn(), helper will get
	 * IO_FD mapping from getenv().  Ugly hack, but it works...
	 *     --nenolod
	 */
	helper->child = mowgli_process_spawn(path, argv);

	if (helper->child == NULL)
	{
		mowgli_pollable_destroy(eventloop, helper->pfd);

		close(io_fd[0]);
		close(io_fd[1]);

		mowgli_free(helper);
		return NULL;
	}

	close(io_fd[1]);

	return helper;
}

/* Set up a helper connection to parent using getenv() */
mowgli_eventloop_helper_proc_t *
mowgli_helper_setup(mowgli_eventloop_t *eventloop)
{
	mowgli_eventloop_helper_proc_t *helper;
	const char *env_io_fd;

	env_io_fd = getenv("IO_FD");

	/* this shouldn't be a hard-fail because some idiot may run the helper from
	 * the cmdline.  allow the helper to error out gracefully if not spawned as
	 * a helper.
	 */
	if (env_io_fd == NULL)
		return NULL;

	helper = mowgli_alloc(sizeof(mowgli_eventloop_helper_proc_t));
	helper->type.type = MOWGLI_EVENTLOOP_TYPE_HELPER;
	helper->eventloop = eventloop;
	helper->fd = atoi(env_io_fd);
	helper->pfd = mowgli_pollable_create(helper->eventloop, helper->fd, helper);

	mowgli_pollable_set_nonblocking(helper->pfd, true);

	return helper;
}

static void
mowgli_helper_io_trampoline(mowgli_eventloop_t *eventloop, mowgli_eventloop_io_t *io, mowgli_eventloop_io_dir_t dir, void *userdata)
{
	mowgli_eventloop_helper_proc_t *helper = userdata;

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:

		if (helper->read_function != NULL)
		{
			helper->read_function(eventloop, helper, MOWGLI_EVENTLOOP_IO_READ, helper->userdata);
			return;
		}

	default:
		break;
	}
}

void
mowgli_helper_set_read_cb(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper, mowgli_eventloop_io_cb_t *read_fn)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(helper != NULL);

	if (read_fn == NULL)
		mowgli_pollable_setselect(eventloop, helper->pfd, MOWGLI_EVENTLOOP_IO_READ, NULL);

	helper->read_function = read_fn;
	mowgli_pollable_setselect(eventloop, helper->pfd, MOWGLI_EVENTLOOP_IO_READ, mowgli_helper_io_trampoline);
}

void
mowgli_helper_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_helper_proc_t *helper)
{
	return_if_fail(eventloop != NULL);
	return_if_fail(helper != NULL);

	mowgli_process_kill(helper->child);
	mowgli_pollable_destroy(eventloop, helper->pfd);
	close(helper->fd);

	mowgli_free(helper);
}
