#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libmowgli-2], [2.0.0-alpha1], [http://jira.atheme.org/])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([config.h])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S

AC_PROG_CC_C99
AS_IF([test "x$ac_cv_prog_cc_c99" = "xno"], [
   AC_MSG_ERROR([C compiler does not support C99], 1)
])

MORECFLAGS="-pipe -pedantic -Wall -Wextra"
AS_IF([test "x$GCC" = "xyes"], [
   CFLAGS="$CFLAGS $MORECFLAGS"
])

AC_PATH_PROG(AR, ar)
AC_PATH_PROG(RANLIB, ranlib)

BUILDSYS_SHARED_LIB

AC_CHECK_HEADERS([poll.h winsock2.h linux/futex.h sys/epoll.h sys/select.h])
AC_CHECK_FUNCS([fcntl kqueue mmap select])

OPENSSL="no"
AX_CHECK_OPENSSL([
   OPENSSL="yes"
   AC_DEFINE(HAVE_OPENSSL,[1], [Define to 1 if OpenSSL is available])
])
AS_IF([test x"$OPENSSL" = x"yes"], [
   LIBS="$LIBS $OPENSSL_LIBS"
   LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"
   CPPFLAGS="$CPPFLAGS $OPENSSL_INCLUDES"
])

AC_CHECK_LIB(dl, dlopen, [LIBS="$LIBS -ldl"])

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

EXAMPLES_BUILD=""
EXAMPLES="no"
AC_ARG_ENABLE([examples], [AS_HELP_STRING([--enable-examples], [build examples])])
AS_IF([test x"$enable_examples" = x"yes"], [
   EXAMPLES_BUILD="examples"
   EXAMPLES="yes"
])

AC_SUBST(EXAMPLES_BUILD)

BUILDSYS_INIT
BUILDSYS_TOUCH_DEPS

AC_SUBST(PACKAGE)

AC_CONFIG_FILES([buildsys.mk extra.mk libmowgli-2.pc])
AC_OUTPUT

cat << _EOF_

Configuration:
   OpenSSL support: ${OPENSSL}
   Examples: ${EXAMPLES}

Now type "make" to build, and "make install" to install.
Thank you for using libmowgli.

_EOF_
