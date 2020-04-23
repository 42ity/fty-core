# shell include file: scriptlib.sh
#
# Copyright (C) 2014 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    scriptlib.sh
#  \brief   Base library for scripts
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Determine the directory name variables relevant for compiled
#           workspace which is under test. Mainly for inclusion in 42ity
#           ./tests/CI scripts.
#           The variable values may be set by caller or an earlier stage
#           in script interpretation, otherwise they get defaulted here.

if [ -n "${BASH-}" ]; then
    # A bash-ism, should set the exitcode of the rightmost failed command
    # in a pipeline, otherwise e.g. exitcode("false | true") == 0
    set -o pipefail 2>/dev/null || true
    echo_E() { echo -E "$@"; }
    echo_e() { echo -e "$@"; }
else
    echo_E() { /bin/echo -E "$@"; }
    echo_e() { /bin/echo -e "$@"; }
fi

### Some variables might not be initialized
set +u

### Store some important CLI values. DO NOT "export" THESE!
[ -z "${_SCRIPT_STARTPWD-}" ] && _SCRIPT_STARTPWD="`pwd`"
[ -z "${_SCRIPT_PATH-}" ] && _SCRIPT_PATH="$0"
[ -z "${_SCRIPT_NAME-}" ] && _SCRIPT_NAME="`basename "${_SCRIPT_PATH}"`"
_SCRIPT_ARGS="$*"
_SCRIPT_ARGC="$#"

### Just a tag for pretty output below
[ -z "${_SCRIPT_TYPE-}" ] && \
    case "${_SCRIPT_NAME-}" in
        ci-*|CI-*) _SCRIPT_TYPE="Test" ;;
        *) _SCRIPT_TYPE="Program" ;;
    esac

### Database credentials
[ -z "${DB_USER-}" ] || DBUSER="${DB_USER-}"
[ -z "${DB_PASSWD-}" ] || DBPASSWD="${DB_PASSWD}"
[ -z "${DBUSER-}" ] && DBUSER=root
[ -z "${DATABASE-}" ] && DATABASE=box_utf8
export DBUSER DATABASE DBPASSWD

### REST API (and possibly non-privileged SSH) user credentials
[ -z "${BIOS_USER-}" ] && BIOS_USER="admin"
[ -z "${BIOS_PASSWD-}" ] && BIOS_PASSWD="admin"
[ -z "${SASL_SERVICE-}" ] && SASL_SERVICE="bios"
export BIOS_USER BIOS_PASSWD SASL_SERVICE

