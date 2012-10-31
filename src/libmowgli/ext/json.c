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

#define LL_STACK_SIZE 128

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

/* Debugging tool to recursively dump reference counts for an object
   and any children it may have.  This function will never be called
   outside of development, so is commented here.
void dump_refs(mowgli_json_t *n)
{
	char *s = "unk";
	mowgli_patricia_iteration_state_t st;

	switch (n->tag) {
	case MOWGLI_JSON_TAG_NULL: s = "null"; break;
	case MOWGLI_JSON_TAG_BOOLEAN: s = "boolean"; break;
	case MOWGLI_JSON_TAG_INTEGER: s = "integer"; break;
	case MOWGLI_JSON_TAG_FLOAT: s = "float"; break;
	case MOWGLI_JSON_TAG_STRING: s = "string"; break;
	case MOWGLI_JSON_TAG_ARRAY: s = "array"; break;
	case MOWGLI_JSON_TAG_OBJECT: s = "object"; break;
	}

	if (n->refcount == JSON_REFCOUNT_CONSTANT)
		printf("- %s\n", s);
	else
		printf("%d %s\n", n->refcount, s);

	if (n->tag == MOWGLI_JSON_TAG_ARRAY) {
		mowgli_node_t *cur;

		MOWGLI_LIST_FOREACH(cur, n->v_array->head) {
			dump_refs(cur->data);
		}
	} else if (n->tag == MOWGLI_JSON_TAG_OBJECT) {
		mowgli_json_t *cur;

		mowgli_patricia_foreach_start(n->v_object, &st);
		while ((cur = mowgli_patricia_foreach_cur(n->v_object, &st)) != NULL) {
			dump_refs(cur);
			mowgli_patricia_foreach_next(n->v_object, &st);
		}
	}
}
*/

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

			/* XXX: \u output does not do UTF-8 */
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
        arr-body  17        17   16             17   17   17
       arr-elems  18        18                  18   18   18
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

struct ll_token {
	enum ll_sym sym;
	mowgli_json_t *val;
};

#define ERRBUFSIZE 128

/* typedef'd to mowgli_json_parse_t in json.h */
struct _mowgli_json_parse_t {
	/* output queue */
	mowgli_list_t *out;

	/* error buffer */
	char error[ERRBUFSIZE];

	/* parser */
	mowgli_list_t *build;
	enum ll_sym stack[LL_STACK_SIZE];
	unsigned top;

	/* lexer */
	mowgli_string_t *buf;
	enum {
		LEX_LIMBO,
		LEX_STRING,
		LEX_STRING_ESC,
		LEX_STRING_ESC_U,
		LEX_NUMBER,
		LEX_IDENTIFIER
	} lex;
	unsigned lex_u;
};

typedef void ll_action_t(mowgli_json_parse_t*, struct ll_token *tok);

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
	[NTS_ARR_BODY][TS_BEGIN_OBJECT] = 17,
	[NTS_ARR_BODY][TS_BEGIN_ARRAY] = 17,
	[NTS_ARR_BODY][TS_END_ARRAY] = 16,
	[NTS_ARR_BODY][TS_STRING] = 17,
	[NTS_ARR_BODY][TS_NUMBER] = 17,
	[NTS_ARR_BODY][TS_IDENTIFIER] = 17,
	[NTS_ARR_ELEMS][TS_BEGIN_OBJECT] = 18,
	[NTS_ARR_ELEMS][TS_BEGIN_ARRAY] = 18,
	[NTS_ARR_ELEMS][TS_STRING] = 18,
	[NTS_ARR_ELEMS][TS_NUMBER] = 18,
	[NTS_ARR_ELEMS][TS_IDENTIFIER] = 18,
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
	{ TS_VALUE_SEP, NTS_ARR_ELEMS },
	{ TS_END_ARRAY }, /* 20 */
};

static struct ll_token *ll_token_alloc(enum ll_sym sym, mowgli_json_t *val)
{
	struct ll_token *tok;

	tok = mowgli_alloc(sizeof(*tok));
	tok->sym = sym;
	tok->val = val;

	return tok;
}

static void ll_token_free(struct ll_token *tok)
{
	mowgli_json_decref(tok->val);
	mowgli_free(tok);
}

static bool parse_out_empty(mowgli_json_parse_t *parse)
{
	return MOWGLI_LIST_LENGTH(parse->out) == 0;
}

static void parse_out_enqueue(mowgli_json_parse_t *parse, mowgli_json_t *val)
{
	mowgli_node_add(val, mowgli_node_create(), parse->out);
}

