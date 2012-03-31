include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_COROUTINE}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_COROUTINE}

SRCS = coroutine.c

INCLUDES := ${SRCS:.c=.h}

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/coroutine

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