### Variables for remote testing - avoid "variable not defined" errors
[ -z "${SUT_IS_REMOTE-}" ] && SUT_IS_REMOTE="auto" # auto|yes|no
[ -z "${SUT_USER-}" ] && SUT_USER="root"   # Username on remote SUT
[ -z "${SUT_HOST-}" ] && SUT_HOST="127.0.0.1"       # Hostname or IP address
[ -z "${SUT_SSH_PORT-}" ] && SUT_SSH_PORT="22"       # SSH (maybe via NAT)
[ -z "${SUT_WEB_PORT-}" ] && SUT_WEB_PORT="8000"       # TNTNET (maybe via NAT)
[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""
[ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="http"
if [ "${SUT_WEB_PORT-}" -eq 443 ]; then
    SUT_WEB_SCHEMA="https"
fi
case "${SUT_API_PREFIX-}" in
    "") SUT_API_PREFIX="/api/v1" ;;
    /*) ;;
    *)  SUT_API_PREFIX="/$SUT_API_PREFIX" ;;
esac
[ -z "${BASE_URL-}" ] && BASE_URL="${SUT_WEB_SCHEMA}://${SUT_HOST}:${SUT_WEB_PORT}${SUT_API_PREFIX}"

### By default, do we stop 42ity services before re-initializing the database?
### See reloaddb_stops_BIOS() as the callable method
[ -z "${LOADDB_RESTART_BIOS-}" ] && LOADDB_RESTART_BIOS=auto
[ x"${LOADDB_RESTART_BIOS-}" = x- ] && LOADDB_RESTART_BIOS=auto

export SUT_IS_REMOTE LOADDB_RESTART_BIOS SUT_USER SUT_HOST SUT_SSH_PORT SUT_WEB_PORT SUT_WEB_SCHEMA BASE_URL

### Should the test suite break upon first failed test?
[ x"${CITEST_QUICKFAIL-}" = x- ] && CITEST_QUICKFAIL=""
[ x"${CITEST_QUICKFAIL-}" != xyes ] && CITEST_QUICKFAIL=no
export CITEST_QUICKFAIL

### For remote tests and general database initalization, we want some quiet
### time after initializing the database. For tests that loop with uploads
### of small DB snippets, we want to minimize such sleeps!
[ -z "${LOADDB_FILE_REMOTE_SLEEP-}" ] && LOADDB_FILE_REMOTE_SLEEP=0
export LOADDB_FILE_REMOTE_SLEEP

### Set the default language (e.g. for CI apt-get to stop complaining)
[ -z "${LANG-}" ] && LANG=C
[ -z "${LANGUAGE-}" ] && LANGUAGE=C
[ -z "${LC_ALL-}" ] && LC_ALL=C
[ -z "${TZ-}" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

determineDirs() {
    ### Note: a set, but invalid, value will cause an error to the caller
    [ -n "${SCRIPTDIR-}" -a -d "${SCRIPTDIR-}" ] || \
        SCRIPTDIR="$(cd "`dirname ${_SCRIPT_PATH-}`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname ${_SCRIPT_PATH-}`" || \
        SCRIPTDIR="$(realpath "`dirname ${_SCRIPT_PATH-}`")" || \
        SCRIPTDIR="`dirname ${_SCRIPT_PATH-}`"

    if [ -z "${CHECKOUTDIR-}" ]; then
        case "$SCRIPTDIR" in
            */tests/CI|tests/CI)
                CHECKOUTDIR="$(realpath $SCRIPTDIR/../..)" || \
                CHECKOUTDIR="$(echo "$SCRIPTDIR" | sed 's|/tests/CI$||')" || \
                CHECKOUTDIR="$(cd "$SCRIPTDIR"/../.. && pwd)" || \
                CHECKOUTDIR="" ;;
            */tools|tools)
                CHECKOUTDIR="$(realpath $SCRIPTDIR/..)" || \
                CHECKOUTDIR="$(echo "$SCRIPTDIR" | sed 's|/tools$||')" || \
                CHECKOUTDIR="$(cd "$SCRIPTDIR"/.. && pwd)" || \
                CHECKOUTDIR="" ;;
        esac
    fi
    [ -z "${CHECKOUTDIR-}" -a -d ~/project ] && CHECKOUTDIR=~/project

    if [ -z "${BUILDSUBDIR-}" ]; then
        ### Keep a caller-defined BUILDSUBDIR value even if it is not made yet
        BUILDSUBDIR="$CHECKOUTDIR"
        [ ! -x "$BUILDSUBDIR/config.status" -a ! -s "$BUILDSUBDIR/autogen.sh" ] && \
            BUILDSUBDIR="$PWD"
    fi

    export BUILDSUBDIR CHECKOUTDIR SCRIPTDIR

    if [ -n "$BUILDSUBDIR" -a x"$BUILDSUBDIR" != x"$CHECKOUTDIR" ]; then
        AUTOGEN_ACTION_MAKE=make-subdir
        AUTOGEN_ACTION_BUILD=build-subdir
        AUTOGEN_ACTION_CONFIG=configure-subdir
        AUTOGEN_ACTION_INSTALL=install-subdir
    else
        AUTOGEN_ACTION_MAKE=make
        AUTOGEN_ACTION_BUILD=build
        AUTOGEN_ACTION_CONFIG=configure
        AUTOGEN_ACTION_INSTALL=install
    fi
    export AUTOGEN_ACTION_MAKE AUTOGEN_ACTION_BUILD AUTOGEN_ACTION_CONFIG \
        AUTOGEN_ACTION_INSTALL

    [ -z "${MAKELOG-}" ] && MAKELOG="$BUILDSUBDIR/make.output"
    export MAKELOG

    ### Ultimate status: if false, then the paths are non-development
    [ -n "$SCRIPTDIR" -a -n "$CHECKOUTDIR" -a -n "$BUILDSUBDIR" ] && \
    [ -d "$SCRIPTDIR" -a -d "$CHECKOUTDIR" -a -d "$BUILDSUBDIR" ] && \
    [ -x "$BUILDSUBDIR/config.status" ]
}

### Empty and non-numeric and non-positive values should be filtered out here
is_positive() {
    [ -n "$1" -a "$1" -gt 0 ] 2>/dev/null
}
default_posval() {
    eval is_positive "\$$1" || eval "$1"="$2"
}

### Caller can disable specific debuggers by setting their level too
### high in its environment variables for a specific script run.
### Scripts can use this mechanism to set flexible required-verbosity
### levels for their messages.
CI_DEBUGLEVEL_NOOP="-1"
# logmsg_echo() adds no prefix and prints the message if CI_DEBUG>=$1
# (or CI_DEBUGLEVEL_ECHO if $1 is not a number)
default_posval CI_DEBUGLEVEL_ECHO       0

# Standard stuff
default_posval CI_DEBUGLEVEL_ERROR      1
default_posval CI_DEBUGLEVEL_WARN       2
default_posval CI_DEBUGLEVEL_INFO       3
default_posval CI_DEBUGLEVEL_TRACE      4
default_posval CI_DEBUGLEVEL_DEBUG      5

