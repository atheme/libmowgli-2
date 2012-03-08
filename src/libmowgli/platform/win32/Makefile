include ../../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.platform.win32.lib.a
STATIC_LIB_NOINST = mowgli.platform.win32.a

SRCS = fork.c gettimeofday.c pipe.c setenv.c

INCLUDES = win32_stdinc.h

include ../../../../buildsys.mk

includesubdir = $(PACKAGE)/platform/win32

CPPFLAGS += -I. -I.. -I../.. -I../../../.. -DMOWGLI_CORE