static mowgli_json_t *parse_out_dequeue(mowgli_json_parse_t *parse)
{
	mowgli_json_t *n;
	mowgli_node_t *head;

	if (MOWGLI_LIST_LENGTH(parse->out) == 0)
		return NULL;

	head = parse->out->head;
	if (head == NULL)
		return NULL;

	n = head->data;
	mowgli_node_delete(head, parse->out);
	mowgli_node_free(head);

	return n;
}

static void parse_error(mowgli_json_parse_t *parse, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vsnprintf(parse->error, ERRBUFSIZE, fmt, va);
	va_end(va);

	/* TODO: shut down everything, yadda yadda */
}

static void ll_build_push(mowgli_json_parse_t *parse, mowgli_json_t *val)
{
	mowgli_node_add_head(val, mowgli_node_create(), parse->build);
}

static mowgli_json_t *ll_build_pop(mowgli_json_parse_t *parse)
{
	mowgli_json_t *n;
	mowgli_node_t *head;

	if (MOWGLI_LIST_LENGTH(parse->build) == 0)
		return NULL;

	head = parse->build->head;
	if (head == NULL) /* shouldn't happen... */
		return NULL;

	n = head->data;
	mowgli_node_delete(head, parse->build);
	mowgli_node_free(head);

	return n;
}

static mowgli_json_t obj_start_marker = {
	.refcount = JSON_REFCOUNT_CONSTANT,
};
static mowgli_json_t arr_start_marker = {
	.refcount = JSON_REFCOUNT_CONSTANT,
};

static void ll_act_echo(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	ll_build_push(parse, mowgli_json_incref(tok->val));
}

static void ll_act_obj_start(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	ll_build_push(parse, &obj_start_marker);
}

static void ll_act_obj_end(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	mowgli_json_t *obj;
	mowgli_json_t *key, *val;

	obj = mowgli_json_incref(mowgli_json_create_object());

	for (;;) {
		val = ll_build_pop(parse);
		if (val == &obj_start_marker)
			break;

		key = ll_build_pop(parse);
		if (key == &obj_start_marker)
			break; /* should never happen, but in case */
		if (MOWGLI_JSON_TAG(key) != MOWGLI_JSON_TAG_STRING)
			break; /* should also never happen */

		mowgli_json_object_add(obj, MOWGLI_JSON_STRING_STR(key), val);
		mowgli_json_decref(key);
		mowgli_json_decref(val);
	}

	ll_build_push(parse, obj);
}

static void ll_act_arr_start(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	ll_build_push(parse, &arr_start_marker);
}

static void ll_act_arr_end(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	mowgli_json_t *arr;
	mowgli_json_t *val;

	arr = mowgli_json_incref(mowgli_json_create_array());

	for (;;) {
		val = ll_build_pop(parse);
		if (val == &arr_start_marker)
			break;

		mowgli_json_array_add_head(arr, val);
		mowgli_json_decref(val);
	}

	ll_build_push(parse, arr);
}

static ll_action_t *ll_action[] = {
	NULL, /* 0 */

	NULL,
	NULL,

	NULL,
	NULL,
	ll_act_echo, /* 5 */
	ll_act_echo,
	ll_act_echo,

	ll_act_obj_start,
	ll_act_obj_end,
	NULL, /* 10 */
	NULL,
	NULL,
	ll_act_obj_end,
	ll_act_echo,

	ll_act_arr_start, /* 15 */
	ll_act_arr_end,
	NULL,
	NULL,
	NULL,
	ll_act_arr_end, /* 20 */
};

static void ll_push(mowgli_json_parse_t *parse, enum ll_sym sym)
{
	parse->stack[parse->top++] = sym;
}

static enum ll_sym ll_pop(mowgli_json_parse_t *parse)
{
	if (parse->top <= 0)
		return SYM_NONE;

	return parse->stack[--parse->top];
}

static bool ll_stack_empty(mowgli_json_parse_t *parse)
{
	return parse->top == 0;
}

static void ll_cycle(mowgli_json_parse_t *parse)
{
	mowgli_json_t *n;

	n = ll_build_pop(parse);
	if (n == NULL)
		return; /* should not happen */

	parse_out_enqueue(parse, n);
}

static void ll_parse(mowgli_json_parse_t *parse, struct ll_token *tok)
{
	enum ll_sym top, sym;
	int rule, i;

	for (;;) {
		if (ll_stack_empty(parse)) {
			parse_error(parse, "Unexpected %s after JSON input", ll_sym_name[tok->sym]);
			break;
		}

		top = ll_pop(parse);
		if (top == tok->sym) {
			/* perfect! */

			if (ll_stack_empty(parse))
				ll_cycle(parse);

			break;
		}

		rule = ll_table[top][tok->sym];
		if (rule == 0) {
			parse_error(parse, "Expected %s, got %s", ll_sym_name[top],
						ll_sym_name[tok->sym]);
			break;
		}

		if (ll_action[rule] != NULL)
			ll_action[rule](parse, tok);

		for (i=2; i>=0; i--) {
			sym = ll_rules[rule][i];
			if (sym != SYM_NONE)
				ll_push(parse, sym);
		}
	}

	ll_token_free(tok);
}

