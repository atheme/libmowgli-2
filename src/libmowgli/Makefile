include ../../mk/rules.mk
include ../../mk/init.mk

PICLDFLAGS = $(LIBLDFLAGS)

OBJECTIVE_LIBS = libmowgli$(SHARED_SUFFIX)
OBJECTIVE_SONAME_SUFFIX = 1

LIBADD += $(DYNAMIC_LD_LIBS)

CFLAGS += $(PICFLAGS) -I..

SOURCES = \
	mowgli_alloc.c \
	mowgli_dictionary.c \
	mowgli_hash.c \
	mowgli_list.c \
	mowgli_memorypool.c \
	mowgli_module.c \
	mowgli_object.c \
	mowgli_queue.c

OBJECTS = ${SOURCES:.c=.o}

INCLUDEDIR = ${prefix}/include/libmowgli

HEADERS = mowgli.h mowgli_config.h

include ../../mk/objective.mk

install-posthook:
	@mv ${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX) ${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX).1.0.0
	@ln -sf libmowgli$(SHARED_SUFFIX).1.0.0 \
		${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX).1
	@ln -sf libmowgli$(SHARED_SUFFIX).1 \
		${DESTDIR}/${LIBDIR}/libmowgli$(SHARED_SUFFIX)
