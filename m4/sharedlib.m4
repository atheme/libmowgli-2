AC_DEFUN([AM_SHARED_LIB], [
	AC_MSG_CHECKING(how to create shared libraryes)
	case "$target" in
		intel-apple*)
			AC_MSG_RESULT([Mac OS X (Intel)])
			SHARED_CFLAGS="-fPIC -DPIC"
			SHARED_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			SHARED_PREFIX="lib"
			SHARED_PREVER=""
			SHARED_POSTVER=".dylib"
			;;
		*-apple-*)
			AC_MSG_RESULT(Mac OS X)
			SHARED_CFLAGS=""
			SHARED_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			SHARED_PREFIX="lib"
			SHARED_PREVER=""
			SHARED_POSTVER=".dylib"
			;;
		*)
			AC_MSG_RESULT(Unix)
			SHARED_CFLAGS="-fPIC -DPIC"
			SHARED_LDFLAGS="-shared -fPIC"
			SHARED_PREFIX="lib"
			SHARED_PREVER=".so"
			SHARED_POSTVER=""
			;;
	esac

	AC_SUBST(SHARED_CFLAGS)
	AC_SUBST(SHARED_LDFLAGS)
	AC_SUBST(SHARED_PREFIX)
	AC_SUBST(SHARED_PREVER)
	AC_SUBST(SHARED_POSTVER)
])
