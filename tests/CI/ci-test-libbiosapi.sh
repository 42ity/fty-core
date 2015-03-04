#!/bin/sh
#
# Copyright (C) 2015 Eaton
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
# Author(s): Michal Vyskocil <MichalVyskocil@eaton.com>
#
# Description:
#   - tests build and run against libbiosapi.so
#
# Requirements:
#   - devel tools and source code
#


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

[ -z "$BUILDSUBDIR" -o ! -d "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
[ ! -x "$BUILDSUBDIR/config.status" ] && BUILDSUBDIR="$PWD"
if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "Cannot find $BUILDSUBDIR/config.status, did you run configure?"
    echo "Search path: $CHECKOUTDIR, $PWD"
    exit 1
fi
echo "CI-INFO: Using BUILDSUBDIR='$BUILDSUBDIR' to run the database tests"

LIBDIR=$(dirname $(find "${BUILDSUBDIR}" -name 'libbiosapi.so'))
if [ ! -d "${LIBDIR}" ]; then
    echo "Cannot find libbiosapi.so in ${BUILDSUBDIR}"
    exit 1
fi

# build it
make -j `/usr/bin/getconf _NPROCESSORS_ONLN` all -C "${BUILDSUBDIR}" || exit $?
# run malamute
if pgrep malamute; then
    echo "ERROR: malamute is running!!"
    exit 1
fi
malamute /etc/malamute/malamute.cfg &
echo $! > malamute.pid

echo "CC test-libbiosapi.c"
gcc -g -o test-libbiosapi -I ${CHECKOUTDIR}/include/ -lczmq -lzmq -lmlm -L ${LIBDIR} -lbiosapi ${CHECKOUTDIR}/tests/api/test-libbiosapi.c
gccret=$?

# to get malamute enough time to start on slow HW
sleep 2

if [ ${gccret} -eq 0 ]; then
    echo "RUN test-libbiosapi"
    LD_LIBRARY_PATH=${LIBDIR}/test-libbiosapi
    ret=$?
else
    ret=${gccret}
fi

kill -9 $(cat malamute.pid)
rm -f t malamute.pid
if [ $ret -eq 0 ]; then
    echo "ci-test-libbiosapi: OK"
else
    echo "ci-test-libbiosapi: FAIL"
fi
exit $ret
