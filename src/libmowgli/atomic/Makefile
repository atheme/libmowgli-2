include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.atomic.lib.a
STATIC_LIB_NOINST = mowgli.atomic.a

SRCS = spinlock.c

INCLUDES = atomic.h spinlock.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/atomic

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