static void lex_easy(mowgli_json_parse_t *parse, enum ll_sym sym)
{
	ll_parse(parse, ll_token_alloc(sym, NULL));
}

static void lex_append(mowgli_json_parse_t *parse, char c)
{
	mowgli_string_append_char(parse->buf, c);
}

static mowgli_json_t *lex_string_scan(char *s, size_t n)
{
	mowgli_json_t *val;
	mowgli_string_t *str;
	char ubuf[5];
	char *end = s + n;

	val = mowgli_json_incref(mowgli_json_create_string(""));
	str = val->v_string;

	ubuf[4] = '\0'; /* always */

	while (s < end) {
		if (*s == '\\') {
			/* Should this be moved to a separate function? */
			s++;
			switch (*s) {
			case '\"':
			case '\\':
			case '/':
				mowgli_string_append_char(str, *s);
				break;

			case 'b': mowgli_string_append_char(str, 0x8); break;
			case 'f': mowgli_string_append_char(str, 0xc); break;
			case 'n': mowgli_string_append_char(str, 0xa); break;
			case 'r': mowgli_string_append_char(str, 0xd); break;
			case 't': mowgli_string_append_char(str, 0x9); break;

			/* XXX: this is not the right way to parse \u */
			case 'u':
				s++;
				if (end - s < 4) {
					/* error */
				} else {
					memcpy(ubuf, s, 4);
					mowgli_string_append_char(str, strtol(ubuf, NULL, 16) & 0xff);
				}
				s+=3; /* +3 points to last char */
				break;

			default: /* aww */
				break;
			}
		} else {
			mowgli_string_append_char(str, *s);
		}

		s++;
	}

	return val;
}

static void lex_tokenize(mowgli_json_parse_t *parse)
{
	char *s = parse->buf->str;
	size_t n = parse->buf->pos;
	enum ll_sym sym = SYM_NONE;
	mowgli_json_t *val = NULL;

	switch (parse->lex) {
	case LEX_STRING:
		sym = TS_STRING;
		val = lex_string_scan(s, n);
		break;

	case LEX_NUMBER:
		if (strchr(s, '.') || strchr(s, 'e')) {
			val = mowgli_json_incref(mowgli_json_create_float(strtod(s, NULL)));
		} else {
			val = mowgli_json_incref(mowgli_json_create_integer(strtol(s, NULL, 0)));
		}
		sym = TS_NUMBER;
		break;

	case LEX_IDENTIFIER:
		sym = TS_IDENTIFIER;
		if (!strcmp(s, "null")) {
			val = mowgli_json_null;
		} else if (!strcmp(s, "true")) {
			val = mowgli_json_true;
		} else if (!strcmp(s, "false")) {
			val = mowgli_json_false;
		} else {
			val = mowgli_json_null;
			/* error condition! */
		}
		break;

	default:
		/* we should not be tokenizing here! */
		break;
	}

	mowgli_string_reset(parse->buf);
	parse->lex = LEX_LIMBO;

	ll_parse(parse, ll_token_alloc(sym, val));
}

