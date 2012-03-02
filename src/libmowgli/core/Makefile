include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.core.lib.a
STATIC_LIB_NOINST = mowgli.core.a

SRCS = init.c			\
       alloc.c			\
       allocation_policy.c	\
       allocator.c		\
       heap.c			\
       logger.c			\
       mowgli_string.c		\
       process.c

INCLUDES = init.h		\
	   alloc.h		\
	   allocation_policy.h	\
	   allocator.h		\
	   assert.h		\
	   exception.h		\
	   heap.h		\
	   iterator.h		\
	   logger.h		\
	   mowgli_string.h	\
	   stdinc.h		\
	   process.h

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

