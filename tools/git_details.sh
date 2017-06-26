#!/bin/bash
#
#   Copyright (c) 2014-2016 Eaton
#
#   This file is part of the Eaton $BIOS project.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    git_details.sh
#  \brief   Print the details of the current repository
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Print the details of the current repository, if any,
#           and of build-host and current build-timestamp, to the
#           stdout as a shell-includable markup.
#

# Establish standard environment
LANG=C
LC_ALL=C
TZ=UTC
export LANG LC_ALL TZ

### We need to convert a potentially multi-line value like "git status" into
### a single-line token for C macros or Makefiles; our JSON.sh can do that.
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="`dirname $0`/JSON.sh"
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="/usr/share/fty/scripts/JSON.sh"
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="/usr/share/bios/scripts/JSON.sh"
[ ! -x "$JSONSH" ] && \
    echo "GIT_DETAILS-FATAL: FAILED to use JSON.sh from '$JSONSH'" >&2 && exit 3

[ -z "$GIT" -o ! -x "$GIT" ] && GIT="$(which git 2>/dev/null | head -1)"
if [ -n "$GIT" -a -x "$GIT" ] && $GIT --help >/dev/null 2>&1; then :
else
    echo "GIT_DETAILS-WARN: FAILED to execute 'git' program (tried '$GIT')" >&2
    GIT="git"
fi

[ -z "$DATE" -o ! -x "$DATE" ] && DATE="$(which date 2>/dev/null | head -1)"
[ -z "$DATE" -o ! -x "$DATE" ] && DATE="$(which gdate 2>/dev/null | head -1)"
[ -n "$DATE" -a -x "$DATE" ] || DATE=date

PACKAGE_BUILD_TSTAMP_ISO8601=""

reportVar() {
    # Argument is the name of the variable to report in "original"
    # and "escaped" form
    V="$1"
    VE="${V}_ESCAPED"

    if [ -z "$V" ]; then
        echo "$V=\"\";"
        echo "$VE=\"\";"
    else
        eval echo -E $V=\\\"\"'$'$V\"\\\""\\;"
        eval $VE=\"$(eval echo -E \"'$'\{$V\}\" | $JSONSH -Q)\"
        eval echo -E $VE=\\\"'$'$VE\\\""\\;"
    fi
    unset $VE
    unset VE
    unset V
    return 0
}

reportBuildTimestamp() {
    echo "GIT_DETAILS-INFO: Recording the build timestamp..." >&2
    # Packaging metadata: Current timestamp at the build host
    # (as of compilation)

    # May be passed by caller like the obs-service_git_nas.sh script
    # to use some unified value across a mass build, if needed
    if [ -z "$PACKAGE_BUILD_TSTAMP" ] ; then
        PACKAGE_BUILD_TSTAMP="`TZ=UTC $DATE -u '+%s'`" || \
        PACKAGE_BUILD_TSTAMP="`TZ=UTC $DATE '+%s'`" || \
        return 1
    fi

    [ -n "$PACKAGE_BUILD_TSTAMP" ] && [ -z "$PACKAGE_BUILD_TSTAMP_ISO8601" ] && \
        PACKAGE_BUILD_TSTAMP_ISO8601="`TZ=UTC date -u -d "@${PACKAGE_BUILD_TSTAMP}" '+%Y%m%dT%H%M%SZ'`"

    for PV in \
        PACKAGE_BUILD_TSTAMP                    \
        PACKAGE_BUILD_TSTAMP_ISO8601            \
    ; do
        reportVar "$PV"
    done
    return 0
}

reportBuildHost() {
    echo "GIT_DETAILS-INFO: Getting buildhost attributes..." >&2

    # Packaging metadata: Full 'uname -a' of the building host
    PACKAGE_BUILD_HOST_UNAME="`uname -a`"
    # Packaging metadata: Hostname of the building host
    PACKAGE_BUILD_HOST_NAME="`uname -n`"
    # Packaging metadata: OS/kernel of the building host
    PACKAGE_BUILD_HOST_OS="`uname -s -r -v`"

    for PV in \
        PACKAGE_BUILD_HOST_UNAME                \
        PACKAGE_BUILD_HOST_NAME                 \
        PACKAGE_BUILD_HOST_OS                   \
    ; do
        reportVar "$PV"
    done
    return 0
}

