include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.atomic.a

SRCS = spinlock.c		\

INCLUDES = spinlock.h		\

includesubdir = $(PACKAGE)/atomic

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
