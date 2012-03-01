#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libmowgli-2], [2.0.0-alpha1], [http://jira.atheme.org/])
AC_CONFIG_SRCDIR([src/libmowgli/core/alloc.c])
AC_CONFIG_HEADER([src/libmowgli/platform/autoconf.h])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(RANLIB, ranlib)
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_ISC_POSIX

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
   AC_MSG_ERROR([C compiler does not support C99], 1)
])

MORECFLAGS="-pipe -pedantic -Wall -Wextra"
AS_IF([test "x$GCC" = "xyes"], [
   CFLAGS="$CFLAGS $MORECFLAGS"
])

# XXX workaround
PACKAGE="libmowgli-2"
AC_SUBST(PACKAGE)

# Checks for libraries.
BUILDSYS_SHARED_LIB
LIBS="$LIBS $DYNAMIC_LD_LIBS"

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h locale.h stdarg.h sys/types.h sys/stat.h errno.h poll.h sys/epoll.h linux/futex.h winsock2.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([memset setlocale strcasecmp strchr strdup strerror strtol strtod])
AC_CHECK_FUNCS([printf sprintf snprintf vsnprintf mmap gettimeofday strndup])
AC_CHECK_FUNCS([kqueue fcntl select])
AC_FUNC_STAT

# Check for OpenSSL
OPENSSL="no"
AX_CHECK_OPENSSL([
   LIBS="$LIBS $OPENSSL_LIBS"
   LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"
   CPPFLAGS="$CPPFLAGS $OPENSSL_INCLUDES"
	 OPENSSL="yes"
])

AC_CHECK_LIB(dl, dlopen, [LIBS="$LIBS -ldl"])

dnl Detect which loader to use
AC_MSG_CHECKING(loader type)
case "$target" in
*-*-mingw32)
    MOWGLI_MODULE='loader_win32.c'
    LIBS="$LIBS -lwsock32 -lws2_32"
	;;
*)
    MOWGLI_MODULE='loader_posix.c'
    ACX_PTHREAD
    CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS"
    AC_SUBST(PTHREAD_LIBS)
	;;
esac
AC_MSG_RESULT($MOWGLI_MODULE);
AC_SUBST(MOWGLI_MODULE)

# Check for optional features.
EXAMPLES_BUILD=""
EXAMPLES="no"
AC_ARG_ENABLE([examples], [AS_HELP_STRING([--enable-examples], [build examples])])
AS_IF([test x"$enable_examples" = x"yes"], [
	EXAMPLES_BUILD="examples"
	EXAMPLES="yes"
])

AC_SUBST(EXAMPLES_BUILD)

BUILDSYS_INIT
BUILDSYS_SHARED_LIB
BUILDSYS_TOUCH_DEPS

AC_CONFIG_FILES([buildsys.mk extra.mk libmowgli-2.pc])
AC_OUTPUT

cat << _EOF_

Configuration:
	OpenSSL support	: ${OPENSSL}
	Examples : ${EXAMPLES}

Now type "make" to build, and "make install" to install.
Thank you for using libmowgli.

_EOF_
