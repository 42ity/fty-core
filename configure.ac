# autoconf/automake basis
AC_PREREQ([2.5])
AC_INIT([core], [0.1])

AC_CONFIG_SRCDIR([src/cored/main.cc])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile docs/man/Makefile])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects])

# libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])

AS_IF([test "x$enable_static" = "xyes"], [AC_MSG_ERROR([--enable-static is not supported by core])])

# toolchain
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
PKG_PROG_PKG_CONFIG

# Checks for the header files
AC_CHECK_HEADERS([libnetlink.h], [], AC_MSG_ERROR([cannot find required header libnetlink.h]))
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([jsoncpp/json/json.h], [], AC_MSG_ERROR([cannot find required header jsoncpp/json/json.h]))
AC_LANG_POP([C++])
PKG_CHECK_MODULES([LIBZMQ], [libzmq >= 3.0], [], [AC_MSG_ERROR([cannot find libzmq version 4])])

dnl ------ it was suggested by autoscan

dnl Checks for libraries.
dnl FIXME: Replace `main' with a function in `-ljsoncpp':
dnl AC_CHECK_LIB([jsoncpp], [main])
dnl FIXME: Replace `main' with a function in `-lnetlink':
dnl AC_CHECK_LIB([netlink], [main])
dnl FIXME: Replace `main' with a function in `-lzmq':
dnl AC_CHECK_LIB([zmq], [main])


AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset socket strdup])

dnl--- end of the section suggested by autoscan

# Checks for optional programs. 

dnl script is in m4
#doxygen
AC_PROG_TRY_DOXYGEN

if test -z "$DOXYGEN"; then
  AC_MSG_NOTICE([
  -----------------------------------------
  No Doxygen program found - continuing
  without Doxygen documentation support.
  -----------------------------------------
  ])
fi
myDOXDIR="docs/doxygen"
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([$myDOXDIR/Doxyfile tools/run-test-doc.sh])])
AC_SUBST([myDOXDIR])

# astyle
dnl ASTYLE would have the absolute path, use AC_CHECK_PROGS for relative
AC_PATH_PROGS([ASTYLE], [astyle])
AM_CONDITIONAL([HAVE_ASTYLE], [test -n "${ASTYLE}"])

dnl Documentation generation
# asciidoc
AC_PATH_PROGS([ASCIIDOC], [asciidoc])

# a2x
AC_PATH_PROGS([A2X], [a2x])
dnl is it ok?
AM_CONDITIONAL([HAVE_ASCIIDOC], [test -n "${A2X}"])

# set flags CFLAGS
my_CFLAGS="\
-std=c11 \
-Wall \
-Wextra \
-Wunused-function \
-fstack-protector \
-fstack-protector-all\
"

AC_SUBST([my_CFLAGS])

# set flags CXXFLAGS
my_CXXFLAGS="\
-std=c++11 \
-Wall \
-Wextra \
-Wunused-function \
-fstack-protector \
-fstack-protector-all\
"

AC_SUBST([my_CXXFLAGS])

AC_MSG_NOTICE([
        summary

        $PACKAGE_NAME $VERSION


        prefix:                  ${prefix}
        sysconf dir:             ${sysconfdir}
        datarootdir:             ${datarootdir}
        includedir:              ${includedir}
        lib dir:                 ${libdir}
	
        astyle:     			 ${ASTYLE:-NONE}
        asciidoc:                ${A2X:-NONE}

        CFLAGS:                  ${my_CFLAGS} ${CFLAGS}
        CXXFLAGS:                ${my_CXXFLAGS} ${CXXFLAGS}
        CPPFLAGS:                ${OUR_CPPFLAGS} ${CPPFLAGS}
        LDFLAGS:                 ${OUR_LDFLAGS} ${LDFLAGS}
        LIBZMQ_CFLAGS:           ${LIBZMQL_CFLAGS}
        LIBZMQ_LIBS:             ${LIBZMQ_LIBS}
    	DOXYGEN:        		 ${DOXYGEN:-NONE}
])

AC_OUTPUT
