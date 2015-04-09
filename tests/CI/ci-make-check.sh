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
# Description: installs dependecies and compiles the project

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e

apt-get update
mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc

if [ -s "make.log" ] ; then
    # This branch was already configured and compiled here, only refresh it now
    echo "================= auto-make (refresh) ======================="
    ./autogen.sh --no-distclean make 2>&1 | tee -a make.log
else
    # Newly checked-out branch, rebuild
    echo "==================== auto-configure ========================="
    ./autogen.sh --configure-flags "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" configure
    echo "=================== make and install ========================"
    ./autogen.sh make install | tee make.log
fi

echo "======================== make check ========================="
./autogen.sh make check | tee -a make.log
echo "==================== make distcheck ========================="
./autogen.sh make distcheck | tee -a make.log
