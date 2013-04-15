/*
 * Code has been calqued from PostgreSQL 9.1 by Elizabeth J. Myers.
 * Below is their copyright header.
 *
 * Do note I've made extensive changes to this code, including adding
 * Linux (and Irix?) prctl support. Also added support for searching for
 * argc/argv.
 */

/*--------------------------------------------------------------------
 * ps_status.c
 *
 * Routines to support changing the ps display of PostgreSQL backends
 * to contain some useful information. Mechanism differs wildly across
 * platforms.
 *
 * src/backend/utils/misc/ps_status.c
 *
 * Copyright (c) 2000-2011, PostgreSQL Global Development Group
 * various details abducted from various places
 ***--------------------------------------------------------------------
 */

#include "mowgli.h"

#ifdef HAVE_SYS_PSTAT_H
# include <sys/pstat.h>		/* for HP-UX */
#endif
#ifdef HAVE_PS_STRINGS
# include <machine/vmparam.h>	/* for old BSD */
# include <sys/exec.h>
#endif
#ifdef MOWGLI_OS_OSX
# include <crt_externs.h>
#endif
#ifdef HAVE_SYS_PRCTL_H
# include <sys/prctl.h>
#endif

#if !defined(MOWGLI_OS_WIN) && !defined(MOWGLI_OS_OSX)
extern char **environ;
#endif

/*
 * Alternative ways of updating ps display:
 *
 * MOWGLI_SETPROC_USE_SETPROCTITLE
 *	   use the function setproctitle(const char *, ...)
 *	   (newer BSD systems)
 * MOWGLI_SETPROC_USE_PSTAT
 *	   use the pstat(PSTAT_SETCMD, )
 *	   (HPUX)
 * MOWGLI_SETPROC_USE_PS_STRINGS
 *	   assign PS_STRINGS->ps_argvstr = "string"
 *	   (some BSD systems)
 * MOWGLI_SETPROC_USE_PRCTL
 *	   use prctl(PR_SET_NAME, ...)
 *	   (Newer Linux and possibly Irix? -- Note some utilities don't use this name)
 * MOWGLI_SETPROC_USE_CHANGE_ARGV
 *	   assign argv[0] = "string"
 *	   (some other BSD systems)
 * MOWGLI_SETPROC_USE_CLOBBER_ARGV
 *	   write over the argv and environment area
 *	   (Old Linux and most SysV-like systems)
 * MOWGLI_SETPROC_USE_WIN32
 *	   push the string out as the name of a Windows event
 * MOWGLI_SETPROC_USE_NONE
 *	   don't update ps display
 *	   (This is the default, as it is safest.)
 */
#if defined(HAVE_SETPROCTITLE)
# define MOWGLI_SETPROC_USE_SETPROCTITLE
#elif defined(PR_SET_NAME) && defined(HAVE_SYS_PRCTL_H)
# define MOWGLI_SETPROC_USE_PRCTL
#elif defined(HAVE_PSTAT) && defined(PSTAT_SETCMD)
# define MOWGLI_SETPROC_USE_PSTAT
#elif defined(HAVE_PS_STRINGS)
# define MOWGLI_SETPROC_USE_PS_STRINGS
#elif (defined(BSD) || defined(__bsdi__) || defined(__hurd__)) && !defined(__darwin__)
# define MOWGLI_SETPROC_USE_CHANGE_ARGV
#elif defined(__linux__) || defined(_AIX) || defined(__sgi) || (defined(sun) && !defined(BSD)) || defined(ultrix) || defined(__ksr__) || defined(__osf__) || defined(__svr4__) || defined(__svr5__) || defined(__darwin__)
# define MOWGLI_SETPROC_USE_CLOBBER_ARGV
#elif defined(WIN32)
# define MOWGLI_SETPROC_USE_WIN32
#else
# define MOWGLI_SETPROC_USE_NONE
#endif