# Custom stuff for specific routines
default_posval CI_DEBUGLEVEL_RUN        $CI_DEBUGLEVEL_TRACE
default_posval CI_DEBUGLEVEL_LOADDB     $CI_DEBUGLEVEL_RUN
default_posval CI_DEBUGLEVEL_DUMPDB     $CI_DEBUGLEVEL_RUN
default_posval CI_DEBUGLEVEL_SELECT     $CI_DEBUGLEVEL_RUN
default_posval CI_DEBUGLEVEL_PIPESNIFFER $CI_DEBUGLEVEL_DEBUG

# Semihack used below to enable "set -x" by very large debug level
default_posval CI_DEBUGLEVEL_TRACEEXEC  200
default_posval CI_DEBUGLEVEL_TIME_SUT_RUN 99

### Default debugging/info/warning level for this lifetime of the script
### Messages are printed if their assigned level is at least CI_DEBUG
### The default of "3" allows INFO messages to be printed or easily
### suppressed by change to a smaller number. The level of "2" is default
### for warnings and "1" for errors, and a "0" would likely hide most
### such output. "Yes" bumps up a high level to enable even greater debug
### details, while "No" only leaves the default errors and warnings.
case "${CI_DEBUG-}" in
    ""|"-")
        CI_DEBUG="$CI_DEBUGLEVEL_INFO"
        case "${BIOS_LOG_LEVEL-}" in
            LOG_DEBUG)      CI_DEBUG="$CI_DEBUGLEVEL_DEBUG" ;;
            LOG_INFO)       CI_DEBUG="$CI_DEBUGLEVEL_INFO"  ;;
            LOG_WARNING)    CI_DEBUG="$CI_DEBUGLEVEL_WARN"  ;;
            LOG_ERR)        CI_DEBUG="$CI_DEBUGLEVEL_ERROR" ;;
            LOG_CRIT)       CI_DEBUG="`expr $CI_DEBUGLEVEL_NOOP + 1`"  ;;
            LOG_NOOP)       CI_DEBUG="$CI_DEBUGLEVEL_NOOP" ;;
        esac
        ;;
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])     CI_DEBUG=99 ;;
    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]) CI_DEBUG="$CI_DEBUGLEVEL_WARN" ;;
esac
[ "$CI_DEBUG" -ge -1 ] 2>/dev/null || CI_DEBUG="$CI_DEBUGLEVEL_INFO"
[ "$CI_DEBUG" -ge -1 ] 2>/dev/null || CI_DEBUG=3

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-"

logmsg_echo() {
    ### Optionally echoes a message, based on current debug-level
    ### Does not add any headers to the output line
        # By default, do echo unless $1 says otherwise

    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_ECHO
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "$@"
    :
}

logmsg_info() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_INFO
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}INFO: ${_SCRIPT_PATH}:" "$@"
    :
}

logmsg_warn() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_WARN
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}WARN: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_error() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_ERROR
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}ERROR: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_trace() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_TRACE
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}TRACE: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_sut_run() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_RUN
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}TRACE-SUT_RUN: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_loaddb() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_LOADDB
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}TRACE-LOADDB: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_dumpdb() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_DUMPDB
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}TRACE-DUMPDB: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_do_select() {
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_SELECT
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
    echo_E "${LOGMSG_PREFIX}TRACE-DO_SELECT: ${_SCRIPT_PATH}:" "$@" >&2
    :
}

logmsg_debug() {
    # A script can flexibly define its different debug messages via variables
    # with debug-levels assigned (and easily changeable) to different subjects
    WANT_DEBUG_LEVEL=$CI_DEBUGLEVEL_DEBUG
    if [ "$1" -ge 0 ] 2>/dev/null; then
        WANT_DEBUG_LEVEL="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi

    [ "$CI_DEBUG" -ge "$WANT_DEBUG_LEVEL" ] 2>/dev/null && \
        for LINE in "$@"; do
            echo_E "${LOGMSG_PREFIX}DEBUG[$WANT_DEBUG_LEVEL<=$CI_DEBUG]: $LINE"
        done >&2
    :
}

tee_stderr() {
    ### This routine allows to optionally "sniff" piped streams, e.g.
    ###   prog1 | tee_stderr LISTING_TOKENS $DEBUGLEVEL_PRINTTOKENS | prog2
    TEE_TAG="PIPESNIFF:"
    [ -n "$1" ] && TEE_TAG="$1:"
    [ -n "$2" -a "$2" -ge 0 ] 2>/dev/null && \
        TEE_DEBUG="$2" || \
        TEE_DEBUG=$CI_DEBUGLEVEL_PIPESNIFFER

    ### If debug is not enabled, skip tee'ing quickly with little impact
    [ "$CI_DEBUG" -lt "$TEE_DEBUG" ] 2>/dev/null && cat || \
    while IFS= read -r LINE; do
        echo_E "$LINE"
        echo_E "${LOGMSG_PREFIX}$TEE_TAG" "$LINE" >&2
    done
    :
}

