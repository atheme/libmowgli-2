include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.vio.lib.a
STATIC_LIB_NOINST = mowgli.vio.a

SRCS = vio.c vio-openssl.c

INCLUDES = vio.h vio-types.h

includesubdir = $(PACKAGE)/vio

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

