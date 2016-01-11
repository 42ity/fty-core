# shell include file: scriptlib.sh
#
# Copyright (C) 2014-2015 Eaton
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
#           workspace which is under test. Mainly for inclusion in $BIOS
#           ./tests/CI scripts.
#           The variable values may be set by caller or an earlier stage
#           in script interpretation, otherwise they get defaulted here.

# A bash-ism, should set the exitcode of the rightmost failed command
# in a pipeline, otherwise e.g. exitcode("false | true") == 0
if [ -n "${BASH-}" ]; then
    set -o pipefail 2>/dev/null || true
    echo_E() { echo -E "$@"; }
    echo_e() { echo -e "$@"; }
else
    echo_E() { /bin/echo -E "$@"; }
    echo_e() { /bin/echo -e "$@"; }
fi

### Some variables might not be initialized
set +u

### Store some important CLI values
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
[ -z "${BASE_URL-}" ] && BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
export SUT_IS_REMOTE SUT_USER SUT_HOST SUT_SSH_PORT SUT_WEB_PORT SUT_WEB_SCHEMA BASE_URL

### Should the test suite break upon first failed test?
[ x"${CITEST_QUICKFAIL-}" = x- ] && CITEST_QUICKFAIL=""
[ x"${CITEST_QUICKFAIL-}" != xyes ] && CITEST_QUICKFAIL=no
export CITEST_QUICKFAIL

### For remote tests and general database initalization, we want some quiet
### time after initializing the database. For tests that loop with uploads
### of small DB snippets, we want to minimize such sleeps!
[ -z "${LOADDB_FILE_REMOTE_SLEEP-}" ] && LOADDB_FILE_REMOTE_SLEEP=5
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

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-"
### Default debugging/info/warning level for this lifetime of the script
### Messages are printed if their assigned level is at least CI_DEBUG
### The default of "3" allows INFO messages to be printed or easily
### suppressed by change to a smaller number. The level of "2" is default
### for warnings and "1" for errors, and a "0" would likely hide most
### such output. "Yes" bumps up a high level to enable even greater debug
### details, while "No" only leaves the default errors and warnings.
[ -z "${CI_DEBUG-}" ] && CI_DEBUG=3
case "$CI_DEBUG" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])     CI_DEBUG=99 ;;
    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]) CI_DEBUG=2 ;;
esac
[ "$CI_DEBUG" -ge 0 ] 2>/dev/null || CI_DEBUG=3

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
default_posval CI_DEBUGLEVEL_ECHO       0
        # By default, do echo unless $1 says otherwise

default_posval CI_DEBUGLEVEL_ERROR      1
default_posval CI_DEBUGLEVEL_WARN       2
default_posval CI_DEBUGLEVEL_INFO       3
default_posval CI_DEBUGLEVEL_LOADDB     5
default_posval CI_DEBUGLEVEL_DUMPDB     3
default_posval CI_DEBUGLEVEL_SELECT     3
default_posval CI_DEBUGLEVEL_RUN        4
default_posval CI_DEBUGLEVEL_DEBUG      5
default_posval CI_DEBUGLEVEL_PIPESNIFFER 5

logmsg_echo() {
    ### Optionally echoes a message, based on current debug-level
    ### Does not add any headers to the output line
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
        echo_E "${LOGMSG_PREFIX}FATAL: ${_SCRIPT_PATH}:" "$LINE" >&2
    done
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
    if [ "${SUT_IS_REMOTE-}" = yes ]; then
        ### Yes, we are testing a remote box or a VTE,
        ### and have a cached decision or explicit setting
        return 0
    else
        ### No, test is local
        return 1
    fi

    if  [ -z "$SUT_IS_REMOTE" -o x"$SUT_IS_REMOTE" = xauto ] && \
        [ -n "$SUT_HOST" -a -n "$SUT_SSH_PORT" ] && \
        [ x"$SUT_HOST" != xlocalhost -a x"$SUT_HOST" != x127.0.0.1 ] \
    ; then
        ### TODO: Maybe a better test is needed e.g. "localhost and port==22"
        SUT_IS_REMOTE=yes
        return 0
        ### NOTE: No automatic decision for "no" since the needed variables
        ### may become defined later.
    fi
}