# This value can be set, at least in bash, to specify at which location
# the caller of DIE was in the scripts
SCRIPTLIB_DIE_FILENAME=""	# Which (maybe included) file caused the failure
SCRIPTLIB_DIE_FUNCNAME=""	# Which function failed, maybe main(), source() etc
SCRIPTLIB_DIE_LINENO=0  	# Which line in the file (or function) failed?
SCRIPTLIB_DIE_SUBSHELL=-1	# Depth of automatic subshelling "()" "``" "$()"...
# These are inherited from caller in any shell:
SCRIPTLIB_DIE_SCRIPT_PATH=""
SCRIPTLIB_DIE_SCRIPT_NAME=""
SCRIPTLIB_DIE_SCRIPT_ARGS=""
SCRIPTLIB_DIE_SCRIPT_ARGC=-1
# This value is also a flag to decide if the SCRIPTLIB_DIE_* vars are to
# be consulted in the settraps() handler below rather than autodetection
# at the moment of trap execution (which may be already limited by shell).
SCRIPTLIB_DIE_ERRCODE=-1
die() {
    # The exit CODE can be passed as a variable, or as the first parameter
    # (if it is a number), both ways can be used for legacy reasons
    CODE="${CODE-1}"
    if [ "$1" -ge 0 ] 2>/dev/null; then
        CODE="$1"
        shift
    else if [ x"$1" = x"" ] && [ $# -gt 0 ]; then shift; fi
    fi
    [ "$CODE" -ge 0 ] 2>/dev/null || CODE=1
    for LINE in "$@" ; do
        echo_E "${LOGMSG_PREFIX-}FATAL: ${_SCRIPT_PATH-}:" "$LINE" >&2
    done
    # Set common vars for settraps() standard handler
    SCRIPTLIB_DIE_ERRCODE="${CODE}"
    SCRIPTLIB_DIE_SCRIPT_PATH="${_SCRIPT_PATH-}"
    SCRIPTLIB_DIE_SCRIPT_NAME="${_SCRIPT_NAME-}"
    SCRIPTLIB_DIE_SCRIPT_ARGS="${_SCRIPT_ARGS-}"
    SCRIPTLIB_DIE_SCRIPT_ARGC="${_SCRIPT_ARGC-}"
    if [ -n "${BASH}" ]; then
        # Detect who called die()
        SCRIPTLIB_DIE_LINENO="${BASH_LINENO[0]-}"
        SCRIPTLIB_DIE_FUNCNAME="${FUNCNAME[1]-}"
        SCRIPTLIB_DIE_FILENAME="${BASH_SOURCE[1]}"
        SCRIPTLIB_DIE_SUBSHELL="${BASH_SUBSHELL}"
    else
        # Definitions of LINENO vary greatly from shell to shell
        # In practice must be >= 1 if defined at all
        SCRIPTLIB_DIE_LINENO="${LINENO-}"
        [ "$SCRIPTLIB_DIE_LINENO" -ge 1 ] 2>/dev/null || SCRIPTLIB_DIE_LINENO=0
        SCRIPTLIB_DIE_FILENAME="${SCRIPTLIB_DIE_SCRIPT_NAME}"
        SCRIPTLIB_DIE_FUNCNAME=""
        SCRIPTLIB_DIE_SUBSHELL=-1
    fi
    exit $CODE
}

determineDirs_default() {
    determineDirs
    RES=$?

    [ "${NEED_CHECKOUTDIR-}" = no ] || \
    if [ -n "$CHECKOUTDIR" -a -d "$CHECKOUTDIR" ]; then
        echo "${LOGMSG_PREFIX}INFO: ${_SCRIPT_TYPE} '${_SCRIPT_PATH} ${_SCRIPT_ARGS}' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."
    else
        echo "${LOGMSG_PREFIX}WARN: ${_SCRIPT_TYPE} '${_SCRIPT_PATH} ${_SCRIPT_ARGS}' can not detect a CHECKOUTDIR value..." >&2
        RES=1
        if [ "${NEED_CHECKOUTDIR-}" = yes ]; then
            exit $RES
        fi
    fi

    [ "${NEED_BUILDSUBDIR-}" = no ] || \
    if [ -n "$BUILDSUBDIR" -a \
         -d "$BUILDSUBDIR" -a -x "$BUILDSUBDIR/config.status" ]; then
        logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR'"
    else
        [ "${NEED_BUILDSUBDIR-}" = yes ] && _LM=logmsg_error || _LM=logmsg_warn
        ${_LM} "Cannot find '$BUILDSUBDIR/config.status', did you run configure?"
        ${_LM} "Search path checked: '$CHECKOUTDIR', '$PWD'"
        unset _LM
        RES=1
        if [ "${NEED_BUILDSUBDIR-}" = yes ]; then
            ls -lad "$BUILDSUBDIR/config.status" "$CHECKOUTDIR/config.status" \
                "$PWD/config.status" >&2
            exit $RES # This is expected to fail with report of what's missing
        fi
    fi

    return $RES
}

isRemoteSUT() {
    case "${SUT_IS_REMOTE-}" in
        yes)
            ### Yes, we are testing a remote box or a VTE,
            ### and have a cached decision or explicit setting
            return 0 ;;
        no) ### No, the test is known local
            return 1 ;;
    esac

    if  [ -z "${SUT_IS_REMOTE-}" -o x"${SUT_IS_REMOTE-}" = xauto -o x"${SUT_IS_REMOTE-}" = x- ] && \
        [ -n "${SUT_HOST-}" -a -n "${SUT_SSH_PORT-}" ] && \
        [ x"${SUT_HOST-}" != xlocalhost -a x"${SUT_HOST-}" != x127.0.0.1 ] \
    ; then
        ### TODO: Maybe a better test is needed e.g. "localhost and port==22"
        SUT_IS_REMOTE=yes
        return 0
    fi

    ### No, test is local as far as we currently know
    ### NOTE: No automatic caching of decision for "maybe-no" since the needed
    ### variables may become defined later.
    return 2
}

