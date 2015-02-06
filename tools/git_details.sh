#!/bin/sh

# Print the details of the current repository, if any
# Copyright (C) 2015 by Jim Klimov, <EvgenyKlimov@eaton.com>

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
    [ -x "`dirname $0`/JSON.sh" ] && \
	PACKAGE_GIT_STATUS="$(git status -s | "`dirname $0`/JSON.sh" -Q | sed 's,\\,\\\\,g')" || \
	PACKAGE_GIT_STATUS="$(git status -s)"

    for V in PACKAGE_GIT_ORIGIN PACKAGE_GIT_BRANCH PACKAGE_GIT_TSTAMP \
	 PACKAGE_GIT_HASH_S PACKAGE_GIT_HASH_L PACKAGE_GIT_STATUS ; do
        eval echo $V=\\\"'$'$V\\\""\\;"
    done
else
    echo "FAILED to execute 'git' program" >&2
    exit 2
fi

exit 0