sut_run() {
    ### This tries to run a command either locally or externally via SSH,
    ### depending on what we are testing (local or remote System Under Test).
    ### If the first argument ($1) is "-t" then this requests a TTY for SSH.
    ### NOTE: By current construction this may fail for parameters that are
    ### not one token aka "$1".
    if isRemoteSUT ; then
        logmsg_info "$CI_DEBUGLEVEL_RUN" \
            "sut_run()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $@" >&2
        SSH_TERMINAL_REQUEST=""
        [ "$1" = "-t" ] && shift && SSH_TERMINAL_REQUEST="-t -t" #" -o RequestTTY=true"
        [ "$CI_DEBUG" -gt 0 ] 2>/dev/null && \
            REMCMD="sh -x -c \"$@\"" ||
            REMCMD="sh -c \"$@\""
        ssh $SSH_TERMINAL_REQUEST -p "${SUT_SSH_PORT}" -l "${SUT_USER}" "${SUT_HOST}" "$@"
        return $?
    else
        [ "$1" = "-t" ] && shift        # Ignore for local host
        logmsg_info "$CI_DEBUGLEVEL_RUN" \
            "sut_run()::local: $@" >&2
        if [ "$CI_DEBUG" -gt 0 ] 2>/dev/null ; then
            sh -x -c "$@"
        else
            sh -c "$@"
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
    logmsg_info "$CI_DEBUGLEVEL_SELECT" \
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
    logmsg_info "$CI_DEBUGLEVEL_DUMPDB" \
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
    DBFILE="$1"
    if [ $# -gt 0 ] && [ x"$1" = x ] ; then
        die "loaddb_file() was called with a present but empty filename argument, check your scripts!"
    fi
    [ -z "$DBFILE" ] && DBFILE='&0'
    ### Note: syntax below 'eval ... "<$DBFILE"' is sensitive to THIS spelling

    ### Due to comments currently don't converge to sut_run(), maybe TODO later
    if isRemoteSUT ; then
        ### Push local SQL file contents to remote system and sleep a bit
        logmsg_info "$CI_DEBUGLEVEL_LOADDB" \
            "loaddb_file()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $DBFILE" >&2
        ( sut_run "systemctl start mysql"
          REMCMD="mysql -u ${DBUSER}"
          eval sut_run "${REMCMD}" "<$DBFILE" && \
          { [ "$LOADDB_FILE_REMOTE_SLEEP" -gt 0 ] 2>/dev/null && sleep ${LOADDB_FILE_REMOTE_SLEEP} || true; } && \
          logmsg_info "Updated DB on remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE" ) || \
          CODE=$? die "Could not load database file to remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE"
    else
        logmsg_info "$CI_DEBUGLEVEL_LOADDB" \
            "loaddb_file()::local: $DBFILE" >&2
        eval mysql -u "${DBUSER}" "<$DBFILE" > /dev/null || \
            CODE=$? die "Could not load database file: $DBFILE"
    fi
    return 0
}

loaddb_file_params() {
    # 
    if [ $# -eq 0 ]; then
        die "loaddb_file_param() requires parameters"
    fi  
    if [ -z "$1" -o ! -e "$1" ]; then
        die "loaddb_file_param(): empty first parameter or file '$1' does not exist."
    fi

    local DBFILE="$1"
    shift
    
    logmsg_info "$CI_DEBUGLEVEL_LOADDB" \
        "loaddb_file_params()::local: $DBFILE $@" >&2

    local E=
    local i=
    for i in $@; do
        E="${E}set ${i};"
    done
    E="${E}source $DBFILE;"
    mysql -u "${DBUSER}" -e "${E}" > /dev/null || \
        CODE=$? die "Could not load database file: $DBFILE with params $@"
    return 0        
}

settraps() {
        # Not all trap names are recognized by all shells consistently
        [ -z "${TRAP_SIGNALS-}" ] && TRAP_SIGNALS="EXIT QUIT TERM HUP INT"
        for P in "" SIG; do for S in $TRAP_SIGNALS ; do
                case "$1" in
                -|"") trap "$1" $P$S 2>/dev/null || true ;;
                *)    trap 'ERRCODE=$?; { '"$*"' ;}; exit $ERRCODE;' $P$S 2>/dev/null || true ;;
                esac
        done; done
}
