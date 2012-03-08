include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.thread.lib.a
STATIC_LIB_NOINST = mowgli.thread.a

SRCS = mutex.c			\
       null_mutexops.c		\
       linux-futex_mutexops.c	\
       posix_mutexops.c		\
       win32_mutexops.c

INCLUDES = thread.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/thread

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

