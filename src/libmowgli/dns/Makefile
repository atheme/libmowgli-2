include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_DNS}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_DNS}

SRCS = dns.c	\
       res.c	\
       reslib.c

INCLUDES = dns.h	\
           res.h	\
           reslib.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/dns

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

