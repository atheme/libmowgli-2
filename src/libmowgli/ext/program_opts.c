/*
 * libmowgli: A collection of useful routines for programming.
 * program_opts.h: Replacement for GNU getopt().
 *
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

#include "mowgli.h"

#include "ext/getopt_long.h"

void
mowgli_program_opts_consumer_str(const char *arg, void *userdata)
{
	return_if_fail(arg != NULL);
	return_if_fail(userdata != NULL);

	*(char **) userdata = mowgli_strdup(arg);
}

void
mowgli_program_opts_consumer_int(const char *arg, void *userdata)
{
	return_if_fail(arg != NULL);
	return_if_fail(userdata != NULL);

	*(int *) userdata = atoi(arg);
}

void
mowgli_program_opts_consumer_bool(const char *arg, void *userdata)
{
	return_if_fail(arg != NULL);
	return_if_fail(userdata != NULL);

	*(bool *) userdata = true;
}

static inline mowgli_program_opts_t *
mowgli_program_opts_lookup_name(mowgli_program_opts_t *opts, size_t opts_size, const char *name)
{
	size_t i;

	if (strlen(name) > 1)
		for (i = 0; i < opts_size; i++)
			if (!strcasecmp(name, opts[i].longopt))
				return &opts[i];

			else
				for (i = 0; i < opts_size; i++)
					if (*name == opts[i].smallopt)
						return &opts[i];

	return NULL;
}

static inline mowgli_getopt_option_t *
mowgli_program_opts_convert(const mowgli_program_opts_t *opts, size_t opts_size)
{
	mowgli_getopt_option_t *g_opts;
	size_t i;

	return_val_if_fail(opts != NULL, NULL);

	g_opts = mowgli_alloc_array(sizeof(mowgli_getopt_option_t), opts_size);

	for (i = 0; i < opts_size; i++)
	{
		if (opts[i].longopt == NULL)
			continue;

		g_opts[i].name = opts[i].longopt;
		g_opts[i].iflag = i;

		if (opts[i].has_param)
			g_opts[i].has_arg = 1;
	}

	return g_opts;
}

static inline const char *
mowgli_program_opts_compute_optstr(const mowgli_program_opts_t *opts, size_t opts_size)
{
	static char buf[256];
	char *p = buf;
	size_t i;

	return_val_if_fail(opts != NULL, NULL);

	memset(buf, '\0', sizeof buf);

	for (i = 0; i < opts_size; i++)
	{
		if (!opts[i].smallopt)
			continue;

		*p++ = opts[i].smallopt;

		if (opts[i].has_param)
			*p++ = ':';
	}

	*p = '\0';

	return buf;
}

static inline void
mowgli_program_opts_dispatch(const mowgli_program_opts_t *opt, const char *optarg)
{
	return_if_fail(opt != NULL);

	if (opt->has_param && (optarg == NULL))
	{
		fprintf(stderr, "no optarg for option %s", opt->longopt);
		return;
	}

	opt->consumer(optarg, opt->userdata);
}

void
mowgli_program_opts_parse(const mowgli_program_opts_t *opts, size_t opts_size, int *argc, char ***argv)
{
	mowgli_getopt_option_t *g_opts;
	const char *shortops;
	int c;
	size_t i;
	int opt_index;

	return_if_fail(opts != NULL);
	return_if_fail(opts_size > 0);
	return_if_fail(argc != NULL);
	return_if_fail(argv != NULL);

	g_opts = mowgli_program_opts_convert(opts, opts_size);
	shortops = mowgli_program_opts_compute_optstr(opts, opts_size);

	for (;;)
	{
		const mowgli_program_opts_t *opt = NULL;

		c = mowgli_getopt_long(*argc, *argv, shortops, g_opts, &opt_index);

		if (c == -1)
			break;

		switch (c)
		{
		case 0:

			/* long-option was provided, resolve it. */
			opt = &opts[g_opts[opt_index].iflag];
			break;
		default:

			for (i = 0; i < opts_size; i++)
				if (opts[i].smallopt == c)
				{
					opt = &opts[i];
					break;
				}

			break;
		}

		mowgli_program_opts_dispatch(opt, mowgli_optarg);
	}

	mowgli_free(g_opts);
}
