# autoconf/automake basis
AC_INIT([core], [0.1])
AC_PREREQ([2.5])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects])

# libtool
LT_PREREQ(2.2)
LT_INIT([disable-static])

AS_IF([test "x$enable_static" = "xyes"], [AC_MSG_ERROR([--enable-static is not supported by core])])

# this is gcc specific
: ${CFLAGS="-std=c11 -Wall -Wextra -Wunused-function -O2 -fstack-protector -D_FORTIFY_SOURCE=2"}
: ${CXXFLAGS="-std=c++11 -Wall -Wextra -Wunused-function -Wshadow -Wpedantic -O2 -fstack-protector -D_FORTIFY_SOURCE=2"}

# toolchain
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S

AC_CHECK_HEADERS([libnetlink.h], [], AC_MSG_ERROR([cannot find required header libnetlink.h]))
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([jsoncpp/json/json.h], [], AC_MSG_ERROR([cannot find required header jsoncpp/json/json.h]))
AC_LANG_POP([C++])

dnl Documentation generation
AC_MSG_CHECKING([for asciiDoc])
AC_PATH_PROGS([ASCIIDOC], [asciidoc])
AC_PATH_PROGS([A2X], [a2x])
if test -z "${A2X}"; then
	AC_MSG_RESULT(asciiDoc found)
fi
AM_CONDITIONAL(HAVE_ASCIIDOC, test -n "${A2X}")

AC_OUTPUT([
 Makefile
 docs/man/Makefile
])
