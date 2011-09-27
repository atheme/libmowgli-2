include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.container.a

SRCS = list.c 		\
       queue.c		\
       index.c		\
       patricia.c

INCLUDES = list.h	\
	   queue.h	\
	   index.h	\
	   patricia.h

includesubdir = $(PACKAGE)/container

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
