#!/bin/bash

# Description: Print the details of the current repository, if any
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

# Caller can 'export GIT_DETAILS_BLANK=yes' to just generate an empty set
if [ x"$GIT_DETAILS_BLANK" = xyes ]; then
    for V in PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
	 PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS \
	 PACKAGE_BUILD_HOST_UNAME PACKAGE_BUILD_HOST_NAME \
	 PACKAGE_BUILD_HOST_OS PACKAGE_BUILD_TSTAMP \
	 PACKAGE_GIT_TAGGED \
    ; do
	echo "$V=\"\";"
	echo "${V}_ESCAPED=\"\";"
    done
    exit 0
fi

[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="`dirname $0`/JSON.sh"
[ -z "$GIT" -o ! -x "$GIT" ] && GIT="$(which git 2>/dev/null | head -1)"

if [ -x "$GIT" ] && $GIT --help >/dev/null 2>&1 ; then
    PACKAGE_GIT_ORIGIN="$($GIT config --get remote.origin.url)"
    if [ $? != 0 ]; then
	echo "SKIPPED: can not get Git metadata in '`pwd`'" >&2
	exit 0
    fi
    PACKAGE_GIT_BRANCH="$($GIT rev-parse --abbrev-ref HEAD)"
    PACKAGE_GIT_TSTAMP="$($GIT log -n 1 --format='%ct')"
    PACKAGE_GIT_HASH_S="$($GIT log -n 1 --format='%h')"
    PACKAGE_GIT_HASH_L="$($GIT rev-parse --verify HEAD)"
    PACKAGE_GIT_STATUS="$($GIT status -s)"

    ### Ported from bios-infra::obs-service_git_nas.sh
    PACKAGE_GIT_TAGGED="$($GIT describe --tags 2>/dev/null)"
    PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED/[tv]/}"  # kill the v or t from version
    PACKAGE_GIT_TAGGED="${PACKAGE_GIT_TAGGED/-/\~}"

    PACKAGE_BUILD_HOST_UNAME="`uname -a`"
    PACKAGE_BUILD_HOST_NAME="`uname -n`"
    PACKAGE_BUILD_HOST_OS="`uname -s -r -v`"
    PACKAGE_BUILD_TSTAMP="`TZ=UTC date -u '+%s'`"

    [ ! -x "$JSONSH" ] && \
	echo "FAILED to use JSON.sh from '$JSONSH'" >&2 && exit 3

    for V in PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
	 PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS \
	 PACKAGE_BUILD_HOST_UNAME PACKAGE_BUILD_HOST_NAME \
	 PACKAGE_BUILD_HOST_OS PACKAGE_BUILD_TSTAMP \
	 PACKAGE_GIT_TAGGED \
    ; do
        eval echo $V=\\\"'$'$V\\\""\\;"
	VE="${V}_ESCAPED"
	eval $VE=\"$(eval echo -E \"\$\{$V\}\" | $JSONSH -Q)\"
        eval echo $VE=\\\"'$'$VE\\\""\\;"
    done
else
    echo "FAILED to execute 'git' program (tried '$GIT')" >&2
    exit 2
fi

exit 0
