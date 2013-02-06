/*
 * libmowgli: A collection of useful routines for programming.
 * bitvector.h: Bitvectors.
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

#ifndef __MOWGLI_BITVECTOR_H__
#define __MOWGLI_BITVECTOR_H__

typedef struct {
	unsigned int bits;
	unsigned int divisor;
	unsigned int *vector;
} mowgli_bitvector_t;

extern mowgli_bitvector_t *mowgli_bitvector_create(int bits);
extern void mowgli_bitvector_set(mowgli_bitvector_t *bv, int slot, mowgli_boolean_t val);
extern mowgli_boolean_t mowgli_bitvector_get(mowgli_bitvector_t *bv, int slot);
extern mowgli_bitvector_t *mowgli_bitvector_combine(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2);
extern mowgli_bitvector_t *mowgli_bitvector_xor(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2);
extern mowgli_boolean_t mowgli_bitvector_compare(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2);

#endif
