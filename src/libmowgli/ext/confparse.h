/*
 * Copyright (C) 2005-2008 William Pitcock, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Config file parser.
 *
 */

#ifndef CONFPARSE_H
#define CONFPARSE_H

typedef struct _mowgli_configfile mowgli_config_file_t;
typedef struct _mowgli_configentry mowgli_config_file_entry_t;

struct _mowgli_configfile
{
	char *filename;
	mowgli_config_file_entry_t *entries;
	mowgli_config_file_t *next;
	int curline;
	char *mem;
};

struct _mowgli_configentry
{
	mowgli_config_file_t *fileptr;

	int varlinenum;
	char *varname;
	char *vardata;
	int sectlinenum;/* line containing closing brace */

	mowgli_config_file_entry_t *entries;
	mowgli_config_file_entry_t *prevlevel;
	mowgli_config_file_entry_t *next;
};

/* confp.c */
extern void mowgli_config_file_free(mowgli_config_file_t *cfptr);
extern mowgli_config_file_t *mowgli_config_file_load(const char *filename);

#endif

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs ts=8 sw=8 noexpandtab
 */
