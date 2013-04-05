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

#ifdef _WIN32

# define DEFAULT_SOCKETMAX (2048)

typedef struct
{
	WSAEVENT *pfd;
	unsigned short pfd_size;
	unsigned short last_slot;
	mowgli_eventloop_pollable_t **pollables;
} mowgli_winsock_eventloop_private_t;

static WSADATA wsock_env;

void
mowgli_winsock_bootstrap(void)
{
	int r;

	r = WSAStartup((short) 0x202, &wsock_env);

	if (r != 0)
	{
		printf("mowgli bootstrap failure (win32): %d\n", r);
		exit(EXIT_FAILURE);
	}

	if (!wsock_env.iMaxSockets)
		wsock_env.iMaxSockets = DEFAULT_SOCKETMAX;
	else
		wsock_env.iMaxSockets -= (wsock_env.iMaxSockets % MAXIMUM_WAIT_OBJECTS);
}

static void
mowgli_winsock_eventloop_pollsetup(mowgli_eventloop_t *eventloop)
{
	unsigned short i;
	mowgli_winsock_eventloop_private_t *priv;

	return_if_fail(wsock_env.iMaxSockets > 0);

	priv = mowgli_alloc(sizeof(mowgli_winsock_eventloop_private_t));
	eventloop->poller = priv;

	priv->pfd_size = wsock_env.iMaxSockets;
	priv->pfd = mowgli_alloc(sizeof(WSAEVENT) * priv->pfd_size);
	priv->pollables = mowgli_alloc(sizeof(mowgli_eventloop_pollable_t *) * priv->pfd_size);

	/* sanitize NT port handle values to a known-good default */
	for (i = 0; i < priv->pfd_size; i++)
	{
		priv->pfd[i] = INVALID_HANDLE_VALUE;
		priv->pollables[i] = NULL;
	}

	return;
}

static unsigned short
mowgli_winsock_eventloop_find_slot(mowgli_winsock_eventloop_private_t *priv)
{
	unsigned short i = 1;

	return_val_if_fail(priv != NULL, 0);

	if (priv->last_slot)
		i = priv->last_slot;

	for (; i < priv->pfd_size; i++)
		if (priv->pfd[i] == INVALID_HANDLE_VALUE)
		{
			priv->last_slot = i;
			return i;
		}

	/* miss, try from beginning. */

	for (i = 1; i < priv->pfd_size; i++)
		if (priv->pfd[i] == INVALID_HANDLE_VALUE)
		{
			priv->last_slot = i;
			return i;
		}

	/* if this happens, we're boned... */
	mowgli_log("out of handles for eventloop %p, aborting\n", priv);
	abort();

	return 0;
}

static void
mowgli_winsock_eventloop_pollshutdown(mowgli_eventloop_t *eventloop)
{
	unsigned short i;
	mowgli_winsock_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	for (i = 0; i < priv->pfd_size; i++)
	{
		WSACloseEvent(priv->pfd[i]);
		priv->pfd[i] = INVALID_HANDLE_VALUE;
	}

	mowgli_free(priv->pfd);
	mowgli_free(priv);

	return;
}

static void
mowgli_winsock_eventloop_destroy(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable)
{
	mowgli_winsock_eventloop_private_t *priv;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;

	if (pollable->slot)
	{
		WSAEventSelect(pollable->fd, priv->pfd[pollable->slot], 0);
		WSACloseEvent(priv->pfd[pollable->slot]);

		priv->pfd[pollable->slot] = INVALID_HANDLE_VALUE;
		priv->pollables[pollable->slot] = NULL;
	}

	pollable->slot = 0;
	pollable->events = 0;
}

