
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
JPATH='"operating-system","processes"$'
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
JPATH='"operating-system","processes"$'
SYSINFO_PARSED="`echo "$OUTPUT" | ${JSONSH} -x="$JPATH"`"
[ $? -eq 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

echo
echo "###################################################################################################"
echo "********* sysinfo-info.sh ******************************** END ************************************"
echo "###################################################################################################"
echo

