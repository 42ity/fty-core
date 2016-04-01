# Copyright (C) 2016 Eaton
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

# Value set up via generated script in ci-test-restapi preparation for the averages test
AVGSRV_ELEMENT_ID="36"
test_it "verify_AVG-SRV::exists"
AVGSRV_ELEMENT_ID="$(do_select "SELECT id_discovered_device FROM t_bios_discovered_device WHERE name='AVG-SRV'")"
[ $? -eq 0 -a -n "$AVGSRV_ELEMENT_ID" ] && [ "$AVGSRV_ELEMENT_ID" -ge 0 ]
print_result $? "See errors above, maybe the custom averages-test SQL files were not imported"

test_it "verify_AVG-SRV::expected=36"
[ "$AVGSRV_ELEMENT_ID" -eq 36 ]
print_result -$? "This used to be '36', is '$AVGSRV_ELEMENT_ID' now... MAYBE database is inconsistent for the test below"

echo "****************************************************************************************"
echo "********* 1. relative=7d step=24h type=arithmetic_mean *********************************"
echo "****************************************************************************************"
test_it "relative=7d"
OUTPUT="`api_get_json /metric/computed/average?type=arithmetic_mean\&step=24h\&source=temperature.thermal_zone0\&element_id=${AVGSRV_ELEMENT_ID}\&relative=7d`"
[ $? -eq 0 -a -n "$OUTPUT" ]
print_result $? "Did not get a JSON response successfully"

echo "DEBUG: Got the following JSON from server:"
echo "$OUTPUT"

# store the "data" array part of json output
test_it "relative=7d - get data"
JPATH='^"data",[0-9]+$'
DATA_OUTPUT="`echo "$OUTPUT" | jsonsh_cli -Nnx -x="$JPATH"`"
[ $? -eq 0 -a -n "$DATA_OUTPUT" ]
print_result $? "Could not excise the data array from server output JSON"

logmsg_debug "Got the following JSON data bits from server:" "$DATA_OUTPUT"

# "units": "C"
test_it "relative=7d - check \"units\""
JPATH='^"units"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"C"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $? "No 'units=C' match in output"

test_it "relative=7d - check \"source\""
# "source": "temperature.thermal_zone0"
JPATH='^"source"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"temperature.thermal_zone0"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $? "No 'source=temperature.thermal_zone0' match in output"

test_it "relative=7d - check \"step\""
# "step": "24h"
JPATH='^"step"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"24h"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $? "No 'step=24h' match in output"

test_it "relative=7d - check \"type\""
# "type": "arithmetic_mean"
JPATH='^"type"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep '"arithmetic_mean"'`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $? "No 'type=arithmetic_mean' match in output"

test_it "relative=7d - check \"element_id\""
# "element_id": ${AVGSRV_ELEMENT_ID}
# e.g.   "element_id": 36
JPATH='^"element_id"$'
JSON_PARSED="`echo "$OUTPUT" | jsonsh_cli -x="$JPATH" | grep -w "${AVGSRV_ELEMENT_ID}"`"
[ $? -eq 0 -a -n "$JSON_PARSED" ]
print_result $? "No 'element_id=${AVGSRV_ELEMENT_ID}' match in output"

test_it "relative=7d - number of \"data\" items"
JSON_PARSED="`echo "$DATA_OUTPUT" | wc -l`"
[ $? -eq 0 -a "$JSON_PARSED" -eq "7" ]
print_result $? "There are not exactly 7 entries in the data array of server output"

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
    print_result $? "No '$pattern' match in data array"
done



