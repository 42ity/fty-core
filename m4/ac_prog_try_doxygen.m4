dnl @synopsis AC_PROG_TRY_DOXYGEN
dnl
dnl AC_PROG_TRY_DOXYGEN tests for an existing doxygen program.
dnl It sets or uses the environment variable DOXYGEN.
dnl
dnl You can use the DOXYGEN variable in your Makefile.in, with
dnl @DOXYGEN@.
dnl
dnl @category Doxygen
dnl @author John Calcote <john.calcote@gmail.com>
dnl @version 2008-06-24
dnl @license AllPermissive

AC_DEFUN([AC_PROG_TRY_DOXYGEN],
[AC_REQUIRE([AC_EXEEXT])dnl
test -z "$DOXYGEN" && AC_CHECK_PROGS([DOXYGEN], [doxygen$EXEEXT])dnl
])

AC_DEFUN_ONCE([BS_CHECK_SASLAUTHD_MUX],
[
  AC_MSG_CHECKING([for saslauthd mux socket])
  AC_ARG_WITH([saslauthd-mux],
             [AS_HELP_STRING([--with-saslauthd-mux],
                              [path to saslauthd mux socket \
                               (default is first predefined path found)])],
             [
                if test "x${withval}" = xyes
                then
                  SASLAUTHD_MUX=
                elif test "x${withval}" = xno
                then
	          SASLAUTHD_MUX=
	        else
                  SASLAUTHD_MUX=${withval}
                fi
              ],
              [
	        SASLAUTHD_MUX=
              ])
  if test -z "${SASLAUTHD_MUX}"; then
    for sock in /var/run/saslauthd/mux \
               /var/run/sasl2/mux \
               ; do
       if test -S $sock; then
         SASLAUTHD_MUX=$sock
	 break
       fi
    done
  fi
  AC_MSG_RESULT(${SASLAUTHD_MUX})
  AC_SUBST(SASLAUTHD_MUX)
])
