include ../../../extra.mk

STATIC_PIC_LIB_NOINST = ${LIBMOWGLI_SHARED_PLATFORM}
STATIC_LIB_NOINST = ${LIBMOWGLI_STATIC_PLATFORM}

SUBDIRS = win32

SRCS = cacheline.c

INCLUDES = attributes.h cacheline.h constructor.h machine.h

include ../../../buildsys.mk

includesubdir = $(PACKAGE_NAME)/platform

CPPFLAGS += -I. -I.. -I../../.. -DMOWGLI_CORE

