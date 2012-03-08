#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libmowgli-2], [2.0.0-alpha1], [http://jira.atheme.org/])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([src/libmowgli/platform/autoconf.h])

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

MORECFLAGS="-pipe -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers"
AS_IF([test "x$GCC" = "xyes"], [
   CFLAGS="$CFLAGS $MORECFLAGS"
])

AC_CHECK_HEADERS([poll.h winsock2.h linux/futex.h sys/epoll.h sys/select.h])
AC_CHECK_FUNCS([fcntl kqueue mmap select])

AC_PATH_PROG(AR, ar)
AC_PATH_PROG(RANLIB, ranlib)

LIBMOWGLI_MODULES="core base container eventloop ext module object thread vio"
AC_SUBST(LIBMOWGLI_MODULES)

LIBMOWGLI_SHARED_MODULES=""
AC_ARG_ENABLE([shared], [AS_HELP_STRING([--disable-shared], [do not build shared library])]) 
AS_IF([test x"$enable_shared" != x"no"], [ 
   BUILDSYS_SHARED_LIB
   BUILDSYS_PROG_IMPLIB
   AC_SUBST([LIBMOWGLI_SHARED_LIB], [${LIB_PREFIX}mowgli-2${LIB_SUFFIX}])

   for i in $LIBMOWGLI_MODULES
   do
      LIBMOWGLI_SHARED_MODULES="$LIBMOWGLI_SHARED_MODULES $i/mowgli.$i.lib.a"
   done

   BUILD_SHARED_MODULES="yes"
])

LIBMOWGLI_STATIC_MODULES=""
AC_ARG_ENABLE([static], [AS_HELP_STRING([--enable-static], [build static library])])
AS_IF([test x"$enable_static" = x"yes" -o x"$enable_shared" = x"no"], [
   AC_SUBST([LIBMOWGLI_STATIC_LIB], [libmowgli-2.a])

   for i in $LIBMOWGLI_MODULES
   do
      LIBMOWGLI_STATIC_MODULES="$LIBMOWGLI_STATIC_MODULES $i/mowgli.$i.a"
   done

   BUILD_STATIC_MODULES="yes"
])

AC_MSG_CHECKING(OS type)
case "$target" in
*-*-mingw32)
   AC_MSG_RESULT([win32])
   LIBMOWGLI_OS="win32"

   LIBS="$LIBS -lwsock32 -lws2_32"

   AS_IF([test x"$LIBMOWGLI_SHARED_MODULES" != x""], [
      LIBMOWGLI_SHARED_MODULES="$LIBMOWGLI_SHARED_MODULES platform/win32/mowgli.platform.win32.lib.a"
   ])

   AS_IF([test x"$LIBMOWGLI_STATIC_MODULES" != x""], [
      LIBMOWGLI_STATIC_MODULES="$LIBMOWGLI_STATIC_MODULES platform/win32/mowgli.platform.win32.a"
   ])
   ;;
*)
   AC_MSG_RESULT([posix])
   LIBMOWGLI_OS="posix"

   ACX_PTHREAD
   CPPFLAGS="$CFLAGS $PTHREAD_CFLAGS"
   LIBS="$PTHREAD_LIBS $LIBS"
   AC_CHECK_LIB(dl, dlopen, [LIBS="$LIBS -ldl"])
   ;;
esac
AC_SUBST([LIBMOWGLI_OS])
AC_SUBST([LIBMOWGLI_SHARED_MODULES])
AC_SUBST([LIBMOWGLI_STATIC_MODULES])

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

AC_SUBST(PACKAGE_NAME)

AC_CONFIG_FILES([buildsys.mk extra.mk libmowgli-2.pc])
AC_OUTPUT

cat << _EOF_

Configuration:
   OpenSSL support: ${OPENSSL}
   Examples: ${EXAMPLES}

Now type "make" to build, and "make install" to install.
Thank you for using libmowgli.

_EOF_
