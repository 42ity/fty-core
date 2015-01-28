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
# Description: installs dependencies and compiles the project

CPPCHECK=$(which cppcheck)

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project

set -e

apt-get update
mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc
cd $CHECKOUTDIR

CPUS=$(getconf _NPROCESSORS_ONLN)
echo "======================== autoreconf ========================="
autoreconf -vfi
echo "======================== configure =========================="
./configure --prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux
echo "======================== make ==============================="
make -j $CPUS
if [ -x "$CPPCHECK" ] ; then
    $CPPCHECK -j $CPUS --enable=all --inconclusive --xml --xml-version=2 src 2> cppcheck.xml
fi
echo "======================== make check ========================="
make -j $CPUS check
echo "======================== make dist =========================="
make -j $CPUS dist
echo "======================== make distcheck ====================="
make -j $CPUS distcheck
