/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>.
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

#ifdef _WIN32

int
inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;

	int size = sizeof(struct sockaddr_storage);
	char src_copy[INET6_ADDRSTRLEN + 1];

	mowgli_strlcpy(src_copy, src, sizeof src_copy);

	if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *) &ss, &size) != SOCKET_ERROR)
		switch (af)
		{
		case AF_INET:
			*(struct in_addr *) dst = ((struct sockaddr_in *) &ss)->sin_addr;
			return 1;

		case AF_INET6:
			*(struct in6_addr *) dst = ((struct sockaddr_in6 *) &ss)->sin6_addr;
			return 1;

		default:
			return 0;
		}

	return -1;
}

const char *
inet_ntop(int af, const void *addr, char *host, size_t hostlen)
{
	struct sockaddr_storage ss;

	int size = sizeof(struct sockaddr_storage);

	ss.ss_family = af;

	switch (af)
	{
	case AF_INET:
		memcpy(&(((struct sockaddr_in *) &ss)->sin_addr), (struct in_addr *) addr, sizeof(struct in_addr));
		break;

	case AF_INET6:
		memcpy(&(((struct sockaddr_in6 *) &ss)->sin6_addr), (struct in6_addr *) addr, sizeof(struct in6_addr));
		break;

	default:
		return NULL;
	}

	if (WSAAddressToString((struct sockaddr *) &ss, size, 0, host, (LPDWORD) &hostlen) != SOCKET_ERROR)
		return host;

	return NULL;
}

#endif
