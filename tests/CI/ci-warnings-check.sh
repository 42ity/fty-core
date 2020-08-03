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
#! \file   ci-warnings-check.sh
#  \brief  installs dependencies and compiles the project
#  \author Tomas Halman <TomasHalman@Eaton.com>

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

RESULT=0
set -o pipefail || true
set -e

### Note that configure and make are used explicitly to avoid a cleanup
### and full rebuild of the project if nothing had changed.
NEWBUILD=no
if [ ! -s "${MAKELOG}" ] || [ ! -s "$BUILDSUBDIR/"Makefile ] || [ ! -s "$BUILDSUBDIR/"config.status ] ; then
    # Newly checked-out branch, rebuild
    echo "============= auto-configure and rebuild all ================"
    /bin/rm -f ${MAKELOG}
    touch ${MAKELOG}
    ${CHECKOUTDIR}/autogen.sh --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_BUILD} 2>&1 | tee ${MAKELOG}
    NEWBUILD=yes
fi

# This branch was already configured and compiled here, only refresh it now
echo "======== auto-make (refresh all-buildproducts) =============="
${CHECKOUTDIR}/autogen.sh --no-distclean --optseqmake ${AUTOGEN_ACTION_MAKE} \
    all-buildproducts 2>&1 | tee -a ${MAKELOG}

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
else
    echo "WARNING: cppcheck binary not found" >&2
fi

sort_warnings() {
    # This routine parses its stdin and outputs counts of lower and higher
    # priority warnings in two columns
    LAST=$(expr ${#LOW_IMPORTANCE_WARNINGS[*]} - 1)
    LOW=0
    HIGH=0
    ( egrep ":[0-9]+:[0-9]+: warning: "; echo "" ) | \
      sort | uniq | \
    ( while read line ; do
        FOUND=0
        [ -z "$line" ] && continue
        for i in $(seq 0 $LAST) ; do
            if [[ "$line" =~ "${LOW_IMPORTANCE_WARNINGS[$i]}" ]] ; then
                LOW=$(expr $LOW + 1)
                echo "CI-INFO-CPPCHECK: Detected a low-priority warning:" \
                    "$line" >&2
                FOUND=1
                break
            fi
        done
        if [[ "$FOUND" == "0" ]] ; then
            HIGH=$(expr $HIGH + 1)
            echo "CI-INFO-CPPCHECK: Detected a warning not known" \
                "as a low-priority: $line" >&2
        fi
    done
    echo $LOW $HIGH )
}

echo "================ Are GitIgnores good? ======================="
RES_GITIGNORE=0
set +e
cat "$BUILDSUBDIR/.git_details" | grep PACKAGE_GIT_STATUS_ESCAPED | \
    grep -v 'PACKAGE_GIT_STATUS_ESCAPED=""'
if [ $? = 0 ]; then
    RES_GITIGNORE=1 && \
    logmsg_warn "Some build products (above) are not in a .gitignore" && \
    echo "" && sleep 1  # Sleep to not mix stderr and stdout in Jenkins
else
    echo "The .gitignores files are OK (build products and test logs are well ignored)"
fi
set -e

echo "==================== sort_warnings =========================="
ls -la ${MAKELOG}
[ -s "${MAKELOG}" ] || \
    logmsg_warn "make log file '$MAKELOG' is absent or empty!"
WARNINGS=$(sort_warnings < ${MAKELOG})
LOW=$(echo $WARNINGS | cut -d " " -f 1 ) 
HIGH=$(echo $WARNINGS | cut -d " " -f 2 ) 
#/bin/rm -f ${MAKELOG}

echo "================ Result ===================="
[[ "$RES_GITIGNORE" != 0 ]] && \
    echo "error: some build products are not gitignored, see details above"

if [[ "$HIGH" != "0" ]] ; then
    echo "error: $HIGH unknown warnings (not among LOW_IMPORTANCE_WARNINGS)"
    echo "warning: $LOW acceptable warnings"
    [[ "$NEWBUILD" = no ]] && \
        echo "NOTE: These may be old logged hits if you build in an uncleaned workspace"
    echo "============================================"
    exit 1
else
    if [[ "$LOW" != "0" ]] ; then
        echo "warning: $LOW acceptable warnings"
        [[ "$NEWBUILD" = no ]] && \
            echo "NOTE: These may be old logged hits if you build in an uncleaned workspace"
    else
        echo "OK, no compilation warnings detected"
    fi
    echo "============================================"
    exit $RESULT
fi >&2
