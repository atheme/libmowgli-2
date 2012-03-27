include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_CONCURRENT}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_CONCURRENT}

SRCS = atomic.c future.c

INCLUDES := ${SRCS:.c=.h}

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/concurrent

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

