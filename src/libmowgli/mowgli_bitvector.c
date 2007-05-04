/*
 * libmowgli: A collection of useful routines for programming.
 * mowgli_bitvector.c: Bitvectors.
 *
 * Copyright (c) 2007 William Pitcock <nenolod -at- sacredspiral.co.uk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

mowgli_bitvector_t *mowgli_bitvector_create(int bits)
{
	mowgli_bitvector_t *bv = (mowgli_bitvector_t *) mowgli_alloc(sizeof(mowgli_bitvector_t));

	bv->bits    = bits;
	bv->divisor = sizeof(int);
	bv->vector  = (unsigned int *) mowgli_alloc_array(bv->divisor, bv->bits / bv->divisor);

	return bv;
}

void mowgli_bitvector_set(mowgli_bitvector_t *bv, int slot, mowgli_boolean_t val)
{
	int value = 1 << slot;

	switch(val)
	{
		case FALSE:
			bv->vector[bv->bits / bv->divisor] &= ~value;
			break;
		default:
		case TRUE:
			bv->vector[bv->bits / bv->divisor] |= value;
			break;
	}
}

mowgli_boolean_t mowgli_bitvector_get(mowgli_bitvector_t *bv, int slot)
{
	int mask = 1 << slot;

	return ((bv->vector[bv->bits / bv->divisor] & mask) != 0) ? TRUE : FALSE;
}

mowgli_bitvector_t *mowgli_bitvector_combine(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2)
{
	int bits, iter, bs;
	mowgli_bitvector_t *out;

	return_val_if_fail(bv1 != NULL, NULL);
	return_val_if_fail(bv2 != NULL, NULL);

	/* choose the larger bitwidth */
	bits = bv1->bits > bv2->bits ? bv1->bits : bv2->bits;

	/* create the third bitvector. */
	out = mowgli_bitvector_create(bits);

	/* cache the size of the bitvector in memory. */
	bs = out->bits / out->divisor;

	for (iter = 0; iter < bs; iter++)
	{
		out->vector[iter] |= bv1->vector[iter];
		out->vector[iter] |= bv2->vector[iter];
	}

	return out;
}

mowgli_bitvector_t *mowgli_bitvector_xor(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2)
{
	int bits, iter, bs;
	mowgli_bitvector_t *out;

	return_val_if_fail(bv1 != NULL, NULL);
	return_val_if_fail(bv2 != NULL, NULL);

	/* choose the larger bitwidth */
	bits = bv1->bits > bv2->bits ? bv1->bits : bv2->bits;

	/* create the third bitvector. */
	out = mowgli_bitvector_create(bits);

	/* cache the size of the bitvector in memory. */
	bs = out->bits / out->divisor;

	for (iter = 0; iter < bs; iter++)
	{
		out->vector[iter] = bv1->vector[iter];
		out->vector[iter] &= ~bv2->vector[iter];
	}

	return out;
}

mowgli_boolean_t mowgli_bitvector_compare(mowgli_bitvector_t *bv1, mowgli_bitvector_t *bv2)
{
	int iter, bs;	
	mowgli_boolean_t ret = TRUE;

	/* cache the size of the bitvector in memory. */
	bs = bv1->bits / bv1->divisor;

	for (iter = 0; iter < bs; iter++)
	{
		if (!(bv1->vector[iter] & bv2->vector[iter]))
			ret = FALSE;
	}

	return ret;
}
