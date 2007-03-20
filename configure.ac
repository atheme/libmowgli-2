#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libmowgli], [0.1], [bugs+libmowgli@atheme.org])
AC_CONFIG_SRCDIR([src/libmowgli/mowgli_list.c])
AC_CONFIG_HEADER([src/libmowgli/mowgli_config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_ISC_POSIX

# XXX workaround
PACKAGE="libmcs"
AC_SUBST(PACKAGE)

# Checks for libraries.
AM_DL

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h locale.h stdarg.h sys/types.h sys/stat.h errno.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([strstr], , [AC_ERROR([strstr is required for the plugin loader, sorry.])])
AC_CHECK_FUNCS([memset setlocale strcasecmp strchr strdup strerror strtol strtod])
AC_FUNC_STAT

# Check for optional features.
AC_ARG_ENABLE(gconf,
    [  --enable-gconf          enable GConf support (default=disabled)],
    [enable_gconf=$enableval],
    [enable_gconf="no"]
)

if test "$enable_gconf" = "yes"; then
    PKG_CHECK_MODULES(GCONF, [gconf-2.0 >= 2.6.0], ,
        [AC_MSG_ERROR([Cannot find GConf])]
    )
fi

if test "$enable_gconf" = "yes"; then
	GCONF_BUILD="gconf"
fi

AC_SUBST(GCONF_BUILD)

AC_ARG_ENABLE(kconfig,
    [  --enable-kconfig        enable KDE KConfig support (default=disabled)],
    [enable_kconfig=$enableval],
    [enable_kconfig="no"]
)

if test "$enable_kconfig" = "yes"; then
   AC_PROG_CXX

   AC_CHECK_LIB([kdecore], [main], [], [AC_MSG_ERROR([Cannot find libkdecore])])

   KCONFIG_BUILD="kconfig"
fi

AC_SUBST(KCONFIG_BUILD)

# Module support
AC_MSG_CHECKING([for what extension and flags to use for plugin compilation])
case "$target" in
        *-apple-*)
                AC_MSG_RESULT([Mac OS X: -fPIC -bundle -fno-common -flat_namespace -undefined suppress, .dylib])
                PICFLAGS="-fPIC -DPIC"
                PICLDFLAGS="-fPIC -DPIC -bundle -fno-common -flat_namespace -undefined suppress"
                LIBLDFLAGS="-dynamiclib"
                SHARED_SUFFIX=".dylib"
                ;;
        *)
                AC_MSG_RESULT([libdl-compatible: -fPIC -DPIC -shared, .so])
                PICFLAGS="-fPIC -DPIC"
                PICLDFLAGS="-fPIC -DPIC -shared"
                LIBLDFLAGS="-fPIC -DPIC -shared"
                SHARED_SUFFIX=".so"
                ;;
esac
AC_SUBST(PICFLAGS)
AC_SUBST(PICLDFLAGS)
AC_SUBST(LIBLDFLAGS)
AC_SUBST(SHARED_SUFFIX)
AC_DEFINE_UNQUOTED(SHARED_SUFFIX, "${SHARED_SUFFIX}",
        [Define the shared module suffix extension on your platform.])

AC_MSG_CHECKING([if you are running Apple-GCC])
case "$target" in
        *-apple-*)
                AC_MSG_RESULT([yes, sorry you poor bastard])
                LDFLAGS="$LDFLAGS -Wl,-framework -Wl,CoreFoundation -Wl,-framework -Wl,CoreServices"
                ;;
        *)
                AC_MSG_RESULT([no])
                ;;
esac


AC_CONFIG_FILES([libmowgli.pc mk/rules.mk])
AC_OUTPUT

cat << _EOF_

Configuration:
	No special features yet.

Now type "make" to build, and "make install" to install.
Thank you for using libmowgli.

_EOF_
