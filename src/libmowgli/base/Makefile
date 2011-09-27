include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.base.a

SRCS = random.c			\

INCLUDES = random.h		\

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}

includesubdir += /base
