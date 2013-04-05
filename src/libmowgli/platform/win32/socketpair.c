/* socketpair.c
 * Copyright 2007, 2010 by Nathan C. Myers <ncm@cantrip.org>
 * This code is Free Software.  It may be copied freely, in original or
 * modified form, subject only to the restrictions that (1) the author is
 * relieved from all responsibilities for any use for any purpose, and (2)
 * this copyright notice must be retained, unchanged, in its entirety.  If
 * for any reason the author might be held responsible for any consequences
 * of copying or use, license is withheld.
 */

/* Changes:
 * 2010-03-31:
 *   set addr to 127.0.0.1 because win32 getsockname does not always set it.
 * 2010-02-25:
 *   set SO_REUSEADDR option to avoid leaking some windows resource.
 *   Windows System Error 10049, "Event ID 4226 TCP/IP has reached
 *   the security limit imposed on the number of concurrent TCP connect
 *   attempts."  Bleah.
 * 2007-04-25:
 *   preserve value of WSAGetLastError() on all error returns.
 * 2007-04-22:  (Thanks to Matthew Gregan <kinetik@flim.org>)
 *   s/EINVAL/WSAEINVAL/ fix trivial compile failure
 *   s/socket/WSASocket/ enable creation of sockets suitable as stdin/stdout
 *     of a child process.
 *   add argument make_overlapped
 */

#include "mowgli.h"

#ifdef _WIN32

/* dumb_socketpair:
 *   If make_overlapped is nonzero, both sockets created will be usable for
 *   "overlapped" operations via WSASend etc.  If make_overlapped is zero,
 *   socks[0] (only) will be usable with regular ReadFile etc., and thus
 *   suitable for use as stdin or stdout of a child process.  Note that the
 *   sockets must be closed with closesocket() regardless.
 */

int
socketpair(int domain, int type, int protocol, int socks[2])
{
	union
	{
		struct sockaddr_in inaddr;

		struct sockaddr addr;
	} a;

	SOCKET listener;
	int e;
	socklen_t addrlen = sizeof(a.inaddr);
	int make_overlapped = 0;
	DWORD flags = (make_overlapped ? WSA_FLAG_OVERLAPPED : 0);
	int reuse = 1;

	if (socks == NULL)
	{
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}

	listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listener == INVALID_SOCKET)
		return SOCKET_ERROR;

	memset(&a, 0, sizeof(a));
	a.inaddr.sin_family = AF_INET;
	a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	a.inaddr.sin_port = 0;

	socks[0] = socks[1] = INVALID_SOCKET;

	do
	{
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
			       (char *) &reuse, (socklen_t) sizeof(reuse)) == -1)
			break;

		if (bind(listener, &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
			break;

		memset(&a, 0, sizeof(a));

		if (getsockname(listener, &a.addr, &addrlen) == SOCKET_ERROR)
			break;

		// win32 getsockname may only set the port number, p=0.0005.
		// ( http://msdn.microsoft.com/library/ms738543.aspx ):
		a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		a.inaddr.sin_family = AF_INET;

		if (listen(listener, 1) == SOCKET_ERROR)
			break;

		socks[0] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, flags);

		if (socks[0] == (int) INVALID_SOCKET)
			break;

		if (connect(socks[0], &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
			break;

		socks[1] = accept(listener, NULL, NULL);

		if (socks[1] == (int) INVALID_SOCKET)
			break;

		closesocket(listener);
		return 0;
	} while (0);

	e = WSAGetLastError();
	closesocket(listener);
	closesocket(socks[0]);
	closesocket(socks[1]);
	WSASetLastError(e);
	return SOCKET_ERROR;
}

#endif
