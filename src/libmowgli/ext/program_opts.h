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

#ifndef __MOWGLI_PROGRAM_OPTS_H__
#define __MOWGLI_PROGRAM_OPTS_H__

typedef void (*mowgli_program_opts_consumer_t)(const char *arg, void *userdata);

typedef struct
{
	const char *longopt;
	const char smallopt;
	bool has_param;
	mowgli_program_opts_consumer_t consumer;
	void *userdata;

	/* optional data */
	const char *description;
	const char *paramname;
} mowgli_program_opts_t;

/* use when has_param is true */
extern void mowgli_program_opts_consumer_str(const char *arg, void *userdata);
extern void mowgli_program_opts_consumer_int(const char *arg, void *userdata);

/* use when has_param is false */
extern void mowgli_program_opts_consumer_bool(const char *arg, void *userdata);

extern void mowgli_program_opts_parse(const mowgli_program_opts_t *opts, size_t opts_size, int *argc, char ***argv);

#endif