reloaddb_stops_BIOS() {
    if [ "$LOADDB_RESTART_BIOS" = auto ] || [ x"$LOADDB_RESTART_BIOS" = x- ]; then
        isRemoteSUT
        case $? in
            0) LOADDB_RESTART_BIOS=yes ;;
            1) LOADDB_RESTART_BIOS=no ;;
            *) LOADDB_RESTART_BIOS=auto ;;
        esac
    fi
    [ "$LOADDB_RESTART_BIOS" = yes ] && return 0
    [ "$LOADDB_RESTART_BIOS" = no ] && return 1
    LOADDB_RESTART_BIOS=auto
    # To be on the safe side, we cause service restart while undecided
    return 0
}

sut_run() {
    ### This tries to run a command either locally or externally via SSH,
    ### depending on what we are testing (local or remote System Under Test).
    ### If the first argument ($1) is "-t" then this requests a TTY for SSH.
    ### NOTE: By current construction this may fail for parameters that are
    ### not one token aka "$1".
    if isRemoteSUT ; then
        logmsg_sut_run \
            "sut_run()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $@" >&2
        SSH_TERMINAL_REQUEST=""
        [ "$1" = "-t" ] && shift && SSH_TERMINAL_REQUEST="-t -t" #" -o RequestTTY=true"
        # TODO: Currently this is No-op for sut_run over SSH usecase :(
        [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_RUN" ] 2>/dev/null && \
            REMCMD="bash -x -c " || \
            REMCMD="bash -c "
        $TIME_SUT_RUN ssh $SSH_TERMINAL_REQUEST -p "${SUT_SSH_PORT}" -l "${SUT_USER}" "${SUT_HOST}" "$@"
        return $?
    else
        [ "$1" = "-t" ] && shift        # Ignore for local host
        logmsg_sut_run \
            "sut_run()::local: $@" >&2
        if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_RUN" ] 2>/dev/null ; then
            $TIME_SUT_RUN bash -x -c "$@"
        else
            $TIME_SUT_RUN bash -c "$@"
        fi
        return $?
    fi
}

### TODO: a routine (or two?) to wrap local "cp" or remote "scp"
### and/or "ssh|tar", to transfer files in a uniform manner for
### the local and remote tests alike.

do_select() {
    ### Note1: while it is technically possible to pass several SQL sentences
    ### separated by semicolons into this function's parameters, the results
    ### will be garbled because we expect only one request (one header line,
    ### followed by results, as our tail is chopped below).
    ### Note2: As verified on version 10.0.17-MariaDB, the amount of trailing
    ### semicolons does not matter for such non-interactive mysql client use.
    logmsg_do_select \
        "do_select(): $1 ;" >&2
    if [ -z "${DBPASSWD-}" ]; then
        echo "$1" | sut_run "mysql -u ${DBUSER} -D ${DATABASE} -N -s"
    else
        echo "$1" | sut_run "mysql -u ${DBUSER} -p\"${DBPASSWD}\" -D ${DATABASE} -N -s"
    fi
#    DB_OUT="$(echo "$1" | sut_run "mysql -u ${DBUSER} ${DATABASE}")"
#    DB_RES=$?
#    echo "$DB_OUT" | tail -n +2
#    [ $? = 0 -a "$DB_RES" = 0 ]
    return $?
}

do_dumpdb() {
    # Unifies the call to get a dump of our database, either all (by default)
    # or some tables etc. via custom arguments that may be set by the caller.
    logmsg_dumpdb \
        "do_dumpdb(): $@ ;" >&2
    if [ -z "${DBPASSWD-}" ]; then
        sut_run "mysqldump -u ${DBUSER} \"${DATABASE}\" $@"
    else
        sut_run "mysqldump -u ${DBUSER} -p\"${DBPASSWD}\" \"${DATABASE}\" $@"
    fi
#    DB_OUT="$(sut_run "mysqldump -u ${DBUSER} \"${DATABASE}\" $@")"
#    DB_RES=$?
#    echo "$DB_OUT"
#    [ $? = 0 -a "$DB_RES" = 0 ]
    return $?
}

loaddb_file() {
    ### Note: The input (file or stdin) MUST specify 'use ${DATABASE};' in order
    ### to upload data into the database the caller wants (including creation of
    ### one, so it can not be specified as command-line argument)
    ### Note: To avoid race-conditions, CI test scripts generally should not
    ### call this routine directly, but rather use wrappers from testlib-db.sh
    ### to stop database consumers before reinitializing it to avoid surprises.
    DBFILE="$1"
    if [ $# -gt 0 ] && [ x"$1" = x ] ; then
        die "loaddb_file() was called with a present but empty filename argument, check your scripts!"
    fi
    [ -z "$DBFILE" ] && DBFILE='&0'
    ### Note: syntax below 'eval ... "<$DBFILE"' is sensitive to THIS spelling

    ### Due to comments currently don't converge to sut_run(), maybe TODO later
    if isRemoteSUT ; then
        ### Push local SQL file contents to remote system and sleep a bit
        logmsg_loaddb \
            "loaddb_file()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $DBFILE" >&2
        ( sut_run "systemctl start mysql"
          REMCMD="mysql -u ${DBUSER}"
          eval sut_run "${REMCMD}" "<$DBFILE" && \
          { [ "$LOADDB_FILE_REMOTE_SLEEP" -gt 0 ] 2>/dev/null && sleep ${LOADDB_FILE_REMOTE_SLEEP} || true; } && \
          logmsg_loaddb "Updated DB on remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE" ) || \
          CODE=$? die "Could not load database file to remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE"
    else
        logmsg_loaddb \
            "loaddb_file()::local: $DBFILE" >&2
        eval mysql -u "${DBUSER}" "<$DBFILE" > /dev/null || \
            CODE=$? die "Could not load database file: $DBFILE"
    fi
    return 0
}

loaddb_file_params() {
    # This routine allows to import the database file named in "$1",
    # prepending some SQL "set" clauses to customize the "@" SQL vars.
    # Due to limitations of MySQL "source" commmand, this is only local so far.
    if [ $# -eq 0 ]; then
        die "loaddb_file_param() requires parameters"
    fi
    if [ -z "$1" -o ! -e "$1" ]; then
        die "loaddb_file_param(): empty first parameter or file '$1' does not exist"
    fi

    local DBFILE="$1"
    shift

    logmsg_loaddb \
        "loaddb_file_params()::local: $DBFILE $@" >&2

    local E=
    local i=
    for i in "$@"; do
        E="${E}set ${i};"
    done
    E="${E}source $DBFILE;"

    if isRemoteSUT ; then
        ### Push local SQL file contents to remote system and sleep a bit
        logmsg_warn \
            "loaddb_file_params($DBFILE, ...): Currently not implemented for remote execution: '${E}" >&2
        return 32
    else
        mysql -u "${DBUSER}" -e "${E}" > /dev/null || \
            CODE=$? die "Could not load database file: $DBFILE with params: '${E}'"
    fi
    return 0
}

settraps_exit_clear() {
    # Reset the exit() handler to defaults, used in routines below
    for SS in EXIT SIGEXIT 0 ERR SIGERR; do trap "-" "$SS" 2>/dev/null || true; done
    set +E      # unset -o errtrace
}

settraps_nonfatal() {
    # While the legacy common settraps() wraps the caller's custom handler "$*"
    # with an exit(), this one is not fatal by itself, and it does not subshell
    # as it is intended to be used for interrupts, etc. And for settraps() too.
    # It detects and presets the variables that can be used by the caller's
    # trap handler, including one from settraps() below. Variables include:
    #   ERRCODE     Number of upstream exitcode that came into the trap
    #   ERRSIGNAL   Name of the signal as registered (HUP or SIGEXIT etc.)
    #   ERRFILE     File from which the trap was called, if we can guess it
    #   ERRFUNC     Function inside which failure, exit() or die() happened
    #   ERRLINE     Line in source file or function, if any (else empty)
    #   ERRPOS      String that combines available bits of _SCRIPT_NAME
    #               ERRFILE ERRFUNC ERRLINE into meaningful markup
    #   ERRTEXT     String that meaningfully combines ERRPOS ERRCODE ERRSIGNAL
    # The handler built into this routine does not report anything, it just
    # sets the variables above and calls the caller's handle - such as the
    # settrap() which reports stuff and exit()s with some code in the end.

    # Not all trap names are recognized by all shells consistently
    [ -z "${TRAP_SIGNALS-}" ] && TRAP_SIGNALS="EXIT QUIT TERM HUP INT ERR"
    for P in "" SIG; do for S in $TRAP_SIGNALS ; do
        if [ -n "$BASH" ] && [ "$S" = ERR ] ; then
            # If "set -e" aka "set -o errexit" would be used, inherit the trap
            set -o errtrace
        fi
        case "$1" in
            -|"") trap "$1" "$P$S" 2>/dev/null || true ;;
            *)    ERRHANDLER="$*"
                  case "$ERRHANDLER" in
                    *";"|*"; "|*";  ") ;;
                    *)    ERRHANDLER="$ERRHANDLER ;" ;;
                  esac
                  trap 'ERRCODE=$?; ERRSIGNAL="'"$P$S"'"; \
[ -z "${ERRIGNORE-}" ] && ERRIGNORE=no
if [ "${ERRSIGNAL-}" = "ERR" ] || [ "${ERRSIGNAL-}" = "SIGERR" ]; then
    set -o | egrep -i "^errexit.*off$" >/dev/null && ERRIGNORE=yes
