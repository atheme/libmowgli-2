include ../../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.platform.win32.a
LIB_MAJOR = 2
LIB_MINOR = 0
DISTCLEAN = mowgli_config.h

SRCS = gettimeofday.c pipe.c setenv.c

INCLUDES = win32_stdinc.h

includesubdir = $(PACKAGE)/platform/win32

include ../../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -I../.. -I../../../.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
