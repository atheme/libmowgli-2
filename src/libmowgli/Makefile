include ../../extra.mk

SHARED_LIB = ${LIB_PREFIX}mowgli${LIB_SUFFIX}
LIB_MAJOR = 2
LIB_MINOR = 0
DISTCLEAN = mowgli_config.h

SUBDIRS = platform object

SRCS = mowgli_alloc.c			\
       mowgli_allocation_policy.c	\
       mowgli_allocator.c		\
       mowgli_argstack.c		\
       mowgli_bitvector.c		\
       mowgli_dictionary.c		\
       mowgli_error_backtrace.c		\
       mowgli_formatter.c		\
       mowgli_global_storage.c		\
       mowgli_hash.c			\
       mowgli_heap.c			\
       mowgli_hook.c			\
       mowgli_index.c			\
       mowgli_init.c			\
       mowgli_ioevent.c			\
       mowgli_list.c			\
       mowgli_logger.c			\
       mowgli_mempool.c			\
       $(MOWGLI_MODULE)			\
       mowgli_patricia.c		\
       mowgli_queue.c			\
       mowgli_random.c			\
       mowgli_signal.c			\
       mowgli_spinlock.c		\
       mowgli_string.c			\

INCLUDES = mowgli.h			\
	   mowgli_alloc.h		\
	   mowgli_allocation_policy.h	\
	   mowgli_allocator.h		\
	   mowgli_argstack.h		\
	   mowgli_assert.h		\
	   mowgli_bitvector.h		\
	   mowgli_config.h		\
	   mowgli_dictionary.h		\
	   mowgli_error_backtrace.h	\
	   mowgli_exception.h		\
	   mowgli_formatter.h		\
	   mowgli_global_storage.h	\
	   mowgli_hash.h		\
	   mowgli_heap.h		\
	   mowgli_hook.h		\
	   mowgli_index.h		\
	   mowgli_init.h		\
	   mowgli_ioevent.h		\
	   mowgli_iterator.h		\
	   mowgli_list.h		\
	   mowgli_logger.h		\
	   mowgli_mempool.h		\
	   mowgli_module.h		\
	   mowgli_patricia.h		\
	   mowgli_queue.h		\
	   mowgli_random.h		\
	   mowgli_signal.h		\
	   mowgli_spinlock.h		\
	   mowgli_stdinc.h		\
	   mowgli_string.h

include ../../buildsys.mk

CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}

LIBS += platform/win32/mowgli.platform.win32.a \
	object/mowgli.object.a
