/*
 * libmowgli: A collection of useful routines for programming.
 * class.c: Object class and type management, cast checking.
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

#include "mowgli.h"

static mowgli_patricia_t *mowgli_object_class_dict;

static void
_object_key_canon(char *str)
{
	while (*str)
	{
		*str = toupper(*str);
		str++;
	}
}

void
mowgli_object_class_bootstrap()
{
	mowgli_object_class_dict = mowgli_patricia_create(_object_key_canon);
}

void
mowgli_object_class_init(mowgli_object_class_t *klass, const char *name, mowgli_destructor_t des, mowgli_boolean_t dynamic)
{
	return_if_fail(klass != NULL);
	return_if_fail(mowgli_object_class_find_by_name(name) == NULL);

	/* initialize object_class::name */
	klass->name = mowgli_strdup(name);

	/* initialize object_class::derivitives */
	klass->derivitives.head = NULL;
	klass->derivitives.tail = NULL;
	klass->derivitives.count = 0;

	/* initialize object_class::destructor */
	klass->destructor = des != NULL ? des : mowgli_free;

	/* initialize object_class::dynamic */
	klass->dynamic = dynamic;

	/* add to the object_class index */
	mowgli_patricia_add(mowgli_object_class_dict, klass->name, klass);
}

int
mowgli_object_class_check_cast(mowgli_object_class_t *klass1, mowgli_object_class_t *klass2)
{
	return_val_if_fail(klass1 != NULL, 0);
	return_val_if_fail(klass2 != NULL, 0);

	mowgli_node_t *n;

	MOWGLI_LIST_FOREACH(n, klass1->derivitives.head)
	{
		mowgli_object_class_t *tklass = (mowgli_object_class_t *) n->data;

		if (tklass == klass2)
			return 1;
	}

	return 0;
}

void
mowgli_object_class_set_derivitive(mowgli_object_class_t *klass, mowgli_object_class_t *parent)
{
	return_if_fail(klass != NULL);
	return_if_fail(parent != NULL);

	mowgli_node_add(klass, mowgli_node_create(), &parent->derivitives);
}

void *
mowgli_object_class_reinterpret_impl( /* mowgli_object_t */ void *opdata, mowgli_object_class_t *klass)
{
	mowgli_object_t *object = mowgli_object(opdata);

	/* this can possibly happen at runtime .. lets not make it a fatal exception. */
	return_val_if_fail(object != NULL, NULL);
	return_val_if_fail(klass != NULL, NULL);

	if (mowgli_object_class_check_cast(object->klass, klass))
		return object;

	mowgli_log("Invalid reinterpreted cast from %s<%p> to %s", object->klass->name, (void *) object, klass->name);
	return NULL;
}

mowgli_object_class_t *
mowgli_object_class_find_by_name(const char *name)
{
	return mowgli_patricia_retrieve(mowgli_object_class_dict, name);
}

void
mowgli_object_class_destroy(mowgli_object_class_t *klass)
{
	return_if_fail(klass != NULL);
	return_if_fail(klass->dynamic == TRUE);

	mowgli_node_t *n, *tn;

	MOWGLI_LIST_FOREACH_SAFE(n, tn, klass->derivitives.head)
	{
		mowgli_node_delete(n, &klass->derivitives);
		mowgli_node_free(n);
	}

	mowgli_free(klass->name);
	mowgli_free(klass);
}
