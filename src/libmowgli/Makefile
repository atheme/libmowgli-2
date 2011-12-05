include ../../extra.mk

SHARED_LIB = ${LIB_PREFIX}mowgli-2${LIB_SUFFIX}
LIB_MAJOR = 0
LIB_MINOR = 0

SUBDIRS = core base atomic container eventloop ext platform module object thread

INCLUDES = mowgli.h

include ../../buildsys.mk

LIBS += -pthread
LDFLAGS += -Wl,-Bsymbolic
CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}

LIB_OBJS += core/mowgli.core.a \
	base/mowgli.base.a \
	platform/win32/mowgli.platform.win32.a \
	module/mowgli.module.a \
	object/mowgli.object.a \
	container/mowgli.container.a \
	eventloop/mowgli.eventloop.a \
	ext/mowgli.ext.a \
	atomic/mowgli.atomic.a \
	thread/mowgli.thread.a


