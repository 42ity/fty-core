# shell include file: scriptlib.sh
#
# Copyright (C) 2014-2015 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description:
#       Determine the directory name variables relevant for compiled
#       workspace which is under test. Mainly for inclusion in $BIOS
#       ./tests/CI scripts.
#       The variable values may be set by caller or an earlier stage
#       in script interpretation, otherwise they get defaulted here.

# A bash-ism, should set the exitcode of the rightmost failed command
# in a pipeline, otherwise e.g. exitcode("false | true") == 0
if [ "$BASH" ]; then
    set -o pipefail 2>/dev/null || true
fi

### Some variables might not be initialized
set +u

### Store some important CLI values
[ -z "$_SCRIPT_NAME" ] && _SCRIPT_NAME="$0"
_SCRIPT_ARGS="$*"
_SCRIPT_ARGC="$#"

### Just a tag for pretty output below
[ -z "$_SCRIPT_TYPE" ] && case "${_SCRIPT_NAME}" in
    ci-*|CI-*) _SCRIPT_TYPE="Test" ;;
    *) _SCRIPT_TYPE="Program" ;;
esac

### Database credentials
[ -z "$DBUSER" ] && DBUSER=root
[ -z "$DATABASE" ] && DATABASE=box_utf8
export DBUSER DATABASE

### REST API (and possibly non-privileged SSH) user credentials
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"
export BIOS_USER BIOS_PASSWD

### Variables for remote testing - avoid "variable not defined" errors
[ -z "$SUT_IS_REMOTE" ] && SUT_IS_REMOTE="auto" # auto|yes|no
[ -z "$SUT_USER" ] && SUT_USER="root"   # Username on remote SUT
[ -z "$SUT_HOST" ] && SUT_HOST=""       # Hostname or IP address
[ -z "$SUT_SSH_PORT" ] && SUT_SSH_PORT=""       # SSH (maybe via NAT)
[ -z "$SUT_WEB_PORT" ] && SUT_WEB_PORT=""       # TNTNET (maybe via NAT)
export SUT_IS_REMOTE SUT_HOST SUT_SSH_PORT SUT_WEB_PORT

### Should the test suite break upon first failed test?
[ x"$CITEST_QUICKFAIL" != xyes ] && CITEST_QUICKFAIL=no

