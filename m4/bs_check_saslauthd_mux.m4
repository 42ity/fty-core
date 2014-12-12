dnl @synopsis BS_CHECK_SASLAUTHD_MUX
dnl
dnl BS_CHECK_SASLAUTHD_MUX tests for an existing paths to the
dnl saslauthd socket.
dnl It sets or uses the environment variable SASLAUTHD_MUX.
dnl
dnl You can use the SASLAUTHD_MUX variable in your Makefile.in,
dnl with @SASLAUTHD_MUX@.
dnl
dnl @category SASL
dnl @author Michal Vyskocil <MichalVyskocil@eaton.com>
dnl @author Jim Klimov <EvgenyKlimov@eaton.com>
dnl @version 2014-10-29
dnl @license AllPermissive

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
  if test -z "${SASLAUTHD_MUX}" ; then
    SASLAUTHD_MUX="/var/run/saslauthd/mux"
    AC_MSG_NOTICE([
  ----------------------------------------------------------
  Could not detect SASLAUTHD_MUX... defaulting to /var/run/saslauthd/mux
  You can change it using --with-saslauthd-mux
  ----------------------------------------------------------
    ])
  fi
  AC_MSG_RESULT(${SASLAUTHD_MUX})
  if ! test -S "${SASLAUTHD_MUX}" ; then
    AC_MSG_NOTICE([
  ----------------------------------------------------------
  NOTE: Can't verify existence of the SASLAUTHD_MUX socket
  file at this moment, just using what was passed explicitly.
  `ls -la "${SASLAUTHD_MUX}"`
  `ls -lad "`dirname ${SASLAUTHD_MUX}`"`
  ----------------------------------------------------------
    ])
  fi
  AC_SUBST(SASLAUTHD_MUX)
])