fi
if [ "${ERRIGNORE-}" = no ]; then
  if [ -n "${SCRIPTLIB_DIE_ERRCODE-}" ] && [ "${SCRIPTLIB_DIE_ERRCODE-}" -ge 0 ] 2>/dev/null; then
    ERRFILE="$SCRIPTLIB_DIE_FILENAME"
    ERRFUNC="$SCRIPTLIB_DIE_FUNCNAME"
    ERRLINE="$SCRIPTLIB_DIE_LINENO"
    ERRCODE="$SCRIPTLIB_DIE_ERRCODE"
  else
    SCRIPTLIB_DIE_ERRCODE=""
    [ -n "${LINENO-}" ] && [ "${LINENO-}" -gt 0 ] 2>/dev/null && ERRLINE="${LINENO-}" || ERRLINE=""
    ERRFILE="${_SCRIPT_NAME-}"; ERRFUNC=""
    if [ -n "${BASH-}" ] 2>/dev/null; then
        [ -n "${FUNCNAME-}" -o -n "${FUNCNAME[0]-}" ] && ERRFUNC="${FUNCNAME[0]-}" || ERRFUNC=""
        ERRLINE="${BASH_LINENO[0]-}" && [ -n "$ERRLINE" ] || ERRLINE=0
        [ "$ERRLINE" -eq 0 ] && ERRLINE="${LINENO-}"
        [ -n "$ERRLINE" ] && [ "$ERRLINE" -gt 1 ] || ERRLINE=""
        ERRFILE="${BASH_SOURCE[0]-}"
    fi
  fi
  ERRPOS="${ERRFILE-}${ERRLINE:+:$ERRLINE}${ERRFUNC:+ :: $ERRFUNC()}"
  [ "`basename "${_SCRIPT_NAME-}"`" = "`basename "${ERRFILE-}"`" ] || ERRPOS="${_SCRIPT_NAME-} => ${ERRPOS-}"
  ERRTEXT="script (${ERRPOS-}) due to trapped signal (${ERRSIGNAL-}) with exit-code (${ERRCODE-})"
  [ -n "${SCRIPTLIB_DIE_ERRCODE-}" ] && ERRTEXT="${ERRTEXT-}, using die()"
  { (settraps_exit_clear; exit ${ERRCODE-} 2>/dev/null 2>&1); '"$ERRHANDLER"' } ;
