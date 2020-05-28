#!/bin/bash

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
#! \file    ci-check-vm.sh
#  \brief   Checks files on VM
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Checks files on VM, whether they are sync with local checkout.
#           If they are not, files re copied to virtual machine and compiled.

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -e
#
# defaults
#
SUT_SSH_PORT=22
COMPILE=1
SUT_HOST="$BUILDMACHINE"
SUT_USER="root"
SUT_IS_REMOTE=yes

usage() {
    echo "Usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine|--sut-host|-sh NAME    virtual machine host name [$SUT_HOST]"
    echo "    -p|--port|--sut-port-ssh|-sp PORT   virtual machine ssh port [$SUT_SSH_PORT]"
    echo "    -u|--user|--sut-user|-su USER       virtual machine ssh username [$SUT_USER]"
    echo "    --dont-compile       don't compile make on target virtual machine"
    echo "    -h|--help            print this help"
}

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|-sh|--machine|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        -p|-sp|--port|--sut-port|--sut-port-ssh)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        -u|-su|--user|--sut-user)
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

if [ -z "$SUT_HOST" ] ; then
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
    ( cd "`dirname "$CHECKOUTDIR"`" && tar cf - "`basename "$CHECKOUTDIR"`" | \
      ssh -p "$SUT_SSH_PORT" "${SUT_USER}@$SUT_HOST" 'cd ~/ && tar xf -' )
}

copy_keys() {
    if [ x"`id -un`" = xjenkins ]; then
        for KF in `cd ${HOME}/.ssh && ls -1 id_* | grep -v '.pub'` ; do
            [ -s "${HOME}/.ssh/$KF" -a -s "${HOME}/.ssh/$KF.pub" ] && \
            echo "-- copying Jenkins SSH keys: $KF $KF.pub" && \
            scp -P "$SUT_SSH_PORT" "${HOME}/.ssh/$KF.pub" "${SUT_USER}@$SUT_HOST:.ssh/${KF}-jenkins.pub" && \
            scp -P "$SUT_SSH_PORT" "${HOME}/.ssh/$KF" "${SUT_USER}@$SUT_HOST:.ssh/${KF}-jenkins" && \
            sut_run "chmod 600 .ssh/${KF}-jenkins ; chmod 644 .ssh/${KF}-jenkins.pub"
        done
    fi
}

remote_make() {
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    BMAKELOG=$(basename $MAKELOG)
    if sut_run "cd $BCHECKOUTDIR && [ -s ${BMAKELOG} ]" ; then
        # This branch was already configured and compiled on that VM, refresh only
        # Just in case, we still provide consistent configure flags
        echo "-- compiling to refresh"
        sut_run -t "/bin/bash --login -x -c 'set -o pipefail ; cd $BCHECKOUTDIR && { ./autogen.sh --install-dir / --nodistclean --configure-flags \"--prefix=\$HOME\ --with-saslauthd-mux=/var/run/saslauthd/mux\" ${AUTOGEN_ACTION_MAKE} all-buildproducts install 2>&1 | tee -a ${BMAKELOG}; }; '"
        return $?
    else
        # Newly fetched branch - clean up, configure and make it fully
        # Note that the "--prefix=$HOME" should expand on the remote system
        echo "-- compiling to rebuild"
        sut_run -t "/bin/bash --login -x -c 'set -o pipefail ; cd $BCHECKOUTDIR && { eval ./autogen.sh --install-dir / --configure-flags \"--prefix=\$HOME\ --with-saslauthd-mux=/var/run/saslauthd/mux\" ${AUTOGEN_ACTION_BUILD} all-buildproducts install 2>&1 | tee ${BMAKELOG}; }; '"
        return $?
    fi
}

remote_log_cleanup() {
    echo "-- deleting old log files"
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    REMCMD='find $BCHECKOUTDIR -name '"'"'*.log'"'"' -o -name '"'"'*.trs'"'"' -o -name '"'"'cppcheck.xml'"'"' -o -wholename '"'"'*/tests/junit/*.xml'"'"' -o -name '"'"'*.out*'"'"' -o -name '"'"'*.err*'"'"
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

copy_keys
if ! compare_revisions ; then
    echo "-------------- project on $SUT_HOST:$SUT_SSH_PORT needs synchronization ---------------"
    remote_cleanup
    copy_project
    [ "$COMPILE" = "1" ] && remote_make
    echo "----------------------------- sync end ----------------------------------"
fi
remote_log_cleanup
