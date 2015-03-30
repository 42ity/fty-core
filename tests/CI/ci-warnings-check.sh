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

# list of warnings we know
# if any of them appears, build is marked as unstable
# if unknown warning appears, build fails
LOW_IMPORTANCE_WARNINGS=(
  "[-Wunused-variable]"
  '[-Wunused-parameter]'
  '[-Wno-unused-but-set-variable]'
)

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -e

echo "======================== update ============================="
apt-get update >/dev/null 2>&1
mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc >/dev/null 2>&1

echo "====================== autoreconf ==========================="
autoreconf -vfi >/dev/null 2>&1
echo "====================== configure ============================"
./configure --prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux >/dev/null 2>&1
echo "========================= make =============================="
make 2>&1 | tee make.log
echo "======================= cppcheck ============================"
CPPCHECK=$(which cppcheck || true)
if [ -x "$CPPCHECK" ] ; then
    $CPPCHECK --enable=all --inconclusive --xml --xml-version=2 \
              --suppress=*:src/include/*_msg.c \
              src 2>cppcheck.xml
    sed -i 's%\(<location file="\)%\1project/%' cppcheck.xml
fi

sort_warnings() {
    LAST=$(expr ${#LOW_IMPORTANCE_WARNINGS[*]} - 1)
    LOW=0
    HIGH=0
    grep -E ":[0-9]+:[0-9]+: warning: " < $CHECKOUTDIR/make.log | ( while read line ; do
        FOUND=0
        for i in $(seq 0 $LAST) ; do
            if [[ "$line" =~ "${LOW_IMPORTANCE_WARNINGS[$i]}" ]] ; then
                LOW=$(expr $LOW + 1)
                FOUND=1
                break
            fi
        done
        if [[ "$FOUND" == "0" ]] ; then
            HIGH=$(expr $HIGH + 1)
            echo "unknown warning: $line" >&2
        fi
    done
    echo $LOW $HIGH )
}

WARNINGS=$(sort_warnings)
LOW=$(echo $WARNINGS | cut -d " " -f 1 ) 
HIGH=$(echo $WARNINGS | cut -d " " -f 2 ) 
/bin/rm make.log

if [[ "$HIGH" != "0" ]] ; then
    echo "================ Result ===================="
    echo "error: $HIGH unknown warnings"
    echo "warning: $LOW acceptable warnings"
    echo "============================================"
    exit 1
else
    echo "================ Result ===================="
    if [[ "$LOW" != "0" ]] ; then
        echo "warning: $LOW acceptable warnings"
    else
        echo "OK"
    fi
    echo "============================================"
    exit 0
fi