/* Different systems want the buffer padded differently */
#if defined(_AIX) || defined(__linux__) || defined(__svr4__) || defined(__darwin__)
# define PS_PADDING '\0'
#else
# define PS_PADDING ' '
#endif

#ifndef MOWGLI_SETPROC_USE_CLOBBER_ARGV

/* all but one option need a buffer to write their ps line in */
# define PS_BUFFER_SIZE 256
static char ps_buffer[PS_BUFFER_SIZE];
static const size_t ps_buffer_size = PS_BUFFER_SIZE;
#else /* MOWGLI_SETPROC_USE_CLOBBER_ARGV */
static char *ps_buffer;	/* will point to argv area */
static size_t ps_buffer_size;	/* space determined at run time */
#endif /* MOWGLI_SETPROC_USE_CLOBBER_ARGV */

static size_t ps_buffer_fixed_size;
static size_t ps_buffer_cur_len;/* nominal strlen(ps_buffer) */

/* save the original argv[] location here */
static int save_argc;
static char **save_argv;

int mowgli_argc = 0;
char **mowgli_argv = NULL;

/*
 * Save the original argc/argv values.
 * If needed, we make a copy of the original argv[] array to preserve it
 * from being clobbered by subsequent ps_display actions.
 */
char **
mowgli_proctitle_init(int argc, char **argv)
{
	if ((argc == 0) || (argv == NULL))
		save_argc = argc;

	save_argv = argv;

#if defined(MOWGLI_SETPROC_USE_CLOBBER_ARGV)

	/*
	 * If we're going to overwrite the argv area, count the available space.
	 * Also move the environment to make additional room.
	 */
	char *end_of_area = NULL;
	char **new_environ;
	int i;

	/*
	 * check for contiguous argv strings
	 */
	for (i = 0; i < argc; i++)
		if ((i == 0) || (end_of_area + 1 == argv[i]))
			end_of_area = argv[i] + strlen(argv[i]);

	if (end_of_area == NULL)/* probably can't happen? */
	{
		ps_buffer = NULL;
		ps_buffer_size = 0;
		return argv;
	}

	/*
	 * check for contiguous environ strings following argv
	 */
	for (i = 0; environ[i] != NULL; i++)
		if (end_of_area + 1 == environ[i])
			end_of_area = environ[i] + strlen(environ[i]);

	ps_buffer = argv[0];
	ps_buffer_size = end_of_area - argv[0];

	/*
	 * move the environment out of the way
	 */
	new_environ = (char **) mowgli_alloc((i + 1) * sizeof(char *));

	for (i = 0; environ[i] != NULL; i++)
		new_environ[i] = mowgli_strdup(environ[i]);

	new_environ[i] = NULL;
	environ = new_environ;

#endif /* MOWGLI_SETPROC_USE_CLOBBER_ARGV */

#if defined(MOWGLI_SETPROC_USE_CHANGE_ARGV) || defined(MOWGLI_SETPROC_USE_CLOBBER_ARGV)

	/*
	 * If we're going to change the original argv[] then make a copy for
	 * argument parsing purposes.
	 *
	 * (NB: do NOT think to remove the copying of argv[].
	 * On some platforms, getopt() keeps pointers into the argv array, and will
	 * get horribly confused when it is re-called to analyze a subprocess'
	 * argument string if the argv storage has been clobbered meanwhile. Other
	 * platforms have other dependencies on argv[].
	 */

	char **new_argv;
	int j;

	new_argv = (char **) mowgli_alloc((argc + 1) * sizeof(char *));

	for (j = 0; j < argc; j++)
		new_argv[j] = mowgli_strdup(argv[j]);

	new_argv[argc] = NULL;

# ifdef MOWGLI_OS_OSX

	/*
	 * Darwin (and perhaps other NeXT-derived platforms?) has a static
	 * copy of the argv pointer, which we may fix like so:
	 */
	*_NSGetArgv() = new_argv;
# endif

	argv = new_argv;

#endif /* MOWGLI_SETPROC_USE_CHANGE_ARGV or MOWGLI_SETPROC_USE_CLOBBER_ARGV */

	mowgli_argc = argc;
	mowgli_argv = argv;

	return argv;
}

