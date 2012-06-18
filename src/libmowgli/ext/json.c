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

static void serialize_pretty_break(mowgli_string_t *str, int pretty)
{
	int i;

	if (pretty < 1)
		return;

	mowgli_string_append_char(str, '\n');

	for (i=0; i<pretty-1; i++)
		mowgli_string_append(str, TAB_STRING, TAB_LEN);
}

static int serialize_pretty_increment(int pretty)
{
	return (pretty ? 0 : pretty + 1);
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

static void serialize_string_data(const char *p, size_t len, mowgli_string_t *str)
{
	unsigned i;
	unsigned char c;

	mowgli_string_append_char(str, '"');

	for (i=0; i<len; i++) {
		c = p[i];

		if (c < 0x20 || c > 0x7f) {
			mowgli_string_append_char(str, '\\');

			switch (c) {
			case '\\': mowgli_string_append_char(str, '\\'); break;
			//case '/': mowgli_string_append_char(str, '/'); break;
			case '\b': mowgli_string_append_char(str, 'b'); break;
			case '\f': mowgli_string_append_char(str, 'f'); break;
			case '\n': mowgli_string_append_char(str, 'n'); break;
			case '\r': mowgli_string_append_char(str, 'r'); break;
			case '\t': mowgli_string_append_char(str, 't'); break;
			default:
				/* TODO: fix this... */
				mowgli_string_append_char(str, 'u');
				mowgli_string_append_char(str, '0');
				mowgli_string_append_char(str, '0');
				mowgli_string_append_char(str, (c >> 4) & 0xf);
				mowgli_string_append_char(str, (c >> 0) & 0xf);
			}

		} else {
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

	MOWGLI_LIST_FOREACH(cur, n->v_array->head) {
		mowgli_json_serialize(cur->data, str, serialize_pretty_increment(pretty));

		if (cur->next != NULL)
			mowgli_string_append_char(str, ',');
		serialize_pretty_break(str, pretty);
	}

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

	serialize_string_data(key, strlen(key), priv->str);
	mowgli_string_append_char(priv->str, ':');
	if (priv->pretty)
		mowgli_string_append_char(priv->str, ' ');

	mowgli_json_serialize(data, priv->str, priv->pretty + 1);

	if (priv->remaining)
		mowgli_string_append_char(priv->str, ',');
	serialize_pretty_break(priv->str, priv->pretty);

	return 0;
}

static void serialize_object(mowgli_json_t *n, mowgli_string_t *str, int pretty)
{
	struct serialize_object_priv priv;

	mowgli_string_append_char(str, '{');

	priv.pretty = pretty + 1;
	priv.remaining = mowgli_patricia_size(n->v_object);
	priv.str = str;
	mowgli_patricia_foreach(n->v_object, serialize_object_cb, &priv);

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
