AC_INIT([automake-trap], [0.0.0])
AC_CONFIG_AUX_DIR([.])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl AC_CANONICAL_HOST and AC_CANONICAL_TARGET require
dnl config.sub/config.guess which you only get if you use automake.
dnl So we just put a Makefile.am in a trap directory to make it happy.
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
