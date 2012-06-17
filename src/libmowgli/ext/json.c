/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in COPYING
 *
 * Structs and functions for interacting with JSON documents from C
 */

#include <mowgli.h>

#define JSON_REFCOUNT_CONSTANT -42

static mowgli_json_t json_null = {
	.tag = MOWGLI_JSON_TAG_NULL,
	.refcount = JSON_REFCOUNT_CONSTANT,
};
mowgli_json_t *mowgli_json_null = &json_null;

static mowgli_json_t json_true = {
	.tag = MOWGLI_JSON_TAG_BOOLEAN,
	.refcount = JSON_REFCOUNT_CONSTANT,
	.v_bool = true,
};
mowgli_json_t *mowgli_json_true = &json_true;

static mowgli_json_t json_false = {
	.tag = MOWGLI_JSON_TAG_BOOLEAN,
	.refcount = JSON_REFCOUNT_CONSTANT,
	.v_bool = false,
};
mowgli_json_t *mowgli_json_false = &json_false;

static mowgli_json_t *json_alloc(mowgli_json_tag_t tag);
static void json_destroy(mowgli_json_t *n);
static void destroy_extra_string(mowgli_json_t *n);
static void destroy_extra_array(mowgli_json_t *n);
static void destroy_extra_object(mowgli_json_t *n);

typedef void (*destroy_extra_cb_t)(mowgli_json_t*);
static destroy_extra_cb_t destroy_extra[] = {
	[MOWGLI_JSON_TAG_STRING] = destroy_extra_string,
	[MOWGLI_JSON_TAG_ARRAY] = destroy_extra_array,
	[MOWGLI_JSON_TAG_OBJECT] = destroy_extra_object,
};

mowgli_json_t *mowgli_json_incref(mowgli_json_t *n)
{
	if (n == NULL || n->refcount == JSON_REFCOUNT_CONSTANT)
		return n;

	n->refcount++;

	return n;
}

mowgli_json_t *mowgli_json_decref(mowgli_json_t *n)
{
	if (n == NULL || n->refcount == JSON_REFCOUNT_CONSTANT)
		return n;

	n->refcount--;

	if (n->refcount <= 0) {
		json_destroy(n);
		return NULL;
	}

	return n;
}

static mowgli_json_t *json_alloc(mowgli_json_tag_t tag)
{
	mowgli_json_t *n;

	n = mowgli_alloc(sizeof(*n));

	n->tag = tag;
	n->refcount = 0;

	return n;
}

static void json_destroy(mowgli_json_t *n)
{
	return_if_fail(n != NULL);

	if (destroy_extra[n->tag] != NULL)
		destroy_extra[n->tag](n);

	mowgli_free(n);
}

mowgli_json_t *mowgli_json_create_integer(int v_int)
{
	mowgli_json_t *n = json_alloc(MOWGLI_JSON_TAG_INTEGER);
	n->v_int = v_int;
	return n;
}

mowgli_json_t *mowgli_json_create_float(double v_float)
{
	mowgli_json_t *n = json_alloc(MOWGLI_JSON_TAG_FLOAT);
	n->v_float = v_float;
	return n;
}

mowgli_json_t *mowgli_json_create_string_n(const char *str, size_t len)
{
	mowgli_json_t *n = json_alloc(MOWGLI_JSON_TAG_STRING);

	n->v_string = mowgli_string_create();
	mowgli_string_append(n->v_string, str, len);

	return n;
}

mowgli_json_t *mowgli_json_create_string(const char *str)
{
	return mowgli_json_create_string_n(str, strlen(str));
}

static void destroy_extra_string(mowgli_json_t *n)
{
	mowgli_string_destroy(n->v_string);
}

mowgli_json_t *mowgli_json_create_array(void)
{
	mowgli_json_t *n = json_alloc(MOWGLI_JSON_TAG_ARRAY);
	n->v_array = mowgli_list_create();
	return n;
}

static void destroy_extra_array(mowgli_json_t *n)
{
	mowgli_node_t *cur, *next;

	MOWGLI_LIST_FOREACH_SAFE(cur, next, n->v_array->head) {
		mowgli_json_decref((mowgli_json_t*)cur->data);
		mowgli_node_delete(cur, n->v_array);
	}

	mowgli_list_free(n->v_array);
}

mowgli_json_t *mowgli_json_create_object(void)
{
	mowgli_json_t *n = json_alloc(MOWGLI_JSON_TAG_OBJECT);
	n->v_object = mowgli_patricia_create(NULL);
	return n;
}

static void destroy_extra_object_cb(const char *key, void *data, void *privdata)
{
	mowgli_json_decref((mowgli_json_t*)data);
}
static void destroy_extra_object(mowgli_json_t *n)
{
	mowgli_patricia_destroy(n->v_object, destroy_extra_object_cb, NULL);
}
