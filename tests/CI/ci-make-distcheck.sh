#!/bin/bash
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
#! \file    ci-make-distcheck.sh
#  \brief   "make distcheck" verification
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Installs dependecies and compiles the project with "make
#           distcheck", which allows to verify that Makefiles are sane.
#           Takes a lot of time, so this is just an occasional CI task

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true

# We can make distcheck unconditionally, or try to see if it is needed
[ -z "$REQUIRE_DISTCHECK" ] && REQUIRE_DISTCHECK=no
#[ -z "$GIT_UPSTREAM" ] && GIT_UPSTREAM='http://stash.mbt.lab.etn.com/scm/bios/core.git'
[ -z "$GIT_UPSTREAM" ] && GIT_UPSTREAM='ssh://git@stash.mbt.lab.etn.com:7999/bios/core.git'

GOT_GIT=no
if ( which git >/dev/null 2>&1) && [ -d .git ] && git status > /dev/null; then
    GOT_GIT=yes
fi

isCheckRequired() {
    [ "$REQUIRE_DISTCHECK" = yes ] && \
        logmsg_info "REQUIRE_DISTCHECK=yes already set by caller" && \
        return 0

    if [ "$GOT_GIT" = yes ] ; then
        # Optionally compare to previous commit and only run this test if there
        # were added/removed/renamed files or changes to Makefile.am / configure.ac

        CHANGED_LOCAL="`git status -s | egrep -v '^\?\? '`"
        if [ $? = 0 ] && [ -n "$CHANGED_LOCAL" ]; then
            logmsg_warn "Uncommitted local changes detected, so requesting the distcheck"
            echo "$CHANGED_LOCAL"
            REQUIRE_DISTCHECK=yes
            return 0
        fi

        case "$GIT_UPSTREAM" in
            ssh://*)
                GIT_HOST="`echo "$GIT_UPSTREAM" | sed 's,^ssh://\([^/]*\)/.*$,\1,'`" && \
                { GIT_PORT="`echo "$GIT_HOST" | grep : | sed 's,^.*:\([0-9]*\)$,\1,'`" || GIT_PORT=22;
                  [ -n "$GIT_PORT" ] && [ "$GIT_PORT" -gt 0 ] 2>/dev/null || GIT_PORT=22; } && \
                GIT_HOST="`echo "$GIT_HOST" | sed 's,^.*@,,' | sed 's,:[0-9]*$,,'`" && \
                egrep -i "Host.*$GIT_HOST" ~/.ssh/config >/dev/null 2>&1
                if [ $? != 0 ] ; then
                    # Entry not found - add it
                    logmsg_info "Adding SSH-client trust to Git host $GIT_HOST:$GIT_PORT"
                    echo -e "Host $GIT_HOST\n\tPort $GIT_PORT\n\tStrictHostKeyChecking no\n\tIdentityFile ~/.ssh/id_dsa\n\tIdentityFile ~/.ssh/id_dsa-jenkins\n\tIdentityFile ~/.ssh/id_rsa\n\tIdentityFile ~/.ssh/id_rsa-jenkins\n\tPubkeyAuthentication yes\n" >> ~/.ssh/config
                fi
                ;;
        esac

        if git remote -v | egrep '^origin.*'"$GIT_UPSTREAM" ; then
            # TODO: We inherently assume this is upstream/master... but it may
            # be a no-problem anyway - we compare old comit in this branch.

            logmsg_info "We are in a replica of upstream; so consider it a fresh one and compare to our own older commit"
            [ -z "$OLD_COMMIT" ] && OLD_COMMIT='HEAD~1'
        else
            # Try to get the upstream to compare with
            if ! git remote -v | egrep '^upstream' > /dev/null ; then
                logmsg_info "Registering Git 'upstream' remote repository"
                git remote add upstream "$GIT_UPSTREAM"
            fi

            logmsg_info "Fetching the latest bits from Git 'upstream' remote repository"
            git fetch upstream
            if [ $? != 0 ]; then
                logmsg_warn "Communication with remote upstream '$GIT_UPSTREAM' failed, so requesting the distcheck"
                REQUIRE_DISTCHECK=yes
                return 0
            fi

            if [ -z "$OLD_COMMIT" ]; then
                OUT="`git diff upstream/master`"
                if [ $? = 0 ] && [ -n "$OUT" ] ; then
                    logmsg_info "We are on a branch not identical to a known upstream/master," \
                        "so compare to that"
                    OLD_COMMIT='upstream/master'
                else
                    logmsg_info "This is a replica of upstream/master, or we couldn't fetch it," \
                        "so compare to our own older commit"
                    OLD_COMMIT='HEAD~1'
                fi
            fi
        fi
        logmsg_info "Will compare current workspace contents to '$OLD_COMMIT'"

        CHANGED_DIRENTRIES="`git diff --summary ${OLD_COMMIT} | egrep '^ (create|delete|rename) '`"
        if [ $? = 0 -a -n "$CHANGED_DIRENTRIES" ] ; then
            # New files might not be reflected in a Makefile
            logmsg_info "Some directory entries were changed since the last Git commit, so requesting a distcheck"
            logmsg_echo "$CHANGED_DIRENTRIES"
            REQUIRE_DISTCHECK=yes
        else
            CHANGED_FILENAMES_SET=""
            CHANGED_FILENAMES="`git diff ${OLD_COMMIT} | egrep '^diff '`" && \
            { CHANGED_FILENAMES_SET="`echo "$CHANGED_FILENAMES" | egrep '/Makefile.am|/configure.ac|/autogen.sh|/tools/(builder.sh|git_details.sh)'`"
              if [ $? = 0 -a -n "$CHANGED_FILENAMES_SET" ] ; then
                logmsg_info "Some central project files were changed since the last Git commit, so requesting a distcheck"
                logmsg_echo "$CHANGED_FILENAMES_SET"
                REQUIRE_DISTCHECK=yes
              fi
              # TODO: It may be possible to detect renames here as well (detect
              # comparison of different filenames under ./a and ./b virtpaths)?
              CHANGED_FILENAMES_SET="`echo "$CHANGED_FILENAMES" | egrep '\.(h|hpp|c|cc|cpp|ecpp|sh|py|sym|[ctu]sv|txt|m4|in)$'`"
              if [ $? = 0 -a -n "$CHANGED_FILENAMES_SET" ] ; then
                logmsg_info "Some project source-code, scripts, make-processed templates or documents," \
                    "or test data files were changed since the last Git commit, so requesting a distcheck"
                logmsg_echo "$CHANGED_FILENAMES_SET"
                REQUIRE_DISTCHECK=yes
              fi ; }
        fi
    else
        logmsg_warn "Could not verify content of recent Git changes, so requesting a distcheck"
        REQUIRE_DISTCHECK=yes
    fi

    [ "$REQUIRE_DISTCHECK" = yes ] # Set a useful return code
}

if [ "$REQUIRE_DISTCHECK" = no ]; then
    isCheckRequired
    if [ "$GOT_GIT" = yes ] ; then
        echo ""
        echo "====================================="
        logmsg_info "Summary of the Git verification performed:"
        git remote -v
        git branch -a
        ./tools/git_details.sh 2>&1 | egrep 'PACKAGE_GIT_(ORIGIN|BRANCH|HASH_L)=' && \
        logmsg_echo "Compare OLD_COMMIT='$OLD_COMMIT'"
        [ -n "${OLD_COMMIT}" ] && \
            logmsg_info "Following files were changed between these commits:" &&\
            git diff "${OLD_COMMIT}" | egrep '^diff '
        echo "====================================="
        echo ""
    fi
fi

if [ "$REQUIRE_DISTCHECK" = no ]; then
    logmsg_warn "Detected that there were no such changes that require a make distcheck."
    logmsg_warn "  export REQUIRE_DISTCHECK=yes  to enforce this test. Quitting cleanly."
    exit 0
fi

if [ ! -s "${MAKELOG}" ] || [ ! -s "$BUILDSUBDIR/"Makefile ] || [ ! -s "$BUILDSUBDIR/"config.status ] ; then
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
