include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_THREAD}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_THREAD}

SRCS = mutex.c			\
       null_mutexops.c		\
       posix_mutexops.c		\
       win32_mutexops.c

INCLUDES = thread.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/thread

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

