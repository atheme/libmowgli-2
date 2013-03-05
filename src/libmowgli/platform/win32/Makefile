include ../../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_PLATFORM_WIN32}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_PLATFORM_WIN32}

SRCS = fork.c inet.c pipe.c socketpair.c setenv.c

INCLUDES = win32_stdinc.h

include ../../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/platform/win32

CPPFLAGS += -I. -I.. -I../.. -I../../../.. -DMOWGLI_CORE

