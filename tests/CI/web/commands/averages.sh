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

#! \file averages.sh
#  \author Karol Hrdina <karolhrdina@eaton.com>
#  \brief test of the computed/averages restapi request

echo
echo "###############################################################################################"
echo "********* averages.sh ******************************* START ***********************************"
echo "###############################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites:     **********************************************************************"
echo "*********     accept_license **********************************************************************"
echo "***************************************************************************************************"

accept_license
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

echo "****************************************************************************************"
echo "********* 1. relative=7d step=24h type=arithmetic_mean *********************************"
echo "****************************************************************************************"
test_it "relative=7d"
OUTPUT="`api_get_json /metric/computed/average?type=arithmetic_mean\&step=24h\&source=temperature.thermal_zone0\&element_id=36\&relative=7d`"
[ $? -eq 0 -a -n "$OUTPUT" ]
print_result $?

# store the "data" array part of json output
JPATH='^"data",[0-9]+$'
DATA_OUTPUT="`echo "$OUTPUT" | jsonsh_cli -Nnx -x="$JPATH"`"
[ $? -eq 0 -a -n "$DATA_OUTPUT" ]
echo "--- POZOR !!!! ---"
echo "$DATA_OUTPUT"

# "units": "C"
test_it "relative=7d - check \"units\""
JPATH='^"units"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"C"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $?

test_it "relative=7d - check \"source\""
# "source": "temperature.thermal_zone0"
JPATH='^"source"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"temperature.thermal_zone0"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $?

test_it "relative=7d - check \"step\""
# "step": "24h"
JPATH='^"step"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"24h"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $?

test_it "relative=7d - check \"type\""
# "type": "arithmetic_mean"
JPATH='^"type"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"arithmetic_mean"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $?

test_it "relative=7d - check \"element_id\""
# "element_id": 36
JPATH='^"element_id"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '36'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $?

test_it "relative=7d - number of \"data\" items"
JSON_PARSED="`echo "$DATA_OUTPUT" | wc -l`"
[ $? -eq 0 -a "$JSON_PARSED" == "7" ]
print_result $?

# $CI_TEST_AVERAGES_DATA contains space delimited '{"value":<value>,"timestamp":<timestamp>}' tokens on each line
# each line represents expected data for one test case
PATTERN_LINE="`echo "$CI_TEST_AVERAGES_DATA" | sed -n '1p'`"
i=0
for pattern in $PATTERN_LINE
do
    i=$((i+1))
    test_it "relative=7d - value/timestamp No.$i"
    JSON_PARSED="`echo "$DATA_OUTPUT" | grep "$pattern"`"
    [ $? -eq 0 -a -n "$JSON_PARSED" ]
    print_result $?
done



