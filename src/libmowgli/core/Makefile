include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.core.a

SRCS = init.c			\
       alloc.c			\
       allocation_policy.c	\
       allocator.c		\
       heap.c			\
       logger.c			\
       mowgli_string.c

INCLUDES = init.h		\
	   alloc.h		\
	   allocation_policy.h	\
	   allocator.h		\
	   logger.h		\
	   mowgli_string.h \

includesubdir = $(PACKAGE)/core

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
