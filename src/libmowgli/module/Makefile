include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_MODULE}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_MODULE}

SRCS = loader_${LIBMOWGLI_OS}.c

INCLUDES = module.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/module

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

