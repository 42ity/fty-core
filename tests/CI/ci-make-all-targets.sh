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
#! \file   ci-make-all-targets.sh
#  \brief  installs dependencies and compiles the project
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

CPPCHECK=$(which cppcheck)

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e

PATH="/usr/lib/ccache:/usr/lib64/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"

# NOTE: with this job we want everything wiped and rebuilt in the workspace
# "make all-buildproducts" builds all possible binaries and libs
# "make check" runs some of them to execute certain unit-tests, etc.
# "make dist" makes sure a source tarball can be made (?)
# "make distcheck" does all of the above in an out-of-tree builddir
echo "====== distclean, auto-configure and rebuild all ============"
./autogen.sh --install-dir / --configure-flags \
    "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux --enable-ci-tests" \
    ${AUTOGEN_ACTION_BUILD} all-buildproducts 2>&1 | tee ${MAKELOG}

if [ x"${1-}" = x"--build-only" ] ; then
    logmsg_info "Requested to only configure and build all products, this has succeeded"
    exit 0
fi

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
else
    echo "WARNING: cppcheck binary not found" >&2
fi

echo "======================== make check ========================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} check 2>&1 | tee -a ${MAKELOG}
echo "======================== make dist =========================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} dist 2>&1 | tee -a ${MAKELOG}
echo "======================== make distcheck ====================="
./autogen.sh ${AUTOGEN_ACTION_MAKE} distcheck 2>&1 | tee -a ${MAKELOG}
