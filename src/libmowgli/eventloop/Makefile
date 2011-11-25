include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.eventloop.a

SRCS = timer.c

INCLUDES = eventloop.h

includesubdir = $(PACKAGE)/eventloop

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
