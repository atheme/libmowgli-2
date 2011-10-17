include ../../../../extra.mk

STATIC_PIC_LIB_NOINST = mowgli.platform.posix.a
LIB_MAJOR = 2
LIB_MINOR = 0
DISTCLEAN = mowgli_config.h

SRCS = thread.c

include ../../../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -I../.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
