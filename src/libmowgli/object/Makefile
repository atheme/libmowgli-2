include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_OBJECT}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_OBJECT}

SRCS = object.c		\
       class.c		\
       message.c	\
       metadata.c

INCLUDES = object.h	\
	   class.h	\
	   message.h	\
	   metadata.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/object

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

