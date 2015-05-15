#!/bin/bash

# Copyright (C) 2014 Eaton
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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: installs dependecies and compiles the project with "make
#            distcheck", which allows to verify that Makefiles are sane.
#            Takes a lot of time, so this is just an occasional CI task

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
#set -e

( which apt-get >/dev/null 2>&1) && apt-get update || true
#mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc

# We can make distcheck unconditionally, or try to see if it is needed
[ -z "$REQUIRE_DISTCHECK" ] && REQUIRE_DISTCHECK=no
#[ -z "$GIT_UPSTREAM" ] && GIT_UPSTREAM='http://stash.mbt.lab.etn.com/scm/bios/core.git'
[ -z "$GIT_UPSTREAM" ] && GIT_UPSTREAM='ssh://git@stash.mbt.lab.etn.com:7999/bios/core.git'

if [ "$REQUIRE_DISTCHECK" = no ]; then
    if ( which git >/dev/null 2>&1) && [ -d .git ] && git status > /dev/null; then
        # Optionally compare to previous commit and only run this test if there
        # were added/removed/renamed files or changes to Makefile.am / configure.ac

        case "$GIT_UPSTREAM" in
            ssh://*)
                GIT_HOST="`echo "$GIT_UPSTREAM" | sed 's,^ssh://\([^/]*\)/.*$,\1,' | sed 's,^.*@,,' | sed 's,:[0-9]*$,,'`" &&
                { egrep -i 'Host.*$GIT_HOST' ~/.ssh/config >/dev/null 2>&1 || \
                    echo -e "Host $GIT_HOST\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config
                }
                ;;
        esac

        if ! git remote -v | egrep '^upstream' > /dev/null ; then
            yes yes | git remote add upstream "$GIT_UPSTREAM"
        fi
        yes yes | git fetch upstream || REQUIRE_DISTCHECK=yes

        if [ -z "$OLD_COMMIT" ]; then
            OUT="`git diff upstream/master`"
            if [ $? = 0 ] && [ -n "$OUT" ] ; then
                # We are on a branch not identical to a known upstream/master
                # So compare to it
                OLD_COMMIT='upstream/master'
            else
                # This is a replica of upstream/master, or we couldn't fetch it
                # Compare to our own older commit
                OLD_COMMIT='HEAD~1'
            fi
        fi

        CHANGED_DIRENTRIES="`git diff --summary ${OLD_COMMIT} | egrep '^ (create|delete|rename) '`"
        if [ $? = 0 -a -n "$CHANGED_DIRENTRIES" ] ; then
            # New files might not be reflected in a Makefile
            logmsg_info "Some directory entries were changed since the last Git commit, so requesting a distcheck"
            logmsg_echo "$CHANGED_DIRENTRIES"
            REQUIRE_DISTCHECK=yes
        else
            CHANGED_FILENAMES_SET=""
            CHANGED_FILENAMES="`git diff ${OLD_COMMIT} | egrep '^diff '`" && \
            CHANGED_FILENAMES_SET="`echo "$CHANGED_FILENAMES" | egrep '/Makefile.am|/configure.ac|/autogen.sh|/tools/(builder.sh|git_details.sh)'`"
            if [ $? = 0 -a -n "$CHANGED_FILENAMES_SET" ] ; then
                logmsg_info "Some central project files were changed since the last Git commit, so requesting a distcheck"
                logmsg_echo "$CHANGED_FILENAMES_SET"
                REQUIRE_DISTCHECK=yes
            fi
            # TODO: It may be possible to detect renames here as well (detect
            # comparison of different filenames under ./a and ./b virtpaths)?
        fi
    else
        logmsg_warn "Could not verify content of recent Git changes, so requesting a distcheck"
        REQUIRE_DISTCHECK=yes
    fi
fi

if [ "$REQUIRE_DISTCHECK" = no ]; then
    logmsg_warn "Detected that there were no such changes that require a make distcheck."
    logmsg_warn "  export REQUIRE_DISTCHECK=yes  to enforce this test. Quitting cleanly."
    ./tools/git_details.sh 2>&1 | egrep 'PACKAGE_GIT_(ORIGIN|BRANCH|HASH_L)=' && \
    logmsg_echo "Compare OLD_COMMIT='$OLD_COMMIT'"
    git remote -v
    git branch -a
    exit 0
fi

if [ ! -s "Makefile" ] ; then
    # Newly checked-out branch, rebuild
    echo "==================== auto-configure ========================="
    ./autogen.sh --install-dir / --no-distclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG} 2>&1 | tee -a ${MAKELOG}
else
    logmsg_info "Using the previously configured source-code workspace"
fi

echo "==================== make distcheck ========================="
./autogen.sh --no-distclean ${AUTOGEN_ACTION_MAKE} distcheck 2>&1 | tee -a ${MAKELOG}
echo $?
