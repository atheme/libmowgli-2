include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.thread.a

SRCS = mutex.c			\
       null_mutexops.c		\
       linux-futex_mutexops.c	\
       posix_mutexops.c		\
       win32_mutexops.c

INCLUDES = thread.h

includesubdir = $(PACKAGE)/thread

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
