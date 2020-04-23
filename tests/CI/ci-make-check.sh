#!/bin/bash
#
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
#! \file   ci-make-check.sh
#  \brief  installs dependecies and compiles the project
#  \author Tomas Halman <TomasHalman@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e

if [ -s "${MAKELOG}" ] && [ -s "$BUILDSUBDIR/"Makefile ] && [ -s "$BUILDSUBDIR/"config.status ] ; then
    # This branch was already configured and compiled here, only refresh it now
    # Just in case, we still provide consistent configure flags
    echo "=========== auto-make (refresh) and install ================="
    ./autogen.sh --install-dir / --no-distclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux --enable-ci-tests" \
        ${AUTOGEN_ACTION_MAKE} install 2>&1 | tee -a ${MAKELOG}
else
    # Newly checked-out branch, rebuild
    echo "========= auto-configure, rebuild and install ==============="
    ./autogen.sh --install-dir / --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux --enable-ci-tests" \
        ${AUTOGEN_ACTION_INSTALL} 2>&1 | tee ${MAKELOG}
fi

echo "======================== make check ========================="
./autogen.sh --no-distclean ${AUTOGEN_ACTION_MAKE} check 2>&1 | tee -a ${MAKELOG}
