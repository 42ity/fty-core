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

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project

set -e

usage() {
    echo "usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine name    virtual machine name"
    echo "    -p|--port PORT       virtual machine ssh port [22]"
    echo "    -h|--help            print this help"
}

#
# defaults
#
PORT=22

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|--machine)
            VM="$2"
            shift 2
            ;;
        -m|--machine)
            PORT="$2"
            shift 2
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

remote_cleanup() {
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
    scp -r -P $PORT $CHECKOUTDIR "root@$VM:~/"
}

remote_make() {
    BCHECKOUTDIR=$(basename $CHECKOUTDIR)
    ssh root@$VM -p $PORT "cd $BCHECKOUTDIR autoreconf -vfi && ./configure --prefix=\$HOME && make -j 4 && make install"
}

if ! compare_revisions ; then
    echo "======================== make needed, data on $VM:$PORT not sync ==============================="
    remote_cleanup
    copy_project
    remote_make
    echo "======================================= make end ================================================"
fi
