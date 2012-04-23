include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_DNS}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_DNS}

SRCS = dns.c			\
       evloop_res.c		\
       evloop_reslib.c	\
       evloop_reslist_win32.c

INCLUDES = dns.h		\
           evloop_res.h	\
           evloop_reslib.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/dns

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

