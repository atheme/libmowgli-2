dnl
dnl Copyright (c) 2007, Jonathan Schleifer <js@h3c.de>
dnl
dnl Permission to use, copy, modify, and/or distribute this software for any
dnl purpose with or without fee is hereby granted, provided that the above
dnl copyright notice and this permission notice is present in all copies.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
dnl ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
dnl LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
dnl INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
dnl CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
dnl ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
dnl POSSIBILITY OF SUCH DAMAGE.
dnl

AC_DEFUN([AM_SHARED_LIB], [
	AC_MSG_CHECKING(for shared library sytem)
	case "$target" in
		intel-apple-*)
			AC_MSG_RESULT([Mac OS X (Intel)])
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			SYMLINK_LIB='${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%%.*.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%%.*.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%%.*.dylib}.dylib'
			UNSYMLINK_LIB='rm -f ${DESTDIR}${libdir}/$${i%%.*.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%%.*.dylib}.dylib'
			;;
		*-apple-*)
			AC_MSG_RESULT(Mac OS X)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS=''
			LIB_LDFLAGS='-dynamiclib -fPIC -install_name ${libdir}/${LIB}'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.${LIB_MAJOR}.${LIB_MINOR}.dylib'
			SYMLINK_LIB='${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%%.*.dylib}.${LIB_MAJOR}.dylib && ${LN_S} -f $${i%%.*.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%%.*.dylib}.dylib'
			UNSYMLINK_LIB='rm -f ${DESTDIR}${libdir}/$${i%%.*.dylib}.${LIB_MAJOR}.dylib ${DESTDIR}${libdir}/$${i%%.*.dylib}.dylib'
			;;
		*-sun-* | *-openbsd-* | *-mirbsd-*)
			AC_MSG_RESULT(Solaris)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-shared -fPIC'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}'
			SYMLINK_LIB='${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%.so.*}.so'
			UNSYMLINK_LIB='rm -f ${DESTDIR}${libdir}/$${i%.so.*}.so'
			;;
		*-*-mingw32)
			AC_MSG_RESULT(MinGW32)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS=''
			LIB_LDFLAGS='-shared'
			LIB_PREFIX=''
			LIB_SUFFIX='.dll'
			SYMLINK_LIB='${LN_S} -f $$i ${DESTDIR}${libdir}/$$i.so'
			UNSYMLINK_LIB='rm -f ${DESTDIR}${libdir}/$$i.so'
			;;
		*)
			AC_MSG_RESULT(POSIX)
			LIB_CPPFLAGS='-DPIC'
			LIB_CFLAGS='-fPIC'
			LIB_LDFLAGS='-shared -fPIC'
			LIB_PREFIX='lib'
			LIB_SUFFIX='.so.${LIB_MAJOR}.${LIB_MINOR}.0'
			SYMLINK_LIB='${LN_S} -f $$i ${DESTDIR}${libdir}/$${i%.so.*}.so.${LIB_MAJOR} && ${LN_S} -f $${i%.so.*}.so.${LIB_MAJOR} ${DESTDIR}${libdir}/$${i%.so.*}.so'
			UNSYMLINK_LIB='rm -f ${DESTDIR}${libdir}/$${i%.so.*}.so.${LIB_MAJOR} ${DESTDIR}${libdir}/$${i%.so.*}.so'
			;;
	esac

	AC_SUBST(LIB_CPPFLAGS)
	AC_SUBST(LIB_CFLAGS)
	AC_SUBST(LIB_LDFLAGS)
	AC_SUBST(LIB_PREFIX)
	AC_SUBST(LIB_SUFFIX)
	AC_SUBST(SYMLINK_LIB)
	AC_SUBST(UNSYMLINK_LIB)
])
