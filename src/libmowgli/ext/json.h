/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in COPYING.
 *
 * Structs and functions for interacting with JSON documents from C
 */

/* A note about refcounting:

   Since JSON cannot represent recursive structures, it makes sense
   to use refcounting. If a recursive structure is created then it's
   guaranteed to be the programmer's fault, and the programmer deserves
   every memory leak he gets.

   When cared for and fed daily, refcounting is a very nifty tool. Entire
   structures can be destroyed cleanly by a single mowgli_json_decref. To
   fully take advantage of this, know the refcounting rules:

     o  JSON objects as arguments should NEVER have a reference for
        the callee.  If the callee wants to keep the object somewhere,
        they will incref themselves.

     o  When returning a JSON object, be sure to leave a single reference
        for the caller. (If you tried to decref before returning, you
        could potentially drop the refcount to zero.)

     o  Newly created structures are the sole exception to the previous
        rule. This is because it is useful to be able to use constructors
        as arguments to functions as follows:
          mowgli_json_object_add(obj, "sum", mowgli_json_create_integer(10));
        Note that this ONLY applies to the mowgli_json_create_*
        constructors provided by mowgli.json. The typical case of
        assigning a new structure to a variable name then looks like this:
          my_integer = mowgli_json_incref(mowgli_json_create_integer(0));

   Since failure to pay attention to reference counts will cause memory
   problems, *always double check your reference counting*!

   --aji */

#ifndef MOWGLI_JSON_H
#define MOWGLI_JSON_H

/* Types with public definitons */
typedef enum
{
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
} mowgli_json_tag_t;
typedef struct _mowgli_json_t mowgli_json_t;

/* Types whose definitons are kept private */
typedef struct _mowgli_json_parse_t mowgli_json_parse_t;

struct _mowgli_json_t
{
	mowgli_json_tag_t tag;
	int refcount;

	union
	{
		bool v_bool;
		int v_int;
		double v_float;
		mowgli_string_t *v_string;
		mowgli_list_t *v_array;
		mowgli_patricia_t *v_object;
	} v;
};

/* Helper macros useful for cleanly interacting with mowgli_json_t
   structs. These are just helpers and are not intended to provide a
   consistent interface. They are as likely to change as the mowgli_json_t
   struct itself. */
#define MOWGLI_JSON_TAG(n) ((n)->tag)
#define MOWGLI_JSON_BOOLEAN(n) ((n)->v.v_bool)
#define MOWGLI_JSON_INTEGER(n) ((n)->v.v_int)
#define MOWGLI_JSON_FLOAT(n) ((n)->v.v_float)
#define MOWGLI_JSON_NUMBER(n) ((n)->tag == MOWGLI_JSON_TAG_FLOAT ? \
			       (n)->v.v_float : (n)->v.v_int)
#define MOWGLI_JSON_STRING(n) ((n)->v.v_string)
#define MOWGLI_JSON_STRING_STR(n) ((n)->v.v_string->str)
#define MOWGLI_JSON_STRING_LEN(n) ((n)->v.v_string->pos)
#define MOWGLI_JSON_ARRAY(n) ((n)->v.v_array)
#define MOWGLI_JSON_OBJECT(n) ((n)->v.v_object)

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
extern mowgli_json_t *mowgli_json_create_string_n(const char *str, size_t len);
extern mowgli_json_t *mowgli_json_create_string(const char *str);
extern mowgli_json_t *mowgli_json_create_array(void);
extern mowgli_json_t *mowgli_json_create_object(void);

#include "json-inline.h"

typedef struct _mowgli_json_output_t mowgli_json_output_t;

struct _mowgli_json_output_t
{
	void (*append)(mowgli_json_output_t *out, const char *str, size_t len);
	void (*append_char)(mowgli_json_output_t *out, const char c);
	void *priv;
};

extern void mowgli_json_serialize(mowgli_json_t *n, mowgli_json_output_t *out, int pretty);
extern void mowgli_json_serialize_to_string(mowgli_json_t *n, mowgli_string_t *str, int pretty);

/* extended parsing interface. The 'multidoc' parameter here indicates
   whether we intend to parse multiple documents from a single data source
   or not.  If you are expecting exactly one complete JSON document,
   indicate 'false'. */
extern mowgli_json_parse_t *mowgli_json_parse_create(bool multidoc);
extern void mowgli_json_parse_destroy(mowgli_json_parse_t *parse);
extern void mowgli_json_parse_reset(mowgli_json_parse_t *parse, bool multidoc);
extern void mowgli_json_parse_data(mowgli_json_parse_t *parse, const char *data, size_t len);
extern char *mowgli_json_parse_error(mowgli_json_parse_t *parse);
extern bool mowgli_json_parse_more(mowgli_json_parse_t *parse);
extern mowgli_json_t *mowgli_json_parse_next(mowgli_json_parse_t *parse);

/* Simple parsing interface. These expect the given data source to
   represent exactly one complete JSON document */
extern mowgli_json_t *mowgli_json_parse_file(const char *path);
extern mowgli_json_t *mowgli_json_parse_string(const char *data);

#endif
