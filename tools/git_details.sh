#!/bin/bash

# Description: Print the details of the current repository, if any,
# and of build-host and curent build-timestamp, to the stdout as a
# shell-includable markup.
#
#   Copyright (c) 2014 Eaton Corporation <www.eaton.com>
#
#   This file is part of the Eaton $BIOS project.
#
#   This is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This software is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   Author(s): Jim Klimov <EvgenyKlimov@eaton.com>

# Establish standard environment
LANG=C
LC_ALL=C
TZ=UTC
export LANG LC_ALL TZ

### We need to convert a potentially multi-line value like "git status" into
### a single-line token for C macros or Makefiles; our JSON.sh can do that.
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="`dirname $0`/JSON.sh"
[ ! -x "$JSONSH" ] && \
    echo "FAILED to use JSON.sh from '$JSONSH'" >&2 && exit 3

[ -z "$GIT" -o ! -x "$GIT" ] && GIT="$(which git 2>/dev/null | head -1)"
if [ -n "$GIT" -a -x "$GIT" ] && $GIT --help >/dev/null 2>&1; then :
else
    echo "FAILED to execute 'git' program (tried '$GIT')" >&2
    GIT="git"
fi

reportVar() {
    # Argument is the name of the variable to report in "original"
    # and "escaped" form
    V="$1"
    VE="${V}_ESCAPED"

    if [ -z "$V" ]; then
	echo "$V=\"\";"
	echo "$VE=\"\";"
    else
        eval echo $V=\\\"'$'$V\\\""\\;"
	eval $VE=\"$(eval echo -E \"\$\{$V\}\" | $JSONSH -Q)\"
        eval echo $VE=\\\"'$'$VE\\\""\\;"
    fi
    unset $VE
    unset VE
    unset V
    return 0
}

reportBuildTimestamp() {
    echo "INFO: Recording the build timestamp..." >&2
    # Packaging metadata: Current timestamp at the build host
    # (as of compilation)

    # May be passed by caller like the obs-service_git_nas.sh script
    # to use some unified value across a mass build, if needed
    if [ -z "$PACKAGE_BUILD_TSTAMP" ] ; then
	PACKAGE_BUILD_TSTAMP="`TZ=UTC date -u '+%s'`"
    fi

    reportVar PACKAGE_BUILD_TSTAMP
    return 0
}

reportBuildHost() {
    echo "INFO: Getting buildhost attributes..." >&2

    # Packaging metadata: Full 'uname -a' of the building host
    PACKAGE_BUILD_HOST_UNAME="`uname -a`"
    # Packaging metadata: Hostname of the building host
    PACKAGE_BUILD_HOST_NAME="`uname -n`"
    # Packaging metadata: OS/kernel of the building host
    PACKAGE_BUILD_HOST_OS="`uname -s -r -v`"

    for PV in \
	PACKAGE_BUILD_HOST_UNAME	\
	PACKAGE_BUILD_HOST_NAME		\
	PACKAGE_BUILD_HOST_OS		\
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
	    echo "SKIPPED: can not get Git metadata in '`pwd`'" >&2
	    PACKAGE_GIT_ORIGIN=""
	    GIT_ERRORED=yes
	    GITRES=2
	fi
    fi

    if [ "$GIT_ERRORED" = no ]; then
	echo "INFO: Getting Git workspace attributes..." >&2

	# Packaging metadata: Git branch in the build workspace repository
	PACKAGE_GIT_BRANCH="$($GIT rev-parse --abbrev-ref HEAD)"
	# Packaging metadata: Git timestamp of the commit used for the build
	PACKAGE_GIT_TSTAMP="$($GIT log -n 1 --format='%ct')"
	# Packaging metadata: Git short-hash of the commit used for the build
	PACKAGE_GIT_HASH_S="$($GIT log -n 1 --format='%h')"
	# Packaging metadata: Git long-hash of the commit used for the build
	PACKAGE_GIT_HASH_L="$($GIT rev-parse --verify HEAD)"
	# Packaging metadata: short list of possible differences against the
	# committed repository state
	PACKAGE_GIT_STATUS="$($GIT status -s)"

	### Ported from bios-infra::obs-service_git_nas.sh
	PACKAGE_GIT_TAGGED="$($GIT describe --tags 2>/dev/null)"
	PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED/[tv]/}"  # kill the v or t from version
	PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED/-/\~}"
    fi

    if [ "$GIT_ERRORED" = no -o x"$GIT_DETAILS_BLANK" = xyes ]; then
	for PV in \
	    PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
	    PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS \
	    PACKAGE_GIT_TAGGED \
	; do
	    reportVar "$PV"
	done
	return 0
    else
	return $GITRES
    fi
}

reportBuildHost
reportBuildTimestamp
reportGitInfo
