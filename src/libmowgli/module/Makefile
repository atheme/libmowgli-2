include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.module.lib.a
STATIC_LIB_NOINST = mowgli.module.a

SRCS = loader_${LIBMOWGLI_OS}.c

INCLUDES = module.h

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

