/*
 * libmowgli: A collection of useful routines for programming.
 * win32_stdinc.h: Support functions and values for Win32 platform.
 *
 * Copyright (c) 2009 SystemInPlace, Inc.
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

#ifndef __LIBMOWGLI_SRC_LIBMOWGLI_WIN32_SUPPORT_H__GUARD
#define __LIBMOWGLI_SRC_LIBMOWGLI_WIN32_SUPPORT_H__GUARD

#ifdef _WIN32

# include <winsock2.h>
# include <ws2tcpip.h>
# include <time.h>

# define strcasecmp _stricmp
# define strdup _strdup
# define usleep(_usecs) Sleep((_usecs) / 1000L)
# ifdef _MSC_VER
#  define snprintf _snprintf
# endif

extern int setenv(const char *name, const char *value, int overwrite);

extern int pipe(int pipefd[2]);
extern int socketpair(int domain, int type, int protocol, int pipefd[2]);
extern int fork(void);
extern int inet_pton(int af, const char *src, void *dst);
extern const char *inet_ntop(int af, const void *addr, char *host, size_t hostlen);

/* MSYS autoconf is fucko. */
# ifndef HAVE_WINSOCK2_H
#  define HAVE_WINSOCK2_H
# endif

# define HAVE_SELECT

#endif

#endif
