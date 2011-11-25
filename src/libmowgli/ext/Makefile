include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.ext.a

SRCS = confparse.c 			\
       error_backtrace.c		\
       global_storage.c 		\

INCLUDES = confparse.h			\
	   error_backtrace.h		\
	   global_storage.h		\

includesubdir = $(PACKAGE)/ext

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
