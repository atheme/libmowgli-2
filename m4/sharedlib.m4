AC_DEFUN([AM_SHARED_LIB], [
	AC_MSG_CHECKING(how to create shared libraryes)
	case "$target" in
		intel-apple*)
			AC_MSG_RESULT([Mac OS X (Intel)])
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			;;
		*-apple-*)
			AC_MSG_RESULT(Mac OS X)
			LIB_CFLAGS=''
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			;;
		*-sun-* | *-openbsd-* | *-mirbsd-*)
			AC_MSG_RESULT(Solaris)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-shared -fPIC'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}'
			;;
		*)
			AC_MSG_RESULT(Unix)
			LIB_CFLAGS='-fPIC -DPIC'
			LIB_LDFLAGS='-shared -fPIC'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}.0'
			;;
	esac

	AC_SUBST(LIB_CFLAGS)
	AC_SUBST(LIB_LDFLAGS)
	AC_SUBST(LIB_PREFIX)
	AC_SUBST(LIB_SUFFIX)
])