else ERRIGNORE=""; fi ;' \
                    "$P$S" 2>/dev/null || true
                  ;;
        esac
    done; done
}

# These variables are tested for equality to "yes" when the trap is processed.
#   When a trap is handled by settraps(), should any message be printed?
[ -n "${SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE-}" ] && [ x"${SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE-}" != "x-" ] || \
    if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_ERROR" ]; then
        SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE="yes"
    else
        SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE="yes"
    fi
#   If SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE==yes and cause is exit(0), still print?
[ -n "${SCRIPTLIB_TRAPWRAP_PRINT_EXIT0-}" ] && [ x"${SCRIPTLIB_TRAPWRAP_PRINT_EXIT0-}" != "x-" ] || \
    if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_DEBUG" ]; then
        SCRIPTLIB_TRAPWRAP_PRINT_EXIT0="yes"
    else
        SCRIPTLIB_TRAPWRAP_PRINT_EXIT0="no"
    fi
#   If SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE==yes and we have a shell with func-call
#   history like bash, print also a stack trace of the failure?
[ -n "${SCRIPTLIB_TRAPWRAP_PRINT_STACKTRACE-}" ] && [ x"${SCRIPTLIB_TRAPWRAP_PRINT_STACKTRACE-}" != "x-" ] || \
    if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_DEBUG" ]; then
        SCRIPTLIB_TRAPWRAP_PRINT_STACKTRACE="yes"
    else
        SCRIPTLIB_TRAPWRAP_PRINT_STACKTRACE="no"
    fi

