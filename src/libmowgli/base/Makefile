include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.base.lib.a
STATIC_LIB_NOINST = mowgli.base.a

SRCS = argstack.c		\
       bitvector.c 		\
       formatter.c		\
       hash.c			\
       hook.c			\
       mempool.c		\
       random.c			\
       mowgli_signal.c

INCLUDES = argstack.h		\
	   bitvector.h		\
	   formatter.h		\
	   hash.h		\
	   hook.h		\
	   mempool.h		\
	   random.h		\
	   mowgli_signal.h

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

