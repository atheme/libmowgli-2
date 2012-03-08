include ../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.ext.lib.a
STATIC_LIB_NOINST = mowgli.ext.a

SRCS = confparse.c 			\
       error_backtrace.c		\
       getopt_long.c			\
       global_storage.c 		\
       program_opts.c

INCLUDES = confparse.h			\
	   error_backtrace.h		\
	   getopt_long.h		\
	   global_storage.h		\
	   program_opts.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE)/ext

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