static void
mowgli_winsock_eventloop_setselect(mowgli_eventloop_t *eventloop, mowgli_eventloop_pollable_t *pollable, mowgli_eventloop_io_dir_t dir, mowgli_eventloop_io_cb_t *event_function)
{
	mowgli_winsock_eventloop_private_t *priv;
	unsigned int old_flags;

	return_if_fail(eventloop != NULL);
	return_if_fail(pollable != NULL);

	priv = eventloop->poller;
	old_flags = pollable->events;

# ifdef DEBUG
	mowgli_log("setselect %p fd %d func %p", pollable, pollable->fd, event_function);
# endif

	switch (dir)
	{
	case MOWGLI_EVENTLOOP_IO_READ:
		pollable->read_function = event_function;
		pollable->events |= (FD_READ | FD_CLOSE | FD_ACCEPT | FD_OOB);
		break;
	case MOWGLI_EVENTLOOP_IO_WRITE:
		pollable->write_function = event_function;
		pollable->events |= (FD_WRITE | FD_CONNECT | FD_CLOSE);
		break;
	default:
		mowgli_log("unhandled pollable direction %d", dir);
		break;
	}

# ifdef DEBUG
	mowgli_log("%p -> read %p : write %p", pollable, pollable->read_function, pollable->write_function);
# endif

	if (pollable->read_function == NULL)
		pollable->events &= ~(FD_READ | FD_CLOSE | FD_ACCEPT | FD_OOB);

	if (pollable->write_function == NULL)
		pollable->events &= ~(FD_WRITE | FD_CONNECT | FD_CLOSE);

	if ((old_flags == 0) && (pollable->events == 0))
	{
		return;
	}
	else if (pollable->events <= 0)
	{
		mowgli_winsock_eventloop_destroy(eventloop, pollable);
		return;
	}

	/* set up the HANDLE if we have not already */
	if (!pollable->slot)
	{
		pollable->slot = mowgli_winsock_eventloop_find_slot(priv);

		priv->pfd[pollable->slot] = WSACreateEvent();
		priv->pollables[pollable->slot] = pollable;
	}

	if (WSAEventSelect(pollable->fd, priv->pfd[pollable->slot], pollable->events) != 0)
	{
		if (mowgli_eventloop_ignore_errno(WSAGetLastError()))
			return;

		mowgli_log("mowgli_winsock_eventloop_setselect(): WSAEventSelect failed: %d", WSAGetLastError());
	}

	return;
}

static void
mowgli_winsock_eventloop_select(mowgli_eventloop_t *eventloop, int delay)
{
	mowgli_winsock_eventloop_private_t *priv;
	int i, j;
	DWORD result;

	return_if_fail(eventloop != NULL);

	priv = eventloop->poller;

	return_if_fail(priv->pfd_size % MAXIMUM_WAIT_OBJECTS == 0);

	for (i = 0; i < priv->pfd_size; i += MAXIMUM_WAIT_OBJECTS)
	{
		result = WaitForMultipleObjects(MAXIMUM_WAIT_OBJECTS, priv->pfd + i, FALSE, delay);

		if (result == WAIT_FAILED)
		{
			if (mowgli_eventloop_ignore_errno(WSAGetLastError()))
				return;

			mowgli_log("mowgli_winsock_eventloop_select(): WaitForMultipleObjects failed: %d", WSAGetLastError());
			return;
		}

		for (j = (result - WAIT_OBJECT_0); j < MAXIMUM_WAIT_OBJECTS; j++)
		{
			mowgli_eventloop_pollable_t *pollable = priv->pollables[i + j];
			WSANETWORKEVENTS events;

			WSAEnumNetworkEvents(pollable->fd, priv->pfd[pollable->slot], &events);

			if (events.lNetworkEvents & (FD_READ | FD_CLOSE | FD_ACCEPT | FD_OOB))
				mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_READ);

			if (events.lNetworkEvents & (FD_WRITE | FD_CONNECT | FD_CLOSE))
				mowgli_pollable_trigger(eventloop, pollable, MOWGLI_EVENTLOOP_IO_WRITE);
		}
	}

	mowgli_eventloop_synchronize(eventloop);
}

mowgli_eventloop_ops_t _mowgli_winsock_pollops =
{
	.timeout_once = mowgli_simple_eventloop_timeout_once,
	.run_once = mowgli_simple_eventloop_run_once,
	.pollsetup = mowgli_winsock_eventloop_pollsetup,
	.pollshutdown = mowgli_winsock_eventloop_pollshutdown,
	.setselect = mowgli_winsock_eventloop_setselect,
	.select = mowgli_winsock_eventloop_select,
	.destroy = mowgli_winsock_eventloop_destroy,
};

#endif