settraps() {
    # Sets up or clear traps defined in $TRAP_SIGNALS (or falls back to default
    # list of signals) to report the trap, call consumer's handler, and if that
    # routine does not exit() the shell by itself - the wrapper would exit with
    # either that handler's non-zero return code or with original trapped code.
    # The ERR* variables reported here are defined by settraps_nonfatal() above
    # Note that all output (if any) goes to stderr (see end of "if" clause).
    case "$1" in
        -|"") settraps_nonfatal "$1" || true ;;
        *)    ERRHANDLER="$*"
              case "$ERRHANDLER" in
                *";"|*"; "|*";  ") ;;
                *)    ERRHANDLER="$ERRHANDLER ;" ;;
              esac
              settraps_nonfatal 'if [ "${SCRIPTLIB_TRAPWRAP_PRINT_MESSAGE-}" = yes ]\
; then
    echo ""
    _DO_PRINT_STACKTRACE=no
    if [ "${SCRIPTLIB_TRAPWRAP_PRINT_STACKTRACE-}" = yes ] && [ -n "${BASH-}" ]; then
        _DO_PRINT_STACKTRACE=yes
    fi
    if [ "${ERRCODE-}" = 0 ]; then
        _DO_PRINT_STACKTRACE=no
        if [ "${SCRIPTLIB_TRAPWRAP_PRINT_EXIT0-}" = yes ] || \
            [ "${ERRSIGNAL-}" != 0 -a "${ERRSIGNAL-}" != EXIT -a "${ERRSIGNAL-}" != SIGEXIT ] \
        ; then
            LOGMSG_PREFIX="CI-SIGNALTRAP-" logmsg_info "Completing ${ERRTEXT-}"
            _DO_PRINT_STACKTRACE=yes
        fi
    else
        echo ""; echo "!!!!!!!!!"
        LOGMSG_PREFIX="CI-SIGNALTRAP-" logmsg_error "Aborting ${ERRTEXT-}"
        echo "!!!!!!!!!"
    fi
    echo ""
    if [ "${_DO_PRINT_STACKTRACE-}" = yes ]; then
        echo "======= Stack trace and other clues of the end-of-work (code=${ERRCODE-}, sig=${ERRSIGNAL-}):"
        echo "  Depth of sub-shelling (BASH_SUBSHELL) = ${BASH_SUBSHELL-}"
        if [ -z "${FUNCNAME-}" ] || [ -z "${FUNCNAME[0]}" ]; then
            FUNCDEPTH=-1
        else
            FUNCDEPTH="${#FUNCNAME[@]-}" 2>/dev/null && [ -n "$FUNCDEPTH" ] && [ "$FUNCDEPTH" -ge 0 ] || FUNCDEPTH=-1
        fi
        printf "  Depth of function call stack = $FUNCDEPTH : "
        if [ "$FUNCDEPTH" -gt 0 ] 2>/dev/null; then
            printf "::%s" ${FUNCNAME[@]-}
        else
            printf "finished in main body of main script"
        fi
        printf "\n"
        i=0
        while [ "$i" -lt "$FUNCDEPTH" ] ; do
            echo "  ($i)	-> in ${FUNCNAME[$i]-}() called at ${BASH_SOURCE[$i+1]-}:${BASH_LINENO[$i]-}"
            i=$(($i+1))
        done
        echo "	~> in ${ERRFUNC:-main-script-body}() at ${ERRFILE-}:${ERRLINE-}"
        echo "======= End of stack trace, ${_SCRIPT_NAME-}:${LINENO-}"
        echo ""
    fi
fi >&2
settraps_exit_clear
{ '"$ERRHANDLER"' } || exit $?
exit ${ERRCODE-};' \
                || true
              ;;
    esac
}

# Allow to togle sut_run timing in CI-driven builds more easily
# Alas, ATM this worked only with an external "time" but not a built-in
# Also note that "eval time" is very toxic for sut_run() ;)
case "${TIME_SUT_RUN-}" in
    *time*) TIME_SUT_RUN="$TIME_SUT_RUN" ;;
    yes|on) (which time 2>/dev/null) && TIME_SUT_RUN="time" || { TIME_SUT_RUN=""; logmsg_warn "'time' not found in PATH"; } ;;
    no|off) TIME_SUT_RUN="" ;;
    *|auto|-|"") TIME_SUT_RUN="auto" ;;
esac

[ "$TIME_SUT_RUN" = auto ] && \
if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_TIME_SUT_RUN" ] ; then
    logmsg_info "CI_DEBUG is $CI_DEBUG >= $CI_DEBUGLEVEL_TIME_SUT_RUN : enabling timing of sut_run()!" >&2
    if [ -x "`which time 2>/dev/null`" ]; then
        TIME_SUT_RUN="time"
    else
        logmsg_warn "'time' not found in PATH"
        TIME_SUT_RUN=""
    fi
else
    TIME_SUT_RUN=""
fi

# Allow to togle shell-tracing in CI-driven builds more easily
if [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_TRACEEXEC" ] ; then
    logmsg_info "CI_DEBUG is $CI_DEBUG >= $CI_DEBUGLEVEL_TRACEEXEC : enabling source-code tracing!" >&2
    set -x
fi

:

