include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_VIO}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_VIO}

SRCS = vio.c vio-sockets.c vio-openssl.c

INCLUDES = vio.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/vio

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

