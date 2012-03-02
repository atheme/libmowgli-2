include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.vio.a

SRCS = vio.c vio_openssl.c

INCLUDES = vio.h vio-types.h

includesubdir = $(PACKAGE)/vio

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -I../../.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
