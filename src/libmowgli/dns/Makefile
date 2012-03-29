include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_DNS}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_DNS}

SRCS = dns.c			\
       dns_evloop_res.c		\
       dns_evloop_reslib.c	\
       dns_evloop_reslist_win32.c

INCLUDES = dns.h		\
           dns_evloop_res.h	\
           dns_evloop_reslib.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/dns

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

