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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: checks files on VM, whether they are sync with local checkout.
#              If they are not, files re copied to vitrual machine and compiled.

if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

set -e

usage() {
    echo "Usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine name    virtual machine name"
    echo "    -p|--port PORT       virtual machine ssh port [22]"
    echo "    --dont-compile       don't compile make on target virtual machine"
    echo "    -h|--help            print this help"
}

#
# defaults
#
PORT=22
COMPILE=1
VM="$BUILDMACHINE"

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|--machine)
            VM="$2"
            shift 2
            ;;
        -p|--port)
            PORT="$2"
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
            echo "Invalid switch $1"
            usage
            exit 1
            ;;
    esac
done

if [ ! "$VM" ] ; then
    echo "Machine is not specified!"
    usage
    exit 1
fi

cd $CHECKOUTDIR || { echo "FATAL: Unusable CHECKOUTDIR='$CHECKOUTDIR'" >&2; exit 1; }

remote_cleanup() {
    echo "-- VM cleanup"
    ssh root@$VM -p $PORT "/bin/rm -rf  bin  core-build-deps_0.1_all.deb  extras  lib  project  share"
}

compare_revisions() {
    LOCALREVISION=$(cd $CHECKOUTDIR; git rev-parse HEAD)
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    REMOTEREVISION=$( ssh root@$VM -p $PORT "cd $BCHECKOUTDIR ; git rev-parse HEAD" )
    echo "L: $LOCALREVISION"
    echo "R: $REMOTEREVISION"
    if [ "$LOCALREVISION" = "$REMOTEREVISION" ] ; then
        return 0
    fi
    return 1
}

copy_project() {
    echo "-- copying files"
    scp -r -P $PORT $CHECKOUTDIR "root@$VM:~/"
}

remote_make() {
    echo "-- compiling"
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    ssh root@$VM -p $PORT "cd $BCHECKOUTDIR && autoreconf -vfi && ./configure --prefix=\$HOME --with-saslauthd-mux=/var/run/saslauthd/mux && make -j 4 && make install"
}

remote_log_cleanup() {
    echo "-- deleting old log files"
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    ssh root@$VM -p $PORT "find $BCHECKOUTDIR -name '*.log' -exec /bin/rm {} \; "
}


#
# taken from environment
#
echo "======================== BUILD PAREMETERS ==============================="
echo "FORK:     $FORK"
echo "BRANCH:   $BRANCH"
echo "PLATFORM: $BUILDMACHINE"
echo "======================== BUILD PAREMETERS ==============================="

if ! compare_revisions ; then
    echo "-------------- project on $VM:$PORT need synchronization ----------------"
    remote_cleanup
    copy_project
    [ "$COMPILE" = "1" ] && remote_make
    echo "----------------------------- sync end ----------------------------------"
fi
remote_log_cleanup
