include ../../extra.mk

SHARED_LIB = ${LIB_PREFIX}mowgli-2${LIB_SUFFIX}
LIB_MAJOR = 0
LIB_MINOR = 0

LIB_OBJS += core/mowgli.core.a \
	base/mowgli.base.a \
	platform/win32/mowgli.platform.win32.a \
	module/mowgli.module.a \
	object/mowgli.object.a \
	container/mowgli.container.a \
	ext/mowgli.ext.a \
	atomic/mowgli.atomic.a

SUBDIRS = core base atomic container ext platform module object

INCLUDES = mowgli.h

include ../../buildsys.mk

LDFLAGS += -Wl,--discard-none
CPPFLAGS += ${LIB_CPPFLAGS} -I. -I.. -DMOWGLI_CORE
CFLAGS += ${LIB_CFLAGS}
