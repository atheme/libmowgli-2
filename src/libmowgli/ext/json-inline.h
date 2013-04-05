/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in COPYING.
 *
 * JSON inline functions, to simplify refcounting etc for other users
 */

#ifndef MOWGLI_JSON_INLINE_H
#define MOWGLI_JSON_INLINE_H

/* We don't need to include any other headers here. This is in a separate
   file to keep clutter out of the main json.h */

/* string */
static inline void
mowgli_json_string_reset(mowgli_json_t *s)
{
	mowgli_string_reset(MOWGLI_JSON_STRING(s));
}

static inline void
mowgli_json_string_append(mowgli_json_t *s, const char *src, size_t n)
{
	mowgli_string_append(MOWGLI_JSON_STRING(s), src, n);
}

static inline void
mowgli_json_string_append_char(mowgli_json_t *s, const char c)
{
	mowgli_string_append_char(MOWGLI_JSON_STRING(s), c);
}

/* array */
static inline size_t
mowgli_json_array_size(mowgli_json_t *arr)
{
	return MOWGLI_JSON_ARRAY(arr)->count;
}

static inline void
mowgli_json_array_add(mowgli_json_t *arr, mowgli_json_t *data)
{
	mowgli_node_add(mowgli_json_incref(data), mowgli_node_create(), MOWGLI_JSON_ARRAY(arr));
}

static inline void
mowgli_json_array_add_head(mowgli_json_t *arr, mowgli_json_t *data)
{
	mowgli_node_add_head(mowgli_json_incref(data), mowgli_node_create(), MOWGLI_JSON_ARRAY(arr));
}

static inline void
mowgli_json_array_insert(mowgli_json_t *arr, mowgli_json_t *data, size_t pos)
{
	mowgli_node_insert(mowgli_json_incref(data), mowgli_node_create(), MOWGLI_JSON_ARRAY(arr), pos);
}

static inline void
mowgli_json_array_delete(mowgli_json_t *arr, mowgli_json_t *data)
{
	mowgli_node_t *n = mowgli_node_find(data, MOWGLI_JSON_ARRAY(arr));

	if (n == NULL)
		return;

	mowgli_node_delete(n, MOWGLI_JSON_ARRAY(arr));
	mowgli_json_decref(data);
}

static inline bool
mowgli_json_array_contains(mowgli_json_t *arr, mowgli_json_t *data)
{
	return mowgli_node_find(data, MOWGLI_JSON_ARRAY(arr)) != NULL;
}

static inline mowgli_json_t *
mowgli_json_array_nth(mowgli_json_t *arr, size_t pos)
{
	return (mowgli_json_t *) mowgli_node_nth_data(MOWGLI_JSON_ARRAY(arr), pos);
}

/* object */
static inline size_t
mowgli_json_object_size(mowgli_json_t *obj)
{
	return mowgli_patricia_size(MOWGLI_JSON_OBJECT(obj));
}

static inline void
mowgli_json_object_add(mowgli_json_t *obj, const char *key, mowgli_json_t *data)
{
	mowgli_patricia_add(MOWGLI_JSON_OBJECT(obj), key, mowgli_json_incref(data));
}

static inline mowgli_json_t *
mowgli_json_object_retrieve(mowgli_json_t *obj, const char *key)
{
	return (mowgli_json_t *) mowgli_patricia_retrieve(MOWGLI_JSON_OBJECT(obj), key);
}

static inline mowgli_json_t *
mowgli_json_object_delete(mowgli_json_t *obj, const char *key)
{
	return (mowgli_json_t *) mowgli_patricia_delete(MOWGLI_JSON_OBJECT(obj), key);
}

#endif
