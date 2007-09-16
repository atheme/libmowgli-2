dnl
dnl Copyright (c) 2007 Jonathan Schliefer <js@h3c.de>
dnl
dnl Permission to use, copy, modify, and/or distribute this software for any
dnl purpose with or without fee is hereby granted, provided that the above
dnl copyright notice and this permission notice is present in all copies.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
dnl ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
dnl ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
dnl (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
dnl LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
dnl ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
dnl (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
dnl SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl

AC_DEFUN([AM_SHARED_LIB], [
	AC_MSG_CHECKING(how to create shared libraries)
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
