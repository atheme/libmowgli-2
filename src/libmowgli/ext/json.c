/*
 * Copyright (C) 2012 Alex Iadicicco
 * Rights to this code are as documented in COPYING
 *
 * Structs and functions for interacting with JSON documents from C
 */

#include <mowgli.h>

/* TOC:
 * 0. JSON subsystem constants
 * 1. Reference counters
 * 2. Object creation and deletion
 * 3. Serializer (a.k.a. formatter, printer, etc.)
 * 4. Deserializer (a.k.a. parser, etc.)
 */

/*
 * 0. JSON SUBSYSTEM CONSTANTS
 */

#define JSON_REFCOUNT_CONSTANT -42

static mowgli_json_t json_null =
{
	.tag = MOWGLI_JSON_TAG_NULL,
	.refcount = JSON_REFCOUNT_CONSTANT,
};
mowgli_json_t *mowgli_json_null = &json_null;

static mowgli_json_t json_true =
{
	.tag = MOWGLI_JSON_TAG_BOOLEAN,
	.refcount = JSON_REFCOUNT_CONSTANT,
	.v_bool = true,
};
mowgli_json_t *mowgli_json_true = &json_true;

static mowgli_json_t json_false =
{
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
static destroy_extra_cb_t destroy_extra[] =
{
	[MOWGLI_JSON_TAG_STRING] = destroy_extra_string,
	[MOWGLI_JSON_TAG_ARRAY] = destroy_extra_array,
	[MOWGLI_JSON_TAG_OBJECT] = destroy_extra_object,
};

/*
 * 1. REFERENCE COUNTERS
 */

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

	if (n->refcount <= 0)
	{
		json_destroy(n);
		return NULL;
	}

	return n;
}

