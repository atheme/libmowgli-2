/*
 * Copyright (c) 2012 William Pitcock <nenolod@dereferenced.org>
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

#define timersub(tvp, uvp, vvp)	\
	do \
	{ \
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
		if ((vvp)->tv_usec < 0)	\
		{ \
			(vvp)->tv_sec--; \
			(vvp)->tv_usec += 1000000; \
		} \
	} while (0)

#include <mowgli.h>

mowgli_allocation_policy_t *memslice;
mowgli_allocation_policy_t *sysmalloc;

int
main(int argc, char *argv[])
{
	size_t i;

	size_t objects;
	size_t *obj_sizes;
	void **ptrs;
	struct timeval ts, te;

	mowgli_thread_set_policy(MOWGLI_THREAD_POLICY_DISABLED);

	objects = 128000;
	ptrs = mowgli_alloc_array(sizeof(void *), objects);
	obj_sizes = mowgli_alloc_array(sizeof(size_t), objects);

	memslice = mowgli_allocation_policy_lookup("memslice");
	sysmalloc = mowgli_allocation_policy_lookup("malloc");

	if (sysmalloc == NULL)
	{
		printf("Couldn't find a sysmalloc component which implements contract 'mowgli.core.allocation_policy' :(\n");
		return EXIT_FAILURE;
	}

	if (memslice == NULL)
	{
		printf("Couldn't find a memslice component which implements contract 'mowgli.core.allocation_policy' :(\n");
		return EXIT_FAILURE;
	}

	printf("Going to allocate %zu objects of random sizes < 256\n", objects);

	printf("Assigning sizes...\n");

	for (i = 0; i < objects; i++)
		obj_sizes[i] = rand() % 256;

	printf("Done!  Lets benchmark.\n");

	/* allocate using sysmalloc */
	gettimeofday(&ts, NULL);

	for (i = 0; i < objects; i++)
		ptrs[i] = mowgli_alloc_using_policy(sysmalloc, obj_sizes[i]);

	gettimeofday(&te, NULL);
	timersub(&te, &ts, &ts);

	printf("sysmalloc alloc time: %ld usec\n",
	       ts.tv_sec * 1000000L + ts.tv_usec);

	gettimeofday(&ts, NULL);

	for (i = 0; i < objects; i++)
		mowgli_free(ptrs[i]);

	gettimeofday(&te, NULL);
	timersub(&te, &ts, &ts);

	printf("sysmalloc free time: %ld usec\n",
	       ts.tv_sec * 1000000L + ts.tv_usec);

	/* allocate using memslice */
	gettimeofday(&ts, NULL);

	for (i = 0; i < objects; i++)
		ptrs[i] = mowgli_alloc_using_policy(memslice, obj_sizes[i]);

	gettimeofday(&te, NULL);
	timersub(&te, &ts, &ts);

	printf("memslice alloc time: %ld usec\n",
	       ts.tv_sec * 1000000L + ts.tv_usec);

	gettimeofday(&ts, NULL);

	for (i = 0; i < objects; i++)
		mowgli_free(ptrs[i]);

	gettimeofday(&te, NULL);
	timersub(&te, &ts, &ts);

	printf("memslice free time: %ld usec\n",
	       ts.tv_sec * 1000000L + ts.tv_usec);

	return EXIT_SUCCESS;
}
