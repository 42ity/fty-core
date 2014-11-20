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
# Description: installs dependecies and compiles the project

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project

set -e

apt-get update
mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc
cd $CHECKOUTDIR

# TODO: parallelization?
echo "======================== autoreconf ==============================="
autoreconf -vfi
echo "======================== configure ==============================="
./configure --prefix=$HOME
echo "======================== make ==============================="
make -j 4
echo "======================== make check ==============================="
make -j 4 check
echo "======================== make dist ==============================="
make -j 4 dist
echo "======================== make distcheck ==============================="
make -j 4 distcheck