/*
 * 2. OBJECT CREATION AND DELETION
 */

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

	MOWGLI_LIST_FOREACH_SAFE(cur, next, n->v_array->head)
	{
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

/*
 * 3. SERIALIZER (A.K.A. FORMATTER, PRINTER, ETC.)
 */

#define TAB_STRING "    "
#define TAB_LEN 4

static void serialize_pretty_indent(mowgli_string_t *str, int pretty)
{
	int i;

	for (i=0; i<pretty; i++)
		mowgli_string_append(str, TAB_STRING, TAB_LEN);
}

static void serialize_pretty_break(mowgli_string_t *str, int pretty)
{
	if (pretty < 1)
		return;

	mowgli_string_append_char(str, '\n');
}

static int serialize_pretty_increment(int pretty)
{
	return (pretty > 0 ? pretty + 1 : 0);
}

static void serialize_boolean(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	if (n->v_bool)
		mowgli_string_append(str, "true", 4);
	else
		mowgli_string_append(str, "false", 5);
}

static void serialize_int(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	char buf[32];
	size_t len;

	len = snprintf(buf, 32, "%d", n->v_int);
	mowgli_string_append(str, buf, len);
}

static void serialize_float(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	char buf[32];
	size_t len;

	len = snprintf(buf, 32, "%g", n->v_float);
	mowgli_string_append(str, buf, len);
}

static const char *serialize_hex_digits = "0123456789abcdef";
static const char *serialize_escape = "\"\\\b\f\n\r\t";
static void serialize_string_data(const char *p, size_t len, mowgli_string_t *str)
{
	unsigned i;
	unsigned char c;

	mowgli_string_append_char(str, '"');

	for (i=0; i<len; i++)
	{
		c = p[i];

		if (strchr(serialize_escape, c))
		{
			mowgli_string_append_char(str, '\\');

			switch (c)
			{
			case '"': mowgli_string_append_char(str, '"'); break;
			case '\\': mowgli_string_append_char(str, '\\'); break;
			//case '/': mowgli_string_append_char(str, '/'); break;
			case '\b': mowgli_string_append_char(str, 'b'); break;
			case '\f': mowgli_string_append_char(str, 'f'); break;
			case '\n': mowgli_string_append_char(str, 'n'); break;
			case '\r': mowgli_string_append_char(str, 'r'); break;
			case '\t': mowgli_string_append_char(str, 't'); break;
			default: // hurrr
				mowgli_string_append_char(str, c);
			}
		}
		else if (c < 0x20 || c > 0x7f)
		{
			mowgli_string_append_char(str, '\\');

			/* TODO: fix this... */
			mowgli_string_append_char(str, 'u');
			mowgli_string_append_char(str, '0');
			mowgli_string_append_char(str, '0');
			mowgli_string_append_char(str, serialize_hex_digits[(c >> 4) & 0xf]);
			mowgli_string_append_char(str, serialize_hex_digits[(c >> 0) & 0xf]);
		}
		else
		{
			mowgli_string_append_char(str, c);
		}
	}

	mowgli_string_append_char(str, '"');
}
static void serialize_string(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	serialize_string_data(n->v_string->str, n->v_string->pos, str);
}

static void serialize_array(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	mowgli_node_t *cur;

	mowgli_string_append_char(str, '[');
	serialize_pretty_break(str, pretty);

	MOWGLI_LIST_FOREACH(cur, n->v_array->head)
	{
		serialize_pretty_indent(str, pretty);
		mowgli_json_serialize(cur->data, str, serialize_pretty_increment(pretty));

		if (cur->next != NULL)
			mowgli_string_append_char(str, ',');
		serialize_pretty_break(str, pretty);
	}

	serialize_pretty_indent(str, pretty - 1);
	mowgli_string_append_char(str, ']');
}

struct serialize_object_priv /* lol, this is bullshit */
{
	int pretty;
	int remaining;
	mowgli_string_t *str;
};

static int serialize_object_cb(const char *key, void *data, void *privdata)
{
	struct serialize_object_priv *priv = privdata;

	priv->remaining--;

	serialize_pretty_indent(priv->str, priv->pretty);

	serialize_string_data(key, strlen(key), priv->str);
	mowgli_string_append_char(priv->str, ':');
	if (priv->pretty)
		mowgli_string_append_char(priv->str, ' ');

	mowgli_json_serialize(data, priv->str,
			serialize_pretty_increment(priv->pretty));

	if (priv->remaining)
		mowgli_string_append_char(priv->str, ',');
	serialize_pretty_break(priv->str, priv->pretty);

	return 0;
}

static void serialize_object(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	struct serialize_object_priv priv;

	mowgli_string_append_char(str, '{');
	serialize_pretty_break(str, pretty);

	priv.pretty = pretty;
	priv.remaining = mowgli_patricia_size(n->v_object);
	priv.str = str;
	mowgli_patricia_foreach(n->v_object, serialize_object_cb, &priv);

	serialize_pretty_indent(str, pretty - 1);
	mowgli_string_append_char(str, '}');
}

typedef void (*serializer_t)(mowgli_json_t*,mowgli_string_t*,int);
static serializer_t serializers[] =
{
	[MOWGLI_JSON_TAG_BOOLEAN] = serialize_boolean,
	[MOWGLI_JSON_TAG_INTEGER] = serialize_int,
	[MOWGLI_JSON_TAG_FLOAT] = serialize_float,
	[MOWGLI_JSON_TAG_STRING] = serialize_string,
	[MOWGLI_JSON_TAG_ARRAY] = serialize_array,
	[MOWGLI_JSON_TAG_OBJECT] = serialize_object,
};

void mowgli_json_serialize(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	if (n && serializers[n->tag])
		serializers[n->tag](n, str, pretty);
	else
		mowgli_string_append(str, "null", 4);
}

/*
 * 4. DESERIALIZER (A.K.A. PARSER, ETC.)
 */

/* LL(1) parser format:

   Terminal symbols: { } [ ] : , STR NUM ID

   Rule table:
       0. [invalid]

       1. <json-document> := <object>
       2. <json-document> := <array>
      
       3.         <value> := <object> 
       4.         <value> := <array> 
       5.         <value> := STR 
       6.         <value> := NUM 
       7.         <value> := ID
      
       8.        <object> := { <obj-body>
       9.      <obj-body> := } 
      10.      <obj-body> := <obj-elems>
      11.     <obj-elems> := <obj-elem> <obj-tail>
      12.      <obj-tail> := , <obj-elems> 
      13.      <obj-tail> := } 
      14.      <obj-elem> := STR : <value>
      
      15.         <array> := [ <arr-body>
      16.      <arr-body> := ]
      17.      <arr-body> := <arr-elems>
      18.     <arr-elems> := <value> <arr-tail>
      19.      <arr-tail> := , <arr-elems>
      20.      <arr-tail> := ]
   
   Transition table:
                   {    }    [    ]    :    ,  STR  NUM   ID
                 ---  ---  ---  ---  ---  ---  ---  ---  ---
   json-document   1         2
           value   3         4                   5    6    7
          object   8
        obj-body        9                       10
       obj-elems                                11
        obj-tail       13                  12
        obj-elem                                14
           array            15
        arr-body  18        18   16             18   18   18
       arr-elems   3         4                   5    6    7
        arr-tail                 20        19
   
   These two tables are effectively a program for an LL(1) parser. The
   hard work has been done. The remaining steps are to attach appropriate
   actions to the rules above, and to implement the LL(1) parsing
   algorithm.
 */

enum ll_sym {
	SYM_NONE,

	TS_BEGIN_OBJECT,  /* { */
	TS_END_OBJECT,    /* } */
	TS_BEGIN_ARRAY,   /* [ */
	TS_END_ARRAY,     /* ] */
	TS_NAME_SEP,      /* : */
	TS_VALUE_SEP,     /* , */
	TS_STRING,
	TS_NUMBER,
	TS_IDENTIFIER,

	NTS_JSON_DOCUMENT,
	NTS_VALUE,
	NTS_OBJECT,
	NTS_OBJ_BODY,
	NTS_OBJ_ELEMS,
	NTS_OBJ_TAIL,
	NTS_OBJ_ELEM,
	NTS_ARRAY,
	NTS_ARR_BODY,
	NTS_ARR_ELEMS,
	NTS_ARR_TAIL,

	SYM_COUNT
};

/* Human-readable versions of symbols used in errors, etc. */
static char *ll_sym_name[] = {
	[SYM_NONE] = "(none)",

	[TS_BEGIN_OBJECT] = "'{'",
	[TS_END_OBJECT] = "'}'",
	[TS_BEGIN_ARRAY] = "'['",
	[TS_END_ARRAY] = "']'",
	[TS_NAME_SEP] = "':'",
	[TS_VALUE_SEP] = "','",
	[TS_STRING] = "string",
	[TS_NUMBER] = "number",
	[TS_IDENTIFIER] = "identifier",

	[NTS_JSON_DOCUMENT] = "json-document",
	[NTS_VALUE] = "value",
	[NTS_OBJECT] = "object",
	[NTS_OBJ_BODY] = "object body",
	[NTS_OBJ_ELEMS] = "object elements",
	[NTS_OBJ_TAIL] = "object tail",
	[NTS_OBJ_ELEM] = "object element",

	[NTS_ARRAY] = "array",
	[NTS_ARR_BODY] = "array body",
	[NTS_ARR_ELEMS] = "array elements",
	[NTS_ARR_TAIL] = "array tail",

	[SYM_COUNT] = "(none)",
};

/* The LL(1) parser table. */
static unsigned char ll_table[SYM_COUNT][SYM_COUNT] = {
	[NTS_JSON_DOCUMENT][TS_BEGIN_OBJECT] = 1,
	[NTS_JSON_DOCUMENT][TS_BEGIN_ARRAY] = 2,

	[NTS_VALUE][TS_BEGIN_OBJECT] = 3,
	[NTS_VALUE][TS_BEGIN_ARRAY] = 4,
	[NTS_VALUE][TS_STRING] = 5,
	[NTS_VALUE][TS_NUMBER] = 6,
	[NTS_VALUE][TS_IDENTIFIER] = 7,

	[NTS_OBJECT][TS_BEGIN_OBJECT] = 8,
	[NTS_OBJ_BODY][TS_END_OBJECT] = 9,
	[NTS_OBJ_BODY][TS_STRING] = 10,
	[NTS_OBJ_ELEMS][TS_STRING] = 11,
	[NTS_OBJ_TAIL][TS_END_OBJECT] = 13,
	[NTS_OBJ_TAIL][TS_VALUE_SEP] = 12,
	[NTS_OBJ_ELEM][TS_STRING] = 14,

	[NTS_ARRAY][TS_BEGIN_ARRAY] = 15,
	[NTS_ARR_BODY][TS_BEGIN_OBJECT] = 18,
	[NTS_ARR_BODY][TS_BEGIN_ARRAY] = 18,
	[NTS_ARR_BODY][TS_END_ARRAY] = 16,
	[NTS_ARR_BODY][TS_STRING] = 18,
	[NTS_ARR_BODY][TS_NUMBER] = 18,
	[NTS_ARR_BODY][TS_IDENTIFIER] = 18,
	[NTS_ARR_ELEMS][TS_BEGIN_OBJECT] = 3,
	[NTS_ARR_ELEMS][TS_BEGIN_ARRAY] = 4,
	[NTS_ARR_ELEMS][TS_STRING] = 5,
	[NTS_ARR_ELEMS][TS_NUMBER] = 6,
	[NTS_ARR_ELEMS][TS_IDENTIFIER] = 7,
	[NTS_ARR_TAIL][TS_END_ARRAY] = 20,
	[NTS_ARR_TAIL][TS_VALUE_SEP] = 19,
};

/* The LL(1) rule table */
static enum ll_sym ll_rules[][3] = {
	{ 0 }, /* 0 */

	{ NTS_OBJECT },
	{ NTS_ARRAY },

	{ NTS_OBJECT },
	{ NTS_ARRAY },
	{ TS_STRING }, /* 5 */
	{ TS_NUMBER },
	{ TS_IDENTIFIER },

	{ TS_BEGIN_OBJECT, NTS_OBJ_BODY },
	{ TS_END_OBJECT },
	{ NTS_OBJ_ELEMS }, /* 10 */
	{ NTS_OBJ_ELEM, NTS_OBJ_TAIL },
	{ TS_VALUE_SEP, NTS_OBJ_ELEMS },
	{ TS_END_OBJECT },
	{ TS_STRING, TS_NAME_SEP, NTS_VALUE },

	{ TS_BEGIN_ARRAY, NTS_ARR_BODY }, /* 15 */
	{ TS_END_ARRAY },
	{ NTS_ARR_ELEMS },
	{ NTS_VALUE, NTS_ARR_TAIL },
	{ TS_VALUE_SEP, NTS_ARR_TAIL },
	{ TS_END_ARRAY }, /* 20 */
};
