include ../../extra.mk

SHARED_LIB = ${LIB_PREFIX}mowgli-2${LIB_SUFFIX}
LIB_MAJOR = 2
LIB_MINOR = 0

SUBDIRS = core platform module object

SRCS = \
       mowgli_argstack.c		\
       mowgli_bitvector.c		\
       mowgli_error_backtrace.c		\
       mowgli_formatter.c		\
       mowgli_global_storage.c		\
       mowgli_hash.c			\
       mowgli_hook.c			\
       mowgli_index.c			\
       mowgli_list.c			\
       mowgli_mempool.c			\
       mowgli_patricia.c		\
       mowgli_queue.c			\
       mowgli_random.c			\
       mowgli_signal.c			\
       mowgli_spinlock.c

INCLUDES = mowgli.h			\
	   mowgli_argstack.h		\
	   mowgli_bitvector.h		\
	   mowgli_config.h		\
	   mowgli_error_backtrace.h	\
	   mowgli_formatter.h		\
	   mowgli_global_storage.h	\
	   mowgli_hash.h		\
	   mowgli_hook.h		\
	   mowgli_index.h		\
	   mowgli_iterator.h		\
	   mowgli_list.h		\
	   mowgli_mempool.h		\
	   mowgli_patricia.h		\
	   mowgli_queue.h		\
	   mowgli_random.h		\
	   mowgli_signal.h		\
	   mowgli_spinlock.h		\
	   mowgli_stdinc.h

include ../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}

LIBS += core/mowgli.core.a \
	platform/win32/mowgli.platform.win32.a \
	module/mowgli.module.a \
	object/mowgli.object.a
