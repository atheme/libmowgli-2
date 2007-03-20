include ../../mk/rules.mk
include ../../mk/init.mk

PICLDFLAGS = $(LIBLDFLAGS)

OBJECTIVE_LIBS = libmowgli$(SHARED_SUFFIX)
OBJECTIVE_SONAME_SUFFIX = 1

LIBADD += $(DYNAMIC_LD_LIBS)

CFLAGS += $(PICFLAGS) -I..

SOURCES = \
	mowgli_alloc.c \
	mowgli_bitvector.c \
	mowgli_dictionary.c \
	mowgli_hash.c \
	mowgli_heap.c \
	mowgli_hook.c \
	mowgli_init.c \
	mowgli_list.c \
	mowgli_logger.c \
	mowgli_memorypool.c \
	mowgli_module.c \
	mowgli_object.c \
	mowgli_queue.c \
	mowgli_signal.c

OBJECTS = ${SOURCES:.c=.o}

INCLUDEDIR = ${prefix}/include/libmowgli

HEADERS = \
	mowgli.h \
	mowgli_alloc.h \
	mowgli_assert.h \
	mowgli_bitvector.h \
	mowgli_config.h \
	mowgli_dictionary.h \
	mowgli_hash.h \
	mowgli_heap.h \
	mowgli_hook.h \
	mowgli_init.h \
	mowgli_list.h \
	mowgli_logger.h \
	mowgli_memorypool.h \
	mowgli_module.h \
	mowgli_object.h \
	mowgli_queue.h \
	mowgli_signal.h \
	mowgli_stdinc.h

include ../../mk/objective.mk

install-posthook:
	@mv ${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX) ${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX).1.0.0
	@ln -sf libmowgli$(SHARED_SUFFIX).1.0.0 \
		${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX).1
	@ln -sf libmowgli$(SHARED_SUFFIX).1 \
		${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX)
