/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Structs and functions for manipulating and describing JSON
 */

#ifndef MOWGLI_JSON_H
#define MOWGLI_JSON_H

#include <mowgli.h>

typedef enum _mowgli_json_tag_t mowgli_json_tag_t;
typedef struct _mowgli_json_t mowgli_json_t;

enum _mowgli_json_tag_t {
	MOWGLI_JSON_NULL,
	MOWGLI_JSON_BOOL,
	/* JSON does not distinguish between integers or floating points
	   (they are both just number types), but in C we are forced to
	   make the distinction */
	MOWGLI_JSON_INTEGER,
	MOWGLI_JSON_FLOAT,
	MOWGLI_JSON_STRING,
	MOWGLI_JSON_ARRAY,
	MOWGLI_JSON_OBJECT,
};

struct _mowgli_json_t {
	mowgli_json_tag_t tag;

	/* NOTE: unnamed unions like this are a GNU extension and probably
	   not supported in many other compilers. Options need to be
	   discussed */
	union {
		bool v_bool;
		int v_int;
		float v_float;
		mowgli_string_t *v_string;
		mowgli_list_t *v_array;
		mowgli_patricia_t *v_object;
	};
};

#endif
