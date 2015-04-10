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
  '[-Wunused-function]'
  '[-Wno-unused-but-set-variable]'
  'deprecated '
)

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

set -o pipefail || true
set -e

echo "======================== update ============================="
apt-get update >/dev/null 2>&1
mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc >/dev/null 2>&1

### Note that configure and make are used explicitly to avoid a cleanup
### and full rebuild of the project if nothing had changed.
if [ -s "${MAKELOG}" ] ; then
    # This branch was already configured and compiled here, only refresh it now
    echo "================= auto-make (refresh) ======================="
    ./autogen.sh --no-distclean ${AUTOGEN_ACTION_MAKE} 2>&1 | tee -a ${MAKELOG}
else
    # Newly checked-out branch, rebuild
    echo "=============== auto-configure and rebuild =================="
    /bin/rm -f ${MAKELOG}
    touch ${MAKELOG}
    ./autogen.sh --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_BUILD} 2>&1 | tee ${MAKELOG}
fi

echo "======================= cppcheck ============================"
CPPCHECK=$(which cppcheck || true)
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
            "but we consider it not fatal"; }
    sed -i 's%\(<location file="\)%\1project/%' cppcheck.xml
    ls -la cppcheck.xml
    /bin/rm -f cppcheck.supp
fi

sort_warnings() {
    # This routine parses its stdin and outputs counts of lower and higher
    # priority warnings in two columns
    LAST=$(expr ${#LOW_IMPORTANCE_WARNINGS[*]} - 1)
    LOW=0
    HIGH=0
    ( egrep ":[0-9]+:[0-9]+: warning: "; echo "" ) | ( while read line ; do
        FOUND=0
        [ -z "$line" ] && continue
        for i in $(seq 0 $LAST) ; do
            if [[ "$line" =~ "${LOW_IMPORTANCE_WARNINGS[$i]}" ]] ; then
                LOW=$(expr $LOW + 1)
                FOUND=1
                break
            fi
        done
        if [[ "$FOUND" == "0" ]] ; then
            HIGH=$(expr $HIGH + 1)
            logmsg_warn "Detected a warning not known" \
                "as a low-priority: $line" >&2
        fi
    done
    echo $LOW $HIGH )
}

echo "==================== sort_warnings =========================="
ls -la ${MAKELOG}
[ -s "${MAKELOG}" ] || \
    logmsg_warn "make log file '$MAKELOG' is absent or empty!"
WARNINGS=$(sort_warnings < ${MAKELOG})
LOW=$(echo $WARNINGS | cut -d " " -f 1 ) 
HIGH=$(echo $WARNINGS | cut -d " " -f 2 ) 
#/bin/rm -f ${MAKELOG}

if [[ "$HIGH" != "0" ]] ; then
    echo "================ Result ===================="
    echo "error: $HIGH unknown warnings (not among LOW_IMPORTANCE_WARNINGS)"
    echo "warning: $LOW acceptable warnings"
    echo "============================================"
    exit 1
else
    echo "================ Result ===================="
    if [[ "$LOW" != "0" ]] ; then
        echo "warning: $LOW acceptable warnings"
    else
        echo "OK, no warnings detected"
    fi
    echo "============================================"
    exit 0
fi >&2
