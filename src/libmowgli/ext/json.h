/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in COPYING.
 *
 * Structs and functions for interacting with JSON documents from C
 */

#ifndef MOWGLI_JSON_H
#define MOWGLI_JSON_H

#include <mowgli.h>

/* Types with public definitons */
typedef enum _mowgli_json_tag_t mowgli_json_tag_t;
typedef struct _mowgli_json_t mowgli_json_t;
/* Types whose definitons are kept private */
typedef struct _mowgli_json_parse_t mowgli_json_parse_t;

enum _mowgli_json_tag_t {
	MOWGLI_JSON_TAG_NULL,
	MOWGLI_JSON_TAG_BOOLEAN,
	/* JSON does not distinguish between integers or floating points
	   (they are both just number types), but in C we are forced to
	   make the distinction */
	MOWGLI_JSON_TAG_INTEGER,
	MOWGLI_JSON_TAG_FLOAT,
	MOWGLI_JSON_TAG_STRING,
	MOWGLI_JSON_TAG_ARRAY,
	MOWGLI_JSON_TAG_OBJECT,
};

struct _mowgli_json_t {
	mowgli_json_tag_t tag;
	int refcount;

	/* NOTE: unnamed unions like this are a GNU extension and probably
	   not supported in many other compilers. Options need to be
	   discussed */
	union {
		bool v_bool;
		int v_int;
		double v_float;
		mowgli_string_t *v_string;
		mowgli_list_t *v_array;
		mowgli_patricia_t *v_object;
	};
};

/* Helper macros useful for cleanly interacting with mowgil_json_t
   structs. These are just helpers and are not intended to provide a
   consistent interface. They are as likely to change as the mowgli_json_t
   struct itself. */
#define MOWGLI_JSON_TAG(n)		((n)->tag)
#define MOWGLI_JSON_BOOLEAN(n)		((n)->v_bool)
#define MOWGLI_JSON_INTEGER(n)		((n)->v_int)
#define MOWGLI_JSON_FLOAT(n)		((n)->v_float)
#define MOWGLI_JSON_NUMBER(n)		((n)->tag == MOWGLI_JSON_TAG_FLOAT ? \
					   (n)->v_float : (n)->v_int)
#define MOWGLI_JSON_STRING(n)		((n)->v_string)
#define MOWGLI_JSON_STRING_STR(n)	((n)->v_string->str)
#define MOWGLI_JSON_STRING_LEN(n)	((n)->v_string->pos)
#define MOWGLI_JSON_ARRAY(n)		((n)->v_array)
#define MOWGLI_JSON_OBJECT(n)		((n)->v_object)

/* Users of the JSON parser/formatter are not required to use these
   constants, but the parser will ALWAYS parse the symbols "null",
   "true", and "false" into these constants */
extern mowgli_json_t *mowgli_json_null;
extern mowgli_json_t *mowgli_json_true;
extern mowgli_json_t *mowgli_json_false;

/* These simply return the node argument. If the node was destroyed,
   the return value will be NULL. Note that in all other cases (invalid
   refcount, unknown type, etc.) the node will be returned untouched */
extern mowgli_json_t *mowgli_json_incref(mowgli_json_t *n);
extern mowgli_json_t *mowgli_json_decref(mowgli_json_t *n);

extern mowgli_json_t *mowgli_json_create_integer(int v_int);
extern mowgli_json_t *mowgli_json_create_float(double v_float);
/* #define mowgli_json_create_number(V) _Generic((V), \
	int: mowgli_json_create_integer, \
	double: mowgli_json_create_float)(V) */
extern mowgli_json_t *mowgli_json_create_string_n(const char *str, unsigned len);
extern mowgli_json_t *mowgli_json_create_string(const char *str);
extern mowgli_json_t *mowgli_json_create_array(void);
extern mowgli_json_t *mowgli_json_create_object(void);

#include "json-inline.h"

extern void mowgli_json_serialize(mowgli_json_t *n, mowgli_string_t *str, int pretty);

typedef struct _mowgli_json_parse_t mowgli_json_parse_t;

/* extended parsing interface */
extern mowgli_json_parse_t *mowgli_json_parse_create(void);
extern void mowgli_json_parse_destroy(mowgli_json_parse_t *parse);
extern void mowgli_json_parse_reset(mowgli_json_parse_t *parse);
extern void mowgli_json_parse_data(mowgli_json_parse_t *parse, const char *data, size_t len);
extern char *mowgli_json_parse_error(mowgli_json_parse_t *parse);
extern bool mowgli_json_parse_more(mowgli_json_parse_t *parse);
extern mowgli_json_t *mowgli_json_parse_next(mowgli_json_parse_t *parse);

/* simple parsing interface */
extern mowgli_json_t *mowgli_json_parse_file(const char *path);
extern mowgli_json_t *mowgli_json_parse_string(const char *data);

#endif
