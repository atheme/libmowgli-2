include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_EXT}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_EXT}

SRCS = confparse.c 			\
       error_backtrace.c		\
       getopt_long.c			\
       global_storage.c 		\
       program_opts.c			\
       proctitle.c			\
       json.c

INCLUDES = confparse.h			\
	   error_backtrace.h		\
	   getopt_long.h		\
	   global_storage.h		\
	   program_opts.h		\
	   proctitle.h			\
	   json.h			\
	   json-inline.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/ext

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

