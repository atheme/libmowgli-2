/*
 * libmowgli: A collection of useful routines for programming.
 * lineardict.c: Testing of the linear dictionary routines.
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

#include <mowgli.h>

int str_comparator(void *left, void *right, void *data)
{
	return strcasecmp(left, right);
}

void
dictionary_add(mowgli_dictionary_t *dict, char *data)
{
	mowgli_dictionary_add(dict, data, data);
}

void test_strings(void)
{
	char *str;
	mowgli_dictionary_t *dict;
	mowgli_dictionary_iteration_state_t state;

	dict = mowgli_dictionary_create(str_comparator);
	mowgli_dictionary_set_linear_comparator_func(dict, str_comparator, NULL);

	dictionary_add(dict, "foo");
	dictionary_add(dict, "bar");
	dictionary_add(dict, "baz");
	dictionary_add(dict, "splork");
	dictionary_add(dict, "rabbit");
	dictionary_add(dict, "meow");
	dictionary_add(dict, "hi");
	dictionary_add(dict, "konnichiwa");
	dictionary_add(dict, "absolutely");
	dictionary_add(dict, "cat");
	dictionary_add(dict, "dog");
	dictionary_add(dict, "woof");
	dictionary_add(dict, "moon");
	dictionary_add(dict, "new");
	dictionary_add(dict, "delete");
	dictionary_add(dict, "alias");

	printf("\nString test results\n");

	MOWGLI_DICTIONARY_FOREACH(str, &state, dict)
	{
		printf("  %s\n", str);
	}

	mowgli_dictionary_destroy(dict, NULL, NULL);
}

int int_comparator(void *left, void *right, void *data)
{
	int a = (int) left;
	int b = (int) right;

	return a - b;
}

void test_integers(void)
{
	mowgli_dictionary_t *dict;
	mowgli_dictionary_iteration_state_t state;
	void *x;

	dict = mowgli_dictionary_create(str_comparator);
	mowgli_dictionary_set_linear_comparator_func(dict, int_comparator, NULL);

	mowgli_dictionary_add(dict, "a", 3);
	mowgli_dictionary_add(dict, "b", 2);
	mowgli_dictionary_add(dict, "c", 4);
	mowgli_dictionary_add(dict, "d", 1);

	printf("\nInteger test results\n");

	MOWGLI_DICTIONARY_FOREACH(x, &state, dict)
	{
		printf("  %d\n", (int) x);
	}
}

int main(int argc, char *argv[])
{
	mowgli_init();

	test_strings();
	test_integers();
}
