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

#ifndef __PS_STATUS_H__
#define __PS_STATUS_H__

extern bool mowgli_proctitle_update;

extern char **mowgli_argv;
extern int mowgli_argc;

extern char **mowgli_proctitle_init(int argc, char **argv);

extern void mowgli_proctitle_set(const char *fmt, ...);

extern const char *mowgli_proctitle_get(int *displen);

#endif /* __PS_STATUS_H__ */
