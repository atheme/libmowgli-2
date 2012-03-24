dnl ax_check_atomic_builtins.m4 - Check for atomic builtins 

dnl Based on Apache libapr's check
dnl http://svn.apache.org/viewvc/apr/apr/trunk/configure.in?revision=1214516

dnl Patrick McFarland <pmcfarland@adterrasperaspera.com>

AC_DEFUN([AX_CHECK_ATOMIC_BUILTINS],[
AC_MSG_CHECKING([if compiler has atomic builtins])

GCC_ATOMIC_BUILTINS=no
C11_ATOMIC_BUILTINS=no

dnl Check if compiler is a new enough GCC to avoid compile due to cross compiling
AS_IF([test "x$GCC" = "xno"],[
  GCC_VERSION="$($CC -dumpversion)"
	GCC_VERSION_MAJOR="$(echo $GCC_VERSION | cut -d'.' -f1)"
  GCC_VERSION_MINOR="$(echo $GCC_VERSION | cut -d'.' -f2)"

  AS_IF([test $GCC_VERSION_MAJOR > 4], [
    AS_IF([test $GCC_VERSION_MINOR > 1], [GCC_ATOMIC_BUILTINS=yes], [GCC_ATOMIC_BUILTINS=no])
  ], [GCC_ATOMIC_BUILTINS=no])
], [

dnl Check if compiler supports GCC atomic builtins
  AC_LANG([C])

  AC_RUN_IFELSE([AC_LANG_SOURCE([
int main()
{
  unsigned long val = 1010, tmp, *mem = &val;

  if (__sync_fetch_and_add(&val, 1010) != 1010 || val != 2020)
    return 1;

  tmp = val;

  if (__sync_fetch_and_sub(mem, 1010) != tmp || val != 1010)
    return 1;

  if (__sync_sub_and_fetch(&val, 1010) != 0 || val != 0)
    return 1;

  tmp = 3030;

  if (__sync_val_compare_and_swap(mem, 0, tmp) != 0 || val != tmp)
    return 1;

  if (__sync_lock_test_and_set(&val, 4040) != 3030)
    return 1;

  mem = &tmp;

  if (__sync_val_compare_and_swap(&mem, &tmp, &val) != &tmp)
    return 1;

  __sync_synchronize();

  if (mem != &val)
    return 1;

  return 0;
}
  ])], [GCC_ATOMIC_BUILTINS=yes], [GCC_ATOMIC_BUILTINS=no], [GCC_ATOMIC_BUILTINS=no])

dnl Check if compiler supports C11 atomic builtins
  AS_IF([test "x$GCC_ATOMIC_BUILTINS" = "xno"], [
    AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <stdatomic.h>
int main()
{
  __Atomic unsigned long val = 1010, tmp, *mem = &val;

  if (atomic_fetch_add(&val, 1010) != 1010 || val != 2020)
    return 1;

  tmp = val;

  if (atomic_fetch_sub(mem, 1010) != tmp || val != 1010)
    return 1;

  return 0;
}
    ])], [C11_ATOMIC_BUILTINS=yes], [C11_ATOMIC_BUILTINS=no], [C11_ATOMIC_BUILTINS=no])
  ])
])

ATOMIC_BUILTINS="no"

AS_IF([test "x$GCC_ATOMIC_BUILTINS" = "xyes"], [
  AC_DEFINE(HAVE_ATOMIC_BUILTINS, 1, [Define if compiler provides atomic builtins])
  AC_DEFINE(HAVE_ATOMIC_BUILTINS_GCC, 1, [Define if compiler provides GCC atomic builtins])
  ATOMIC_BUILTINS="gcc"
])

AS_IF([test "x$C11_ATOMIC_BUILTINS" = "xyes"], [
  AC_DEFINE(HAVE_ATOMIC_BUILTINS, 1, [Define if compiler provides atomic builtins])
  AC_DEFINE(HAVE_ATOMIC_BUILTINS_C11, 1, [Define if compiler provides C11 atomic builtins])
  ATOMIC_BUILTINS="C11"
])

AS_IF([test "x$ATOMIC_BUILTINS" != "xno"], [AC_MSG_RESULT($ATOMIC_BUILTINS)], [AC_MSG_RESULT(no)])
])

