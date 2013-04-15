/*
 * libmowgli: A collection of useful routines for programming.
 * cacheline.c: Platform specific routines to get L1 cache line size
 *
 * Copyright (c) 2013 Patrick McFarland <pmcfarland@adterrasperaspera.com>
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

#ifdef MOWGLI_OS_OSX
# include <sys/sysctl.h>
#endif

size_t cacheline_size;

void
mowgli_cacheline_bootstrap(void)
{
#ifdef MOWGLI_OS_LINUX
	cacheline_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
#elif defined(MOWGLI_OS_OSX)
	size_t size = sizeof(size_t);
	sysctlbyname("hw.cachelinesize", &cacheline_size, &size, 0, 0);
#elif defined(MOWGLI_OS_WIN)
	DWORD buf_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buf = 0;

	GetLogicalProcessorInformation(0, &buf_size);
	buf = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) mowgli_alloc(buf_size);
	GetLogicalProcessorInformation(&buf[0], &buf_size);

	for (i = 0; i != buf_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
		if ((buf[i].Relationship == RelationCache) && (buf[i].Cache.Level == 1))
		{
			cacheline_size = buf[i].Cache.LineSize;
			break;
		}

	mowgli_free(buf);
#else

	// This is often true
# ifdef MOWGLI_CPU_BITS_32
	cacheline_size = 32;
# else
	cacheline_size = 64;
# endif
#endif
}

size_t
mowgli_cacheline_size(void)
{
	return cacheline_size;
}
