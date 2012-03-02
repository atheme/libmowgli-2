include ../../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.platform.win32.lib.a
STATIC_LIB_NOINST = mowgli.platform.win32.a

SRCS = gettimeofday.c pipe.c setenv.c

INCLUDES = win32_stdinc.h

include ../../../../buildsys.mk

CPPFLAGS += -I. -I.. -I../.. -I../../../.. -DMOWGLI_CORE

