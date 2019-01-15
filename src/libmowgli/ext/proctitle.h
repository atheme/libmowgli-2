/*-------------------------------------------------------------------------
 *
 * ps_status.h
 *
 * Declarations for backend/utils/misc/ps_status.c
 *
 * src/include/utils/ps_status.h
 *
 ***-------------------------------------------------------------------------
 */

#ifndef MOWGLI_SRC_LIBMOWGLI_EXT_PROCTITLE_H_INCLUDE_GUARD
#define MOWGLI_SRC_LIBMOWGLI_EXT_PROCTITLE_H_INCLUDE_GUARD 1

#include "core/stdinc.h"
#include "platform/attributes.h"

extern bool mowgli_proctitle_update;

extern char **mowgli_argv;
extern int mowgli_argc;

extern char **mowgli_proctitle_init(int argc, char **argv);

extern void mowgli_proctitle_set(const char *fmt, ...)
    MOWGLI_FATTR_PRINTF(1, 2);

extern const char *mowgli_proctitle_get(int *displen);

#endif /* MOWGLI_SRC_LIBMOWGLI_EXT_PROCTITLE_H_INCLUDE_GUARD */
