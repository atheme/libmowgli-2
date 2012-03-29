include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_VIO}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_VIO}

SRCS = vio.c vio_sockets.c vio_openssl.c

INCLUDES = vio.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/vio

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

