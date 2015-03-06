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
echo "CI-INFO: Using BUILDSUBDIR='$BUILDSUBDIR' to run the libbiosapi tests"

cd "${BUILDSUBDIR}" || exit $?

# We don't have too many files so don't care much if the parallel make fails
# due to resources
MAKE_FLAGS="V=0 -j `/usr/bin/getconf _NPROCESSORS_ONLN`"

# ensure that what we test is built
echo "CI-INFO: Ensure that we have a libbiosapi compiled..."
make $MAKE_FLAGS sdk || make sdk || exit $?

# Note: while we typically use the .so shared library, there can also be
# an .a static built, depending on configure flags. Both might be referenced
# by the .la descriptor. Here we care about where the actual libraries are.
# When rebuilding, note that both the .so and .la files should be removed.
LIBDIR="$(dirname $(find "${BUILDSUBDIR}" -name 'libbiosapi.so' || find "${BUILDSUBDIR}" -name 'libbiosapi.a') | head -1)"
if [ ! -d "${LIBDIR}" ]; then
    echo "CI-ERROR: Cannot find libbiosapi under ${BUILDSUBDIR}" >&2
    exit 1
fi

# run malamute
MALAMUTE_STARTED=no
if pgrep malamute; then
    echo "CI-INFO: malamute is already running!" >&2
    #[ -s malamute.pid ] || pidof malamute > malamute.pid
    #exit 1
else
    malamute /etc/malamute/malamute.cfg & && MALAMUTE_STARTED=yes
    echo $! > malamute.pid
fi

rm -f test-libbiosapi

echo "CI-INFO: Try to compile and run a libbiosapi API-client..."
echo "CC test-libbiosapi.c"
### Note that we should not explicitly pull the other libraries that are used by API...
#gcc -g -o test-libbiosapi -I ${CHECKOUTDIR}/include/ -lczmq -lzmq -lmlm -L ${LIBDIR} -lbiosapi ${CHECKOUTDIR}/tests/api/test-libbiosapi.c
gcc -g -o test-libbiosapi -I ${CHECKOUTDIR}/include/ -L ${LIBDIR} -lbiosapi ${CHECKOUTDIR}/tests/api/test-libbiosapi.c
gccret=$?

# to get malamute enough time to start on slow HW
sleep 2

if [ ${gccret} -eq 0 ]; then
    echo "RUN test-libbiosapi"
    LD_LIBRARY_PATH=${LIBDIR}/ ./test-libbiosapi
    ret=$?
else
    ret=${gccret}
fi

retut=0
echo "CI-INFO: Try to compile and run a libbiosapi Unit-tester..."
if make $MAKE_FLAGS test-libbiosapiut || make test-libbiosapiut ; then
    ./test-libbiosapiut || retut=$?
fi

if [ x"$MALAMUTE_STARTED" = xyes ]; then
    kill -9 $(cat malamute.pid)
    rm -f malamute.pid
fi

# Caller may set this to not delete the program file
[ x"$KEEP_TEST_LIBBOISAPI" = xyes ] || rm -f test-libbiosapi
if [ $ret -eq 0 ]; then
    echo "ci-test-libbiosapi: OK"
else
    echo "ci-test-libbiosapi: FAIL"
fi

if [ $retut -eq 0 ]; then
    echo "ci-test-libbiosapiut: OK"
else
    echo "ci-test-libbiosapiut: FAIL"
fi

[ $ret = 0 -a $retut = 0 ]
exit $?
