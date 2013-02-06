include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_CORE}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_CORE}

SRCS = bootstrap.c			\
       alloc.c			\
       allocation_policy.c	\
       allocator.c		\
       heap.c			\
       logger.c			\
       mowgli_string.c		\
       process.c

INCLUDES = bootstrap.h		\
	   alloc.h		\
	   allocation_policy.h	\
	   allocator.h		\
	   assert.h		\
	   heap.h		\
	   iterator.h		\
	   logger.h		\
	   mowgli_string.h	\
	   stdinc.h		\
	   process.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/core

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

