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

[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="`dirname $0`/JSON.sh"

if git --help >/dev/null 2>&1 ; then
    PACKAGE_GIT_ORIGIN="$(git config --get remote.origin.url)"
    if [ $? != 0 ]; then
	echo "SKIPPED: can not get Git metadata in '`pwd`'" >&2
	exit 0
    fi
    PACKAGE_GIT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
    PACKAGE_GIT_TSTAMP="$(git log -n 1 --format='%ct')"
    PACKAGE_GIT_HASH_S="$(git log -n 1 --format='%h')"
    PACKAGE_GIT_HASH_L="$(git rev-parse --verify HEAD)"
    PACKAGE_GIT_STATUS="$(git status -s)"

    PACKAGE_BUILD_HOST_UNAME="`uname -a`"
    PACKAGE_BUILD_HOST_NAME="`uname -n`"
    PACKAGE_BUILD_HOST_OS="`uname -s -r -v`"
    PACKAGE_BUILD_TSTAMP="`date -u '+%s'`"

    [ ! -x "$JSONSH" ] && \
	echo "FAILED to use JSON.sh from '$JSONSH'" >&2 && exit 3

    for V in PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
	 PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS \
	 PACKAGE_BUILD_HOST_UNAME PACKAGE_BUILD_HOST_NAME \
	 PACKAGE_BUILD_HOST_OS PACKAGE_BUILD_TSTAMP \
    ; do
        eval echo $V=\\\"'$'$V\\\""\\;"
	VE="${V}_ESCAPED"
	eval $VE=\"$(eval echo -E \"\$\{$V\}\" | $JSONSH -Q)\"
        eval echo $VE=\\\"'$'$VE\\\""\\;"
    done
else
    echo "FAILED to execute 'git' program" >&2
    exit 2
fi

exit 0