reportGitInfo() {
    # Caller can 'export GIT_DETAILS_BLANK=yes' to just generate an empty set
    GIT_ERRORED=no
    GITRES=0

    PACKAGE_GIT_ORIGIN=""
    PACKAGE_GIT_BRANCH=""
    PACKAGE_GIT_TSTAMP=""
    PACKAGE_GIT_TSTAMP_ISO8601=""
    PACKAGE_GIT_HASH_S=""
    PACKAGE_GIT_HASH_L=""
    PACKAGE_GIT_STATUS=""
    PACKAGE_GIT_TAGGED=""

    if [ -z "$GIT" ]; then
        GIT_ERRORED=yes
        GITRES=1
    else
        # Packaging metadata: URL of the Git origin repository
        # (parent of the build workspace)
        PACKAGE_GIT_ORIGIN="$($GIT config --get remote.origin.url)"
        if [ $? != 0 ]; then
            echo "GIT_DETAILS-INFO: SKIPPED: can not get Git metadata in '`pwd`'" >&2
            PACKAGE_GIT_ORIGIN=""
            GIT_ERRORED=yes
            GITRES=2
        fi
    fi

    if [ "$GIT_ERRORED" = no ]; then
        echo "GIT_DETAILS-INFO: Getting Git workspace attributes..." >&2

        # Packaging metadata: Git branch in the build workspace repository
        PACKAGE_GIT_BRANCH="$($GIT rev-parse --abbrev-ref HEAD)"
        # Packaging metadata: Git timestamp of the commit used for the build
        PACKAGE_GIT_TSTAMP="$($GIT log -n 1 --format='%ct')" && \
        PACKAGE_GIT_TSTAMP_ISO8601="`TZ=UTC date -u -d "@${PACKAGE_GIT_TSTAMP}" '+%Y%m%dT%H%M%SZ'`"
        # Packaging metadata: Git short-hash of the commit used for the build
        PACKAGE_GIT_HASH_S="$($GIT log -n 1 --format='%h')"
        # Packaging metadata: Git long-hash of the commit used for the build
        PACKAGE_GIT_HASH_L="$($GIT rev-parse --verify HEAD)"
        # Packaging metadata: short list of possible differences against the
        # committed repository state
        PACKAGE_GIT_STATUS="$($GIT status -s)"

        _B=''
        _B_RES=-1
        if [ "$PACKAGE_GIT_BRANCH" = "HEAD" ]; then
            echo "GIT_DETAILS-INFO: This workspace is a 'detached HEAD'," \
                "trying to detect the real source branch name..." >&2

            if [ -n "$BRANCH" -a -n "$BUILDMACHINE" ]; then
                echo "GIT_DETAILS-INFO: envvars set by Jenkins are detected;" \
                    "will rely on them (using '$BRANCH')" >&2
                _B="$BRANCH"
                [ -n "$BRANCH" -a x"$BRANCH" != xHEAD ]
                _B_RES=?
            fi

            _B_FETCH_HEAD=""
            [ $_B_RES != 0 -o -z "$_B" ] && \
            if [ -d ".git" -a -f ".git/FETCH_HEAD" -a\
                 -n "$PACKAGE_GIT_HASH_L" ]; then
                echo "GIT_DETAILS-INFO: Looking for PACKAGE_GIT_BRANCH in .git/FETCH_HEAD..." >&2
                _B="`grep "$PACKAGE_GIT_HASH_L" .git/FETCH_HEAD | grep -w branch | sed 's,^[^ ]* *branch '"'"'\(.*\)'"'"' of .*$,\1,'`" && [ -n "$_B" ]
                _B_RES=$?
                if [ $_B_RES = 0 ] && [ "`echo "$_B" | wc -l`" -gt 1 ] ; then
                    # Note: pedantically, this rule can also be hit if a branch
                    # is branched and no commits are added to either one - both
                    # HEADs are same commit id then... and then we have little
                    # reason to choose or reject either one. Maybe `|head -1` ?
                    # We fall-back to this in the end of this test suite.
                    echo "GIT_DETAILS-WARN: Looking for PACKAGE_GIT_BRANCH in .git/FETCH_HEAD returned more than one line (octopus, shoo!) :" >&2
                    echo "$_B" >&2
                    _B_FETCH_HEAD="$_B"
                    _B=""
                    _B_RES=1
                fi
            fi

            [ $_B_RES != 0 -o -z "$_B" ] && \
            if [ -n "$PACKAGE_GIT_HASH_S" ]; then
                echo "GIT_DETAILS-INFO: Looking for PACKAGE_GIT_BRANCH in 'git branch' info..." >&2
                _B="`git branch -a -v | grep -w "$PACKAGE_GIT_HASH_S" | egrep -v "^\* \(detached from $PACKAGE_GIT_HASH_S\)" | awk '{print $1}' | sed 's,^remotes/,,'`"
                _B_RES=$?
            fi

            [ $_B_RES != 0 -o -z "$_B" ] && \
            if [ -s ".git_details" -a -r ".git_details" ]; then
                echo "GIT_DETAILS-INFO: Looking for PACKAGE_GIT_BRANCH" \
                    "in older .git_details..." >&2
                _B="`source .git_details && echo "$PACKAGE_GIT_BRANCH"`"
                _B_RES=$?
            fi

            [ $_B_RES != 0 -o -z "$_B" ] && \
            if [ -n "$_B_FETCH_HEAD" ]; then \
                echo "GIT_DETAILS-INFO: Fall back to the first hit from .git/FETCH_HEAD as the PACKAGE_GIT_BRANCH..." >&2
                _B="`echo "$_B_FETCH_HEAD" | head -1`"
                _B_RES=$?
            fi

            [ $_B_RES = 0 -a -n "$_B" ] && \
                echo "GIT_DETAILS-INFO: This workspace is a 'detached HEAD'," \
                    "but its commit-id matches the head of known branch '$_B'" >&2 && \
                PACKAGE_GIT_BRANCH="$_B"

            unset _B_FETCH_HEAD
        fi
        unset _B _B_RES

        if [ "$PACKAGE_GIT_BRANCH" = "HEAD" ]; then
            echo "GIT_DETAILS-WARN: This workspace is a 'detached HEAD', and" \
                "we could not reliably detect any predecessor branch" >&2
        fi

        ### Ported from bios-infra::obs-service_git_nas.sh
        PACKAGE_GIT_TAGGED="$($GIT describe --tags 2>/dev/null)"
        ### TODO: is this still needed? The pattern ported from git_nas
        ### is absent nowadays... maybe it was even never implemented...
        ### Kill the "v" or "t" from version or tag
        #PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED/-[tv]/-}"
        #PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED//-/\~}"
    fi

    if [ "$GIT_ERRORED" = no -o x"$GIT_DETAILS_BLANK" = xyes ]; then
        for PV in \
            PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
            PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS \
            PACKAGE_GIT_TAGGED PACKAGE_GIT_TSTAMP_ISO8601 \
        ; do
            reportVar "$PV"
        done
        return 0
    else
        return $GITRES
    fi
}

case "$1" in
    build-host)
        reportBuildHost ;;
    build-timestamp)
        reportBuildTimestamp ;;
    build-source)
        reportGitInfo ;;
    *)
        reportBuildHost
        reportBuildTimestamp

        # NOTE: This must be the last action - it returns the possible error
        # exit-codes in Git metadata detection, if the caller cares about that
        reportGitInfo
        exit
        ;;
esac
