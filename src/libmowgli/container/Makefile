include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_CONTAINER}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_CONTAINER}

SRCS = dictionary.c	\
       list.c 		\
       queue.c		\
       index.c		\
       patricia.c

INCLUDES = dictionary.h	\
	   list.h	\
	   queue.h	\
	   index.h	\
	   patricia.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/container

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

