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
#
# Description: installs dependencies and compiles the project

CPPCHECK=$(which cppcheck)

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project


set -e
( which apt-get >/dev/null &&  apt-get update ) || true
( which mk-build-deps >/dev/null && mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc ) || true

cd $CHECKOUTDIR

CPUS=$(getconf _NPROCESSORS_ONLN)
echo "======================== autoreconf ========================="
autoreconf -vfi
echo "======================== configure =========================="
./configure --prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux
echo "======================== make ==============================="
make -j $CPUS
if [ -x "$CPPCHECK" ] ; then
    echo -e "*:src/msg/*_msg.c\nunusedFunction:src/api/*\n" >cppcheck.supp
    $CPPCHECK --enable=all --inconclusive --xml --xml-version=2 \
        --suppressions-list=cppcheck.supp \
        src 2>cppcheck.xml
    sed -i 's%\(<location file="\)%\1project/%' cppcheck.xml
    /bin/rm -f cppcheck.supp
fi
echo "======================== make check ========================="
make -j $CPUS check
echo "======================== make dist =========================="
make -j $CPUS dist
echo "======================== make distcheck ====================="
make -j $CPUS distcheck
