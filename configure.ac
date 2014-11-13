# autoconf/automake basis
AC_PREREQ([2.5])
AC_INIT([core], [0.1])

AC_CONFIG_SRCDIR([src/simple/main.cc])
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

# display source/build paths of interest to us
echo "Value of srcdir...        '$srcdir'"
echo "Value of current dir...   '`pwd`'"
case "$srcdir" in
    /*)	_srcdir_abs="$srcdir" ;;
    *)  _srcdir_abs="`pwd`/$srcdir"
	if test -d "$_srcdir_abs" ; then
	    _srcdir_abs="`cd "$_srcdir_abs" && pwd || echo "$_srcdir_abs"`"
	fi ;;
esac
echo "Value of _srcdir_abs...   '$_srcdir_abs'"

# toolchain
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
PKG_PROG_PKG_CONFIG

# Get the Git repository metadata, if available
HAVE_PACKAGE_GIT=0
PACKAGE_GIT_ORIGIN=""
PACKAGE_GIT_BRANCH=""
PACKAGE_GIT_TSTAMP=""
PACKAGE_GIT_HASH_S=""
PACKAGE_GIT_HASH_L=""
AC_PATH_PROGS([GIT], [git])
if test ! -z "$GIT" -a -x "$GIT" -a -d "$_srcdir_abs/.git" ; then
	echo "Getting Git workspace attributes..."
	PACKAGE_GIT_ORIGIN="`cd "$_srcdir_abs" && $GIT config --get remote.origin.url`"	&& HAVE_PACKAGE_GIT=1
	PACKAGE_GIT_BRANCH="`cd "$_srcdir_abs" && $GIT rev-parse --abbrev-ref HEAD`"	&& HAVE_PACKAGE_GIT=1
	PACKAGE_GIT_TSTAMP="`cd "$_srcdir_abs" && $GIT log -n 1 --format='%ct'`"		&& HAVE_PACKAGE_GIT=1
	PACKAGE_GIT_HASH_S="`cd "$_srcdir_abs" && $GIT log -n 1 --format='%h'`"		&& HAVE_PACKAGE_GIT=1
	PACKAGE_GIT_HASH_L="`cd "$_srcdir_abs" && $GIT rev-parse --verify HEAD`"		&& HAVE_PACKAGE_GIT=1
fi
AC_DEFINE_UNQUOTED([HAVE_PACKAGE_GIT],   [$HAVE_PACKAGE_GIT],     [Packaging metadata: Is Git metadata available?])
AC_DEFINE_UNQUOTED([PACKAGE_GIT_ORIGIN], ["$PACKAGE_GIT_ORIGIN"], [Packaging metadata: URL of the Git origin repository (parent of the build workspace)])
AC_DEFINE_UNQUOTED([PACKAGE_GIT_BRANCH], ["$PACKAGE_GIT_BRANCH"], [Packaging metadata: Git branch in the build workspace repository])
AC_DEFINE_UNQUOTED([PACKAGE_GIT_TSTAMP], ["$PACKAGE_GIT_TSTAMP"], [Packaging metadata: Git timestamp of the commit used for the build])
AC_DEFINE_UNQUOTED([PACKAGE_GIT_HASH_S], ["$PACKAGE_GIT_HASH_S"], [Packaging metadata: Git short-hash of the commit used for the build])
AC_DEFINE_UNQUOTED([PACKAGE_GIT_HASH_L], ["$PACKAGE_GIT_HASH_L"], [Packaging metadata: Git long-hash of the commit used for the build])

AC_PATH_PROGS([UNAME], [uname])
HAVE_PACKAGE_BUILD_HOST=0
PACKAGE_BUILD_HOST_UNAME=""
PACKAGE_BUILD_HOST_NAME=""
PACKAGE_BUILD_HOST_OS=""
if test ! -z "$UNAME" -a -x "$UNAME" ; then
	echo "Getting buildhost attributes..."
	PACKAGE_BUILD_HOST_UNAME="`$UNAME -a`"		&& HAVE_PACKAGE_BUILD_HOST=1
	PACKAGE_BUILD_HOST_NAME="`$UNAME -n`"		&& HAVE_PACKAGE_BUILD_HOST=1
	PACKAGE_BUILD_HOST_OS="`$UNAME -s -r -v`"	&& HAVE_PACKAGE_BUILD_HOST=1
fi
AC_DEFINE_UNQUOTED([HAVE_PACKAGE_BUILD_HOST],  [$HAVE_PACKAGE_BUILD_HOST],    [Packaging metadata: Are build-host details available?])
AC_DEFINE_UNQUOTED([PACKAGE_BUILD_HOST_UNAME], ["$PACKAGE_BUILD_HOST_UNAME"], [Packaging metadata: Full 'uname -a' of the building host])
AC_DEFINE_UNQUOTED([PACKAGE_BUILD_HOST_NAME],  ["$PACKAGE_BUILD_HOST_NAME"],  [Packaging metadata: Hostname of the building host])
AC_DEFINE_UNQUOTED([PACKAGE_BUILD_HOST_OS],    ["$PACKAGE_BUILD_HOST_OS"],    [Packaging metadata: OS/kernel of the building host])

AC_PATH_PROGS([DATE], [gdate date])
HAVE_PACKAGE_BUILD_TSTAMP=0
PACKAGE_BUILD_TSTAMP=""
if test ! -z "$DATE" -a -x "$DATE" ; then
	echo "Recording the configuration timestamp..."
	PACKAGE_BUILD_TSTAMP="`$DATE -u '+%s'`"		&& HAVE_PACKAGE_BUILD_TSTAMP=1
fi
AC_DEFINE_UNQUOTED([HAVE_PACKAGE_BUILD_TSTAMP],	[$HAVE_PACKAGE_BUILD_TSTAMP],	[Packaging metadata: Is the build-host timestamp available?])
AC_DEFINE_UNQUOTED([PACKAGE_BUILD_TSTAMP],	["$PACKAGE_BUILD_TSTAMP"],	[Packaging metadata: Current timestamp at the build host (as of configuration)])

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
PKG_CHECK_MODULES([LIBSODIUM], [libsodium >= 0.7.0], [], [AC_MSG_ERROR([cannot find libsodium >= 0.7.0])])
PKG_CHECK_MODULES([LIBSASL2], [libsasl2 >= 2.1.0.], [], [AC_MSG_ERROR([cannot find sasl 2 library >= 2.1.0])])

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
dnl TODO  MVY a2x - is it ok?
AC_PATH_PROGS([A2X], [a2x])

if test -z "$ASCIIDOC" -a -z "$A2X"; then
  AC_MSG_NOTICE([
  -----------------------------------------
  No asciidoc or a2x program found - continuing
  without ASCIIDOC documentation support.
  -----------------------------------------
  ])
fi
AM_CONDITIONAL([HAVE_ASCIIDOC],[test -n "$ASCIIDOC"])
AM_CONDITIONAL([HAVE_A2X],[test -n "$A2X"])


# sed is great!
AC_CHECK_PROGS([SED], [sed])
# TNTnet preprocesor
AC_CHECK_PROGS([ECPPC], [ecppc])
# TNTnet
AC_CHECK_PROGS([TNTNET], [tntnet])
AS_IF([test -z "$TNTNET" -o -z "$ECPPC"],[
  AC_MSG_ERROR([TNTnet not found!])
])

BS_CHECK_SASLAUTHD_MUX

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

        Git origin URL:          ${PACKAGE_GIT_ORIGIN}
        Git branch:              ${PACKAGE_GIT_BRANCH}
        Git last-commit tstamp:  ${PACKAGE_GIT_TSTAMP}
        Git last-commit hash(S): ${PACKAGE_GIT_HASH_S}
        Git last-commit hash(L): ${PACKAGE_GIT_HASH_L}

        Build-host 'uname -a':   ${PACKAGE_BUILD_HOST_UNAME}
        Build-host system name:  ${PACKAGE_BUILD_HOST_NAME}
        Build-host OS/kernel:    ${PACKAGE_BUILD_HOST_OS}
        Build-host timestamp:    ${PACKAGE_BUILD_TSTAMP}

        prefix:                  ${prefix}
        sysconf dir:             ${sysconfdir}
        datarootdir:             ${datarootdir}
        includedir:              ${includedir}
        lib dir:                 ${libdir}

        a2x:                     ${A2X:-none}
        astyle:                  ${ASTYLE:-none}
        asciidoc:                ${ASCIIDOC:-none}
        doxygen:                 ${DOXYGEN:-none}
        ecppc:                   ${ECPPC:-none}

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
        SASLAUTHD_MUX:           ${SASLAUTHD_MUX}

        debug mode:              ${enable_debug:-no}

	absolute SRC DIR:        ${_srcdir_abs}
])

unset _srcdir_abs

AC_OUTPUT
