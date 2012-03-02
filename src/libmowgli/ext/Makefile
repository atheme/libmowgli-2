include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.ext.a

SRCS = confparse.c 			\
       error_backtrace.c		\
       getopt_long.c			\
       global_storage.c 		\
       program_opts.c			\

INCLUDES = confparse.h			\
	   error_backtrace.h		\
	   getopt_long.h		\
	   global_storage.h		\
	   program_opts.h		\

includesubdir = $(PACKAGE)/ext

include ../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -I../../.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
