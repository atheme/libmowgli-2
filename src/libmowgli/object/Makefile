include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.object.a

SRCS = object.c		\
       class.c		\
       message.c	\
       metadata.c

INCLUDES = object.h	\
	   class.h	\
	   message.h	\
	   metadata.h

includesubdir = $(PACKAGE)/object

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
