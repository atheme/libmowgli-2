include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.object.lib.a
STATIC_LIB_NOINST = mowgli.object.a

SRCS = object.c		\
       class.c		\
       message.c	\
       metadata.c

INCLUDES = object.h	\
	   class.h	\
	   message.h	\
	   metadata.h

include ../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