### Set the default language (e.g. for CI apt-get to stop complaining)
[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

determineDirs() {
    ### Note: a set, but invalid, value will cause an error to the caller
    [ -n "$SCRIPTDIR" -a -d "$SCRIPTDIR" ] || \
        SCRIPTDIR="$(cd "`dirname ${_SCRIPT_NAME}`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname ${_SCRIPT_NAME}`" || \
        SCRIPTDIR="$(realpath "`dirname ${_SCRIPT_NAME}`")" || \
        SCRIPTDIR="`dirname ${_SCRIPT_NAME}`"

    if [ -z "$CHECKOUTDIR" ]; then
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
    [ -z "$CHECKOUTDIR" -a -d ~/project ] && CHECKOUTDIR=~/project

    if [ -z "$BUILDSUBDIR" ]; then
        ### Keep a caller-defined BUILDSUBDIR value even if it is not made yet
        [ ! -d "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
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

    [ -z "$MAKELOG" ] && MAKELOG="$BUILDSUBDIR/make.output"
    export MAKELOG

    ### Ultimate status: if false, then the paths are non-development
    [ -n "$SCRIPTDIR" -a -n "$CHECKOUTDIR" -a -n "$BUILDSUBDIR" ] && \
    [ -d "$SCRIPTDIR" -a -d "$CHECKOUTDIR" -a -d "$BUILDSUBDIR" ] && \
    [ -x "$BUILDSUBDIR/config.status" ]
}

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-"
logmsg_info() {
    echo "${LOGMSG_PREFIX}INFO: ${_SCRIPT_NAME}:" "$@"
}

logmsg_warn() {
    echo "${LOGMSG_PREFIX}WARN: ${_SCRIPT_NAME}:" "$@" >&2
}

logmsg_error() {
    echo "${LOGMSG_PREFIX}ERROR: ${_SCRIPT_NAME}:" "$@" >&2
}

die() {
    CODE="${CODE-1}"
    [ "$CODE" -ge 0 ] 2>/dev/null || CODE=1
    for LINE in "$@" ; do
        echo "${LOGMSG_PREFIX}FATAL: ${_SCRIPT_NAME}:" "$LINE" >&2
    done
    exit $CODE
}

determineDirs_default() {
    determineDirs
    RES=$?

    [ "$NEED_CHECKOUTDIR" = no ] || \
    if [ -n "$CHECKOUTDIR" -a -d "$CHECKOUTDIR" ]; then
        echo "${LOGMSG_PREFIX}INFO: ${_SCRIPT_TYPE} '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."
    else
        echo "${LOGMSG_PREFIX}WARN: ${_SCRIPT_TYPE} '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' can not detect a CHECKOUTDIR value..." >&2
        RES=1
        if [ "$NEED_CHECKOUTDIR" = yes ]; then
            exit $RES
        fi
    fi

    [ "$NEED_BUILDSUBDIR" = no ] || \
    if [ -n "$BUILDSUBDIR" -a -d "$BUILDSUBDIR" -a -x "$BUILDSUBDIR/config.status" ]; then
        logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR'"
    else
        [ "$NEED_BUILDSUBDIR" = yes ] && _LM=logmsg_error || _LM=logmsg_warn
        ${_LM} "Cannot find '$BUILDSUBDIR/config.status', did you run configure?"
        ${_LM} "Search path checked: '$CHECKOUTDIR', '$PWD'"
        unset _LM
        RES=1
        if [ "$NEED_BUILDSUBDIR" = yes ]; then
            ls -lad "$BUILDSUBDIR/config.status" "$CHECKOUTDIR/config.status" \
                "$PWD/config.status" >&2
            exit $RES
        fi
    fi

    return $RES
}

isRemoteSUT() {
    if [ "$SUT_IS_REMOTE" = yes ]; then
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
    ### This tries to run a command either locally or externally via SSH
    ### depending on what we are testing (local or remote System Under Test)
    ### NOTE: By current construction this may fail for parameters that are
    ### not one token aka "$1"
    if isRemoteSUT ; then
        logmsg_info "sut_run()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $@" >&2
        REMCMD="sh -x -c \"$@\""
        ssh -p "${SUT_SSH_PORT}" -l "${SUT_USER}" "${SUT_HOST}" "$@"
        return $?
    else
        logmsg_info "sut_run()::local: $@" >&2
        sh -x -c "$@"
        return $?
    fi
}

### TODO: a routine (or two?) to wrap local "cp" or remote "scp"
### and/or "ssh|tar", to transfer files in a uniform manner for
### the local and remote tests alike.

do_select() {
    logmsg_info "do_select(): $1;" >&2
    DB_OUT="$(echo "$1;" | sed 's,;*$,;,g' | sut_run "mysql -u ${DBUSER} ${DATABASE}")"
    DB_RES=$?
    echo "$DB_OUT" | tail -n +2
    [ $? = 0 -a "$DB_RES" = 0 ]
    return $?
}

loaddb_file() {
    DBFILE="$1"
    [ -z "$DBFILE" ] && DBFILE='&0'
    ### Note: syntax below 'eval ... "<$DBFILE"' is sensitive to THIS spelling

    ### Due to comments currently don't converge to sut_run(), maybe TODO later
    if isRemoteSUT ; then
        ### Push local SQL file contents to remote system and sleep a bit
        logmsg_info "loaddb_file()::ssh(${SUT_HOST}:${SUT_SSH_PORT}): $DBFILE" >&2
        ( sut_run "systemctl start mysql"
          REMCMD="mysql -u ${DBUSER}"
          eval sut_run "${REMCMD}" "<$DBFILE" && \
          sleep 20 && echo "Updated DB on remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE" ) || \
          CODE=$? die "Could not load database file to remote system $SUT_HOST:$SUT_SSH_PORT: $DBFILE"
    else
        logmsg_info "loaddb_file()::local: $DBFILE" >&2
        eval mysql -u "${DBUSER}" "<$DBFILE" > /dev/null || \
            CODE=$? die "Could not load database file: $DBFILE"
    fi
    return 0
}
