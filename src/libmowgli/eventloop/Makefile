include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_EVENTLOOP}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_EVENTLOOP}

SRCS = eventloop.c helper.c pollable.c timer.c null_pollops.c poll_pollops.c epoll_pollops.c kqueue_pollops.c qnx_pollops.c ports_pollops.c select_pollops.c windows_pollops.c

INCLUDES = eventloop.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/eventloop

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE
