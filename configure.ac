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

# debuging
AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug],
                                     [debugging environment (default no)])])
AS_IF([test "x$enable_debug" != "xyes"], [
    AC_DEFINE(NDEBUG, [1], [Define disabling debugging])
])

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
AC_CHECK_HEADERS([tntdb.h],
                 [],
                 AC_MSG_ERROR([cannot find required header tntdb.h]))
AC_CHECK_HEADER([tnt/tntnet.h], , AC_MSG_ERROR([tntnet headers not found]))
AC_LANG_POP([C++])
PKG_CHECK_MODULES([LIBZMQ], [libzmq >= 3.0], [], [AC_MSG_ERROR([cannot find libzmq version 3])])
PKG_CHECK_MODULES([LIBCIDR], [libcidr], [], [AC_MSG_ERROR([cannot find libcidr])])
PKG_CHECK_MODULES([LIBCZMQ], [libczmq >= 3.0], [], [AC_MSG_ERROR([cannot find libczmq 3.0 (git snapshot)])])
PKG_CHECK_MODULES([LIBNUTCLIENT], [libnutclient >= 0.27], [], [AC_MSG_ERROR([cannot find libnutclient >= 0.27])])
PKG_CHECK_MODULES([CXXTOOLS], [cxxtools >= 2.2.0], [], [AC_MSG_ERROR([cannot find cxxtools >= 2.2.0])])
PKG_CHECK_MODULES([LIBSODIUM], [libsodium >= 1.0.0.], [], [AC_MSG_ERROR([cannot find libsodium >= 1.0.0])])
PKG_CHECK_MODULES([LIBSASL2], [libsasl2 >= 2.1.0.], [], [AC_MSG_ERROR([cannot find libsodium >= 2.1.0])])

dnl ------ it was suggested by autoscan
dnl TODO  I am not sure whether we need this. TODO MVY Please look at this.
dnl Checks for libraries.
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
mydoxylog="doxylog.txt"
mysrcDIR="src" 
dnl the directory to search source file that must be included in documentation
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([$myDOXDIR/Doxyfile tools/run-test-doc.sh],[chmod +x tools/run-test-doc.sh])])
AC_SUBST([myDOXDIR])
AC_SUBST([mydoxylog])
AC_SUBST([mysrcDIR])

#test-db-run-path
AC_CONFIG_FILES([tools/dbpath.h])
dnl sqlite
dnl myDBPath="sqlite:""@srcdir""@/tools/my1db.db"
dnl mariadb/mysql
dnl TODO
myDBuser=root
myDBpath="mysql:db=box_utf8;user=""${myDBuser}"
AC_SUBST([myDBpath])

# astyle
dnl ASTYLE would have the absolute path, use AC_CHECK_PROGS for relative
AC_PATH_PROGS([ASTYLE], [astyle])
AM_CONDITIONAL([HAVE_ASTYLE], [test -n "${ASTYLE}"])

dnl Documentation generation
# asciidoc
AC_PATH_PROGS([ASCIIDOC], [asciidoc])

# a2x
AC_PATH_PROGS([A2X], [a2x])
dnl TODO  MVY is it ok?
AM_CONDITIONAL([HAVE_ASCIIDOC], [test -n "${A2X}"])

# sed is great!
AC_CHECK_PROGS([SED], [sed])
# TNTnet preprocesor
AC_CHECK_PROGS([ECPPC], [ecppc])
# TNTnet
AC_CHECK_PROGS([TNTNET], [tntnet])

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
Summary:

        $PACKAGE_NAME $VERSION

        prefix:                  ${prefix}
        sysconf dir:             ${sysconfdir}
        datarootdir:             ${datarootdir}
        includedir:              ${includedir}
        lib dir:                 ${libdir}

        astyle:                  ${ASTYLE:-none}
        asciidoc:                ${A2X:-none}
        doxygen:                 ${DOXYGEN:-none}
        ecpp:                    ${ECPP:-none}

        CFLAGS:                  ${my_CFLAGS} ${CFLAGS}
        CXXFLAGS:                ${my_CXXFLAGS} ${CXXFLAGS}
        CPPFLAGS:                ${OUR_CPPFLAGS} ${CPPFLAGS}
        LDFLAGS:                 ${OUR_LDFLAGS} ${LDFLAGS}
        LIBZMQ_CFLAGS:           ${LIBZMQL_CFLAGS}
        LIBZMQ_LIBS:             ${LIBZMQ_LIBS}
        LIBCZMQ_CFLAGS:          ${LIBCZMQ_CFLAGS}
        LIBCZMQ_LIBS:            ${LIBCZMQ_LIBS}
        LIBCIDR_CFLAGS:          ${LIBCIDR_CFLAGS}
        LIBCIDR_LIBS:            ${LIBCIDR_LIBS}
        LIBNUTCLIENT_CFLAGS:     ${LIBNUTCLIENT_CFLAGS}
        LIBNUTCLIENT_LIBS:       ${LIBNUTCLIENT_LIBS}
        CXXTOOLS_CFLAGS:         ${CXXTOOLS_CFLAGS}
        CXXTOOLS_LIBS:           ${CXXTOOLS_LIBS}
        LIBSODIUM_CFLAGS:        ${LIBSODIUM_CFLAGS}
        LIBSODIUM_LIBS:          ${LIBSODIUM_LIBS}
        LIBSASL2_CFLAGS:         ${LIBSASL2_CFLAGS}
        LIBSASL2_LIBS:           ${LIBSASL2_LIBS}

        debug mode:              ${enable_debug:-no}
])

AC_OUTPUT
