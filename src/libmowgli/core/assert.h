/*
 * libmowgli: A collection of useful routines for programming.
 * assert.h: Assertions.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
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

#ifndef __MOWGLI_ASSERT_H__
#define __MOWGLI_ASSERT_H__

#define _assert_msg(exp) "assertion '" #exp  "' failed."

#define soft_assert(exp) \
	do { \
		if (!(exp)) { \
			mowgli_log_warning(_assert_msg(exp)); \
		} \
	} while(0)

#define return_if_fail(exp) \
	do { \
		if (!(exp)) { \
			mowgli_log_warning(_assert_msg(exp)); \
			return; \
		} \
	} while(0)

#define return_val_if_fail(exp, val) \
	do { \
		if (!(exp)) { \
			mowgli_log_warning(_assert_msg(exp)); \
			return (val); \
		} \
	} while(0)

#define return_null_if_fail(exp) return_val_if_fail(exp, NULL)

#endif
