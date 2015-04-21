#!/bin/sh

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
# Author(s): Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: checks files on VM, whether they are sync with local checkout.
#              If they are not, files re copied to vitrual machine and compiled.

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -e

usage() {
    echo "Usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine name    virtual machine name"
    echo "    -p|--port PORT       virtual machine ssh port [22]"
    echo "    -u|--user USER       virtual machine ssh username"
    echo "    --dont-compile       don't compile make on target virtual machine"
    echo "    -h|--help            print this help"
}

#
# defaults
#
SUT_SSH_PORT=22
COMPILE=1
SUT_HOST="$BUILDMACHINE"
SUT_USER="root"
SUT_IS_REMOTE=yes

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|-sh|--machine)
            SUT_HOST="$2"
            shift 2
            ;;
        -p|-sp|--port)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        -u|-su|--user)
            SUT_SSH_USER="$2"
            shift 2
            ;;
        --dont-compile)
            COMPILE=0
            shift 1
            ;;
        -h|--help)
            usage
            exit 1
            ;;
        *)
            logmsg_error "Invalid switch $1"
            usage
            exit 1
            ;;
    esac
done

if [ ! "$SUT_HOST" ] ; then
    logmsg_error "Machine is not specified!"
    usage
    exit 1
fi

remote_cleanup() {
    echo "-- VM cleanup"
    sut_run "/bin/rm -rf  bin  core-build-deps_0.1_all.deb  extras  lib  libexec  project  share"
}

compare_revisions() {
    LOCALREVISION=$(cd $CHECKOUTDIR; git rev-parse HEAD)
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    REMOTEREVISION=$(sut_run "cd $BCHECKOUTDIR ; git rev-parse HEAD" )
    echo "L: $LOCALREVISION"
    echo "R: $REMOTEREVISION"
    if [ "$LOCALREVISION" = "$REMOTEREVISION" ] ; then
        return 0
    fi
    return 1
}

copy_project() {
    echo "-- copying files ($CHECKOUTDIR)"
    scp -r -P "$SUT_SSH_PORT" "$CHECKOUTDIR" "${SUT_USER}@$SUT_HOST:~/"
}

remote_make() {
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    BMAKELOG=$(basename $MAKELOG)
    if sut_run "cd $BCHECKOUTDIR && [ -s ${BMAKELOG} ]" ; then
        # This branch was already configured and compiled on that VM, refresh only
        echo "-- compiling to refresh"
        sut_run -t "/bin/bash --login -x -c 'set -o pipefail ; cd $BCHECKOUTDIR && { ./autogen.sh --nodistclean ${AUTOGEN_ACTION_MAKE} all-buildproducts install 2>&1 | tee -a ${BMAKELOG}; }; '"
        return $?
    else
        # Newly fetched branch - clean up, configure and make it fully
        echo "-- compiling to rebuild"
        sut_run -t "/bin/bash --login -x -c 'set -o pipefail ; cd $BCHECKOUTDIR && { eval ./autogen.sh --configure-flags \"--prefix=\$HOME\ --with-saslauthd-mux=/var/run/saslauthd/mux\" ${AUTOGEN_ACTION_BUILD} all-buildproducts install 2>&1 | tee ${BMAKELOG}; }; '"
        return $?
    fi
}

remote_log_cleanup() {
    echo "-- deleting old log files"
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    REMCMD='find $BCHECKOUTDIR -name '"'"'*.log'"'"' -o -name cppcheck.xml'
    FLIST=$(sut_run "$REMCMD -ls")
    if [ $? = 0 -a -n "$FLIST" ]; then
        echo "$FLIST"
        sut_run "$REMCMD -exec /bin/rm -f {} \; "
        return $?
    else
        echo "-- > no old log files detected on the remote system (${SUT_USER}@$SUT_HOST:$BCHECKOUTDIR/), nothing to delete"
        return 0
    fi
}


#
# taken from environment
#
echo "======================== BUILD PARAMETERS ==============================="
echo "FORK:     $FORK"
echo "BRANCH:   $BRANCH"
echo "PLATFORM: $BUILDMACHINE"
echo "======================== BUILD PARAMETERS ==============================="

if ! compare_revisions ; then
    echo "-------------- project on $SUT_HOST:$SUT_SSH_PORT needs synchronization ---------------"
    remote_cleanup
    copy_project
    [ "$COMPILE" = "1" ] && remote_make
    echo "----------------------------- sync end ----------------------------------"
fi
remote_log_cleanup
