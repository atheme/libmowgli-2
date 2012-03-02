include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.container.lib.a
STATIC_LIB_NOINST = mowgli.container.a

SRCS = list.c 		\
       queue.c		\
       index.c		\
       patricia.c

INCLUDES = list.h	\
	   queue.h	\
	   index.h	\
	   patricia.h

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

