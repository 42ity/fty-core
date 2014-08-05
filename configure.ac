dnl autoconf basis
AC_INIT(core, 0.1)
AC_PREREQ(2.5)
AC_CONFIG_SRCDIR(src)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(1.11)

dnl toolchain
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

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
