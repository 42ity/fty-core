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
        SCRIPTDIR="`dirname ${_SCRIPT_NAME}`"

    if [ -z "$CHECKOUTDIR" ]; then
        case "$SCRIPTDIR" in
            */tests/CI|tests/CI)
               CHECKOUTDIR="$(realpath $SCRIPTDIR/../..)" || \
               CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
               CHECKOUTDIR="" ;;
            */tools|tools)
               CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tools$||' )" || \
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
        logmsg_error "Cannot find '$BUILDSUBDIR/config.status', did you run configure?"
        logmsg_error "Search path checked: $CHECKOUTDIR, $PWD"
        ls -lad "$BUILDSUBDIR/config.status" "$CHECKOUTDIR/config.status" \
            "$PWD/config.status" >&2
        RES=1
        if [ "$NEED_BUILDSUBDIR" = yes ]; then
            exit $RES
        fi
    fi

    return $RES
}