void
mowgli_proctitle_set(const char *fmt, ...)
{
#ifndef MOWGLI_SETPROC_USE_NONE
	va_list va;

# if defined(MOWGLI_SETPROC_USE_CHANGE_ARGV) || defined(MOWGLI_SETPROC_USE_CLOBBER_ARGV)

	if (!save_argv)
		return;

# endif

	va_start(va, fmt);
	vsnprintf(ps_buffer, ps_buffer_size, fmt, va);
	va_end(va);

	return_if_fail(*ps_buffer == '\0');

	ps_buffer_cur_len = ps_buffer_fixed_size = strlen(ps_buffer);

# ifdef MOWGLI_SETPROC_USE_CHANGE_ARGV
	save_argv[0] = ps_buffer;
	save_argv[1] = NULL;
# endif

# ifdef MOWGLI_SETPROC_USE_CLOBBER_ARGV

	for (int i = 1; i < save_argc; i++)
		save_argv[i] = ps_buffer + ps_buffer_size;

	/* Pad unused bytes */
	memset(ps_buffer + ps_buffer_cur_len, PS_PADDING, ps_buffer_size - ps_buffer_cur_len + 1);
# endif

# ifdef MOWGLI_SETPROC_USE_SETPROCTITLE
	setproctitle("%s", ps_buffer);
# endif

# ifdef MOWGLI_SETPROC_USE_PRCTL

	/* Limit us to 16 chars to be safe */
	char procbuf[16];
	mowgli_strlcpy(procbuf, ps_buffer, sizeof(procbuf));
	prctl(PR_SET_NAME, procbuf, 0, 0, 0);
# endif

# ifdef MOWGLI_SETPROC_USE_PSTAT
	union pstun pst;

	pst.pst_command = ps_buffer;
	pstat(PSTAT_SETCMD, pst, ps_buffer_cur_len, 0, 0);
# endif /* MOWGLI_SETPROC_USE_PSTAT */

# ifdef MOWGLI_SETPROC_USE_PS_STRINGS
	PS_STRINGS->ps_nargvstr = 1;
	PS_STRINGS->ps_argvstr = ps_buffer;
# endif /* MOWGLI_SETPROC_USE_PS_STRINGS */

# ifdef MOWGLI_SETPROC_USE_WIN32

	/*
	 * Win32 does not support showing any changed arguments. To make it at
	 * all possible to track which backend is doing what, we create a
	 * named object that can be viewed with for example Process Explorer.
	 */
	static HANDLE ident_handle = INVALID_HANDLE_VALUE;
	char name[PS_BUFFER_SIZE + 32];

	if (ident_handle != INVALID_HANDLE_VALUE)
		CloseHandle(ident_handle);

	sprintf(name, "mowgli_ident(%d): %s", getpid(), ps_buffer);

	ident_handle = CreateEvent(NULL, TRUE, FALSE, name);
# endif /* MOWGLI_SETPROC_USE_WIN32 */
#endif /* not MOWGLI_SETPROC_USE_NONE */
}

/*
 * Returns what's currently in the ps display, in case someone needs
 * it.	Note that only the activity part is returned.  On some platforms
 * the string will not be null-terminated, so return the effective
 * length into *displen.
 */
const char *
mowgli_proctitle_get(int *displen)
{
#ifdef MOWGLI_SETPROC_USE_CLOBBER_ARGV

	/* If ps_buffer is a pointer, it might still be null */
	if (!ps_buffer)
	{
		*displen = 0;
		return "";
	}

#endif

	*displen = (int) (ps_buffer_cur_len - ps_buffer_fixed_size);

	return ps_buffer + ps_buffer_fixed_size;
}
