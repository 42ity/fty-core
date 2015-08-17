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
dnl @author Michal Vyskocil <MichalVyskocil@Eaton.com>
dnl @author Jim Klimov <EvgenyKlimov@Eaton.com>
dnl @version 2014-12-16
dnl @license AllPermissive

AC_DEFUN_ONCE([BS_CHECK_SASLAUTHD_MUX],
[
  AC_MSG_CHECKING([for saslauthd mux socket])
  SASLAUTHD_MUX_SRC="null"
  AC_ARG_WITH([saslauthd-mux],
             [AS_HELP_STRING([--with-saslauthd-mux],
                              [path to saslauthd mux socket \
                               (default is first predefined path found)])],
             [
                AS_IF(	[test "x${withval}" = xyes], [SASLAUTHD_MUX=""],
            		[test "x${withval}" = xno],  [SASLAUTHD_MUX=""],
                	[SASLAUTHD_MUX="${withval}"; SASLAUTHD_MUX_SRC="withval";]
                )
              ],
              [
	        SASLAUTHD_MUX=""
              ]
  )

  # default to a list of MUX filenames
  AS_IF([test -z "${SASLAUTHD_MUX}"],
    [for sock in \
	/var/run/saslauthd/mux \
	/var/run/sasl2/mux \
    ; do
       AS_IF([test -S "$sock"], [SASLAUTHD_MUX="$sock"; SASLAUTHD_MUX_SRC="defnames"; break;])
    done
  ])

  # default to a list of directories that might hold a "mux" file
  # these may exist if saslauthd was previously started on this system
  # but either is off now, or the path is not readable to the build user
  AS_IF([test -z "${SASLAUTHD_MUX}"],
    [for sockdir in \
	/var/run/saslauthd \
	/var/run/sasl2 \
    ; do
       AS_IF([test -d "$sockdir"], [SASLAUTHD_MUX="$sockdir/mux"; SASLAUTHD_MUX_SRC="defdirs"; break;])
    done
  ])

  # default to a hardcoded value and report the configure test result,
  # or just report the result if available
  AS_IF([test -z "${SASLAUTHD_MUX}"],
    [SASLAUTHD_MUX="/var/run/saslauthd/mux"
    SASLAUTHD_MUX_SRC="defpath"
    AC_MSG_RESULT(['${SASLAUTHD_MUX}' (${SASLAUTHD_MUX_SRC})])
    AC_MSG_NOTICE([
  ----------------------------------------------------------
  Could not detect SASLAUTHD_MUX... defaulting to '$SASLAUTHD_MUX'
  You can change it using --with-saslauthd-mux
  ----------------------------------------------------------
    ])],
    [AC_MSG_RESULT(['${SASLAUTHD_MUX}' (${SASLAUTHD_MUX_SRC})])]
  )

  # add a comment about unverified MUX path saved into the config anyway
  AS_IF([test ! -S "${SASLAUTHD_MUX}"],
    [_OUT1="`ls -la ${SASLAUTHD_MUX} 2>&1`"
    SASLAUTHD_MUX_DIR="`dirname ${SASLAUTHD_MUX}`"
    _OUT2="`ls -lad ${SASLAUTHD_MUX_DIR} 2>&1`"
    AC_MSG_NOTICE([
  ----------------------------------------------------------
  NOTE: Can't verify existence of the SASLAUTHD_MUX socket
  file at this moment, just using what was requested or guessed.
    ${_OUT1}
    ${_OUT2}
  ----------------------------------------------------------
])
  ])

  AC_SUBST(SASLAUTHD_MUX)
])