/* lex_char returns true if it wants the char to be sent through again */
static bool lex_char(mowgli_json_parse_t *parse, char c)
{
	switch (parse->lex) {
	case LEX_LIMBO:
		/* the easy ones */
		switch (c) {
		case '{': lex_easy(parse, TS_BEGIN_OBJECT); return false;
		case '}': lex_easy(parse, TS_END_OBJECT); return false;
		case '[': lex_easy(parse, TS_BEGIN_ARRAY); return false;
		case ']': lex_easy(parse, TS_END_ARRAY); return false;
		case ':': lex_easy(parse, TS_NAME_SEP); return false;
		case ',': lex_easy(parse, TS_VALUE_SEP); return false;
		}

		if (c == '-' || c == '.' || isdigit(c)) {
			parse->lex = LEX_NUMBER;
			return true;
		} else if (isalpha(c)) {
			parse->lex = LEX_IDENTIFIER;
			return true;
		} else if (c == '"') {
			parse->lex = LEX_STRING;
			return false;
		} else if (isspace(c)) {
			return false;
		}

		parse_error(parse, "Cannot process character '%c' in input stream", c);

		return false;

	case LEX_STRING:

		if (c == '\\') {
			lex_append(parse, c);
			parse->lex = LEX_STRING_ESC;
		} else if (c == '"') {
			lex_tokenize(parse);
		} else {
			lex_append(parse, c);
		}

		return false;

	case LEX_STRING_ESC:
		lex_append(parse, c);

		if (c == 'u') {
			parse->lex_u = 0;
			parse->lex = LEX_STRING_ESC_U;
		} else {
			parse->lex = LEX_STRING;
		}

		return false;

	case LEX_STRING_ESC_U:
		lex_append(parse, c);

		parse->lex_u++;
		if (parse->lex_u >= 4)
			parse->lex = LEX_STRING;

		return false;

	case LEX_NUMBER:
		if (c == '-' || c == '.' || isdigit(c) || toupper(c) == 'E') {
			lex_append(parse, c);
			return false;
		} else {
			lex_tokenize(parse);
			return true;
		}

		break;

	case LEX_IDENTIFIER:
		if (isalpha(c)) {
			lex_append(parse, c);
			return false;
		} else {
			lex_tokenize(parse);
			return true;
		}

		break;
	}

	/* This should never happen... */
	parse->lex = LEX_LIMBO;
	return false;
}

mowgli_json_parse_t *mowgli_json_parse_create(void)
{
	mowgli_json_parse_t *parse;

	parse = mowgli_alloc(sizeof(*parse));

	parse->out = mowgli_list_create();
	parse->error[0] = '\0';
	parse->build = mowgli_list_create();
	parse->top = 0;
	parse->buf = mowgli_string_create();
	parse->lex = LEX_LIMBO;

	ll_push(parse, NTS_JSON_DOCUMENT);

	return parse;
}

void mowgli_json_parse_destroy(mowgli_json_parse_t *parse)
{
	mowgli_node_t *n;

	return_if_fail(parse != NULL);

	MOWGLI_LIST_FOREACH(n, parse->out->head)
		mowgli_json_decref(n->data);
	MOWGLI_LIST_FOREACH(n, parse->build->head)
		mowgli_json_decref(n->data);

	mowgli_list_free(parse->out);
	mowgli_list_free(parse->build);
	mowgli_string_destroy(parse->buf);

	mowgli_free(parse);
}

void mowgli_json_parse_data(mowgli_json_parse_t *parse, const char *data, size_t len)
{
	while (len > 0) {
		/* We cannot continue parsing if there's an error! */
		if (mowgli_json_parse_error(parse))
			return;

		while(lex_char(parse, *data));

		data++;
		len--;
	}
}

char *mowgli_json_parse_error(mowgli_json_parse_t *parse)
{
	if (parse->error[0])
		return parse->error;
	return NULL;
}

bool mowgli_json_parse_more(mowgli_json_parse_t *parse)
{
	return !parse_out_empty(parse);
}

mowgli_json_t *mowgli_json_parse_next(mowgli_json_parse_t *parse)
{
	return parse_out_dequeue(parse);
}

mowgli_json_t *mowgli_json_parse_file(const char *path)
{
	mowgli_json_parse_t *parse;
	char *s;
	char buf[512];
	size_t n;
	mowgli_json_t *ret;
	FILE *f;

	f = fopen(path, "r");
	if (f == NULL) {
		mowgli_log("Could not open %s for reading", path);
		return NULL;
	}

	parse = mowgli_json_parse_create();

	s = NULL;
	while (!feof(f) && s == NULL) {
		n = fread(buf, 1, 512, f);
		mowgli_json_parse_data(parse, buf, n);

		s = mowgli_json_parse_error(parse);
	}

	if (s != NULL) {
		mowgli_log("%s: %s", path, s);
		ret = NULL;
	} else {
		ret = mowgli_json_parse_next(parse);
		if (ret == NULL)
			mowgli_log("%s: Incomplete JSON document", path);
	}

	mowgli_json_parse_destroy(parse);

	fclose(f);

	return ret;
}

mowgli_json_t *mowgli_json_parse_string(const char *data)
{
	mowgli_json_parse_t *parse;
	mowgli_json_t *ret;
	char *s;

	parse = mowgli_json_parse_create();

	mowgli_json_parse_data(parse, data, strlen(data));

	if ((s = mowgli_json_parse_error(parse)) != NULL) {
		mowgli_log("%s", s);
		ret = NULL;
	} else {
		ret = mowgli_json_parse_next(parse);
		if (ret == NULL)
			mowgli_log("Incomplete JSON document");
	}

	mowgli_json_parse_destroy(parse);

	return ret;
}
