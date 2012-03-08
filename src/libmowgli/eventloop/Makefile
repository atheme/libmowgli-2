include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.eventloop.lib.a
STATIC_LIB_NOINST = mowgli.eventloop.a

SRCS = eventloop.c helper.c pollable.c timer.c null_pollops.c poll_pollops.c epoll_pollops.c kqueue_pollops.c select_pollops.c linebuf.c

INCLUDES = eventloop.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/eventloop

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

