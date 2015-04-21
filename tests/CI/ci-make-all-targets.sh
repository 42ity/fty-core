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

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e
( which apt-get >/dev/null &&  apt-get update ) || true
#( which mk-build-deps >/dev/null && mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc ) || true

# NOTE: with this job we want everything wiped and rebuilt in the workspace
echo "============= auto-configure and rebuild ===================="
./autogen.sh --configure-flags \
    "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
    ${AUTOGEN_ACTION_BUILD} all-buildproducts 2>&1 | tee ${MAKELOG}

echo "========================= cppcheck =========================="
CPPCHECK_RES=0
if [ -x "$CPPCHECK" ] ; then
    echo \
'*:src/msg/*_msg.c
*:src/include/git_details_override.c
unusedFunction:src/api/*
' > cppcheck.supp
    $CPPCHECK --enable=all --inconclusive --xml --xml-version=2 \
            --suppressions-list=cppcheck.supp \
            src 2>cppcheck.xml || { CPPCHECK_RES=$?; \
        logmsg_warn "cppcheck reported failure ($CPPCHECK_RES)" \
            "but we consider it not fatal" ; }
    sed -i 's%\(<location file="\)%\1project/%' cppcheck.xml
    /bin/rm -f cppcheck.supp
fi

echo "======================== make check ========================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} check 2>&1 | tee -a ${MAKELOG}
echo "======================== make dist =========================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} dist 2>&1 | tee -a ${MAKELOG}
echo "======================== make distcheck ====================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} distcheck 2>&1 | tee -a ${MAKELOG}
