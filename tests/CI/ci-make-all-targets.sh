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

set -e
( which apt-get >/dev/null &&  apt-get update ) || true
( which mk-build-deps >/dev/null && mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc ) || true

# NOTE: with this job we want everything wiped and rebuilt in the workspace
echo "=================== auto-configure =========================="
./autogen.sh --configure-flags "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" configure
echo "======================== make ==============================="
./autogen.sh make | tee make.log

echo "========================= cppcheck =========================="
if [ -x "$CPPCHECK" ] ; then
    echo '\
*:src/msg/*_msg.c
*:src/include/git_details_override.c
unusedFunction:src/api/*
' > cppcheck.supp
    $CPPCHECK --enable=all --inconclusive --xml --xml-version=2 \
        --suppressions-list=cppcheck.supp \
        src 2>cppcheck.xml
    sed -i 's%\(<location file="\)%\1project/%' cppcheck.xml
    /bin/rm -f cppcheck.supp
fi

echo "======================== make check ========================="
./autogen.sh make check | tee -a make.log
echo "======================== make dist =========================="
./autogen.sh make dist | tee -a make.log
echo "======================== make distcheck ====================="
./autogen.sh make distcheck | tee -a make.log
