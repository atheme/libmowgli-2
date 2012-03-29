include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_BASE}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_BASE}

SRCS = argstack.c		\
       bitvector.c 		\
       formatter.c		\
       hash.c			\
       hook.c			\
       memslice.c		\
       random.c			\
       mowgli_signal.c

INCLUDES = argstack.h		\
	   bitvector.h		\
	   formatter.h		\
	   hash.h		\
	   hook.h		\
	   memslice.h		\
	   random.h		\
	   mowgli_signal.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/base

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

