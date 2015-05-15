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
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: installs dependecies and compiles the project with "make
#            distcheck", which allows to verify that Makefiles are sane.
#            Takes a lot of time, so this is just an occasional CI task

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e

( which apt-get 2>/dev/null ) && apt-get update
#mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc

# TODO: optionally compare to previous commit and only run this test if there
# were added/removed/renamed files or changes to Makefile.am or configure.ac

if [ ! -s "Makefile" ] ; then
    # Newly checked-out branch, rebuild
    echo "========= auto-configure, rebuild and install ==============="
    ./autogen.sh --install-dir / --no-distclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG} 2>&1 | tee -a ${MAKELOG}
fi

echo "==================== make distcheck ========================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} distcheck 2>&1 | tee -a ${MAKELOG}
