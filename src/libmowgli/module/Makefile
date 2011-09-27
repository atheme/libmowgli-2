include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.module.a

SRCS = ${MOWGLI_MODULE}

INCLUDES = module.h		\

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}

includesubdir += /module
