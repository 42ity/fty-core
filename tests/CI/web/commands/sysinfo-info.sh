
#
# Copyright (C) 2015 Eaton
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

#! \file sysinfo-info.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief test of the sysinfo restapi request

echo
echo "###################################################################################################"
echo "********* sysinfo-info.sh ******************************* START ***********************************"
echo "###################################################################################################"
echo

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

echo "***************************************************************************************************"
echo "********* 1. sysinfo_get_auth=0_raw ***************************************************************"
echo "***************************************************************************************************"

test_it "unauthorized sysinfo"
OUTPUT="`api_get_json /admin/sysinfo`"
[ $? -eq 0 -a -n "$OUTPUT" ]
print_result $?

test_it "unauthorized sysinfo - installation-date"
JPATH='"operating-system","installation-date"$'
SYSINFO_PARSED="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH"`"
[ $? -eq 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "unauthorized sysinfo - location"
JPATH='"operating-system","location"$'
SYSINFO_PARSED="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH"`"
[ $? -eq 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "unauthorized sysinfo - processes"
JPATH='"processes",[0-9]+$'
SYSINFO_PARSED="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH"`"
[ $? -eq 0 -a -z "$SYSINFO_PARSED" ]
print_result $?

echo "***************************************************************************************************"
echo "********* 2. sysinfo_get_auth=2_raw ***************************************************************"
echo "***************************************************************************************************"

test_it "authorized sysinfo"
OUTPUT="`api_auth_get_json /admin/sysinfo`"
[ $? -eq 0 -a -n "$OUTPUT" ]
print_result $?

test_it "authorized sysinfo - processes"
JPATH='"processes",[0-9]+$'
SYSINFO_PARSED="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH"`"
[ $? -eq 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "authorized sysinfo - commitid-core-package"
RES=0
JPATH='^"packages",[0-9]+,"package-name"$'
PKG_NUM="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH" | awk -F'\t' '$2 ~ /\"core\"/ { print $1 }' | sed 's/^.*,\([0-9]*\),.*$/\1/'`"
PKG_COMMIT="N/A"
if [ $? -eq 0 -a -n "$PKG_NUM" ]; then
    JPATH='^"packages",'"${PKG_NUM}"',"commit"$'
    PKG_COMMIT="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH" | awk -F'\t' '{ print $2 }' | sed -e 's,^\"\(.*\)\"$,\1,' -e 's,\-.*$,,' | tr '[A-Z]' '[a-z]'`"
    [ $? -eq 0 -a -n "$PKG_COMMIT" ]
    RES=$?
else
    RES=1
fi
logmsg_info "Commit ID built into package 'core' is '$PKG_COMMIT' (inspection result $RES)"
print_result $RES

test_it "authorized sysinfo - commitid-core-restapi"
JPATH='^"restapi-metadata","source-repo","commit"$'
BLD_COMMIT="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH" | awk -F'\t' '{ print $2 }' | sed -e 's,^\"\(.*\)\"$,\1,' | tr '[A-Z]' '[a-z]'`"
if [ $? -eq 0 -a -n "$BLD_COMMIT" ]; then
    logmsg_info "Commit ID built into REST API binaries is '$BLD_COMMIT'"
    print_result 0

    if [ -n "$PKG_COMMIT" -a x"$PKG_COMMIT" != "xN/A" ]; then
        test_it "authorized sysinfo - commitid-core-COMPARE"

        [ "${#PKG_COMMIT}" = "${#BLD_COMMIT}" ] || \
        if [ "${#PKG_COMMIT}" -gt "${#BLD_COMMIT}" ]; then
            PKG_COMMIT="`echo "$PKG_COMMIT" | cut -c 1-"${#BLD_COMMIT}"`"
        else
            BLD_COMMIT="`echo "$BLD_COMMIT" | cut -c 1-"${#PKG_COMMIT}"`"
        fi

        logmsg_info "Comparing PKG_COMMIT='$PKG_COMMIT' and BLD_COMMIT='$BLD_COMMIT'"
        [ x"$PKG_COMMIT" = x"$BLD_COMMIT" ]
        print_result -$?
    fi
else
    logmsg_info "This is debug-build info, not required to succeed"
    print_result -1
fi

echo
echo "###################################################################################################"
echo "********* sysinfo-info.sh ******************************** END ************************************"
echo "###################################################################################################"
echo

