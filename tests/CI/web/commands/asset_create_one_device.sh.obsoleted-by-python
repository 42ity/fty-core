#
# Copyright (c) 2015 Eaton
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
#! \file   asset_create_one_dc.sh
#  \brief  CI tests for asset create and delete DC
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
init_script
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

No=1
SEQUE=0
ent=""
loc=""
#for cube in datacenter room rack; do
echo
curlfail_push_expect_noerrors
for cube in datacenter; do
    echo "********* asset_create_one_device.sh **************************************************************"
    echo "********* ${No}. Create_${cube} ********************************************************************"
    echo "***************************************************************************************************"
    test_it "Create_${cube}"
    PARAM='{"name":"'${cube}'_0","type":"'${cube}'","sub_type":"","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
    api_auth_post_json "/asset" "${PARAM}" >&5
    print_result $?
    loc="$cube"_0
    SEQUE="$(expr $SEQUE + 1)"
    No="$(expr $No + 1)"
done

for ent in feed ups genset server storage switch vm; do
    echo "********* asset_create_one_device.sh **************************************************************"
    echo "********* ${No}. Create_${ent} ********************************************************************"
    echo "***************************************************************************************************"
    test_it "Create_${ent}"
    PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
    api_auth_post_json "/asset" "${PARAM}" >&5
    print_result $?
    loc="${ent}_0"
    SEQUE="$(expr $SEQUE + 1)"
    No="$(expr $No + 1)"
done

loc="datacenter_0"
wpos="left"
for ent in epdu pdu; do
    echo "********* asset_create_one_device.sh **************************************************************"
    echo "********* ${No}. Create_${ent} ********************************************************************"
    echo "***************************************************************************************************"
    test_it "Create_${ent}"
    PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","location_w_pos":"'${wpos}'","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
    api_auth_post_json "/asset" "${PARAM}" >&5
    print_result $?
    SEQUE="$(expr $SEQUE + 1)"
    No="$(expr $No + 1)"
    wpos="right"
done
curlfail_pop

# ERROR MESSAGES
curlfail_push_expect_400

echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document have wrong format or error in the syntax ************************"
echo "***************************************************************************************************"
test_it "Request_document_have_wrong_format_or_error_in_the_syntax"

ent=feed
PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":::"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document is empty ********************************************************"
echo "***************************************************************************************************"
test_it "Request_document_is_empty"
PARAM=''
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

#*#*#*#*#*#*# TODO : What does it mean keys?
if false;then
echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document have keys that are not implemented yet **************************"
echo "***************************************************************************************************"
test_it "Request_document_have_keys_that_are_not_implemented_yet"
curlfail_push_expect_4xx5xx
PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","nonexist":"item","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"
curlfail_pop
fi

#*#*#*#*#*#*# TODO : The response had another msg then is in RFC-11 - Request document has invalid syntax. key 'id' is forbidden to be used","code":48
echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document has key "id" ****************************************************"
echo "***************************************************************************************************"
test_it "Request_document_has_key_by_ID"
PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","id":"item","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document key 'type' has unsupported value ********************************"
echo "***************************************************************************************************"
test_it "Request_document_key_type_has_unsupported_value"
PARAM='{"name":"'${ent}'_0","type":"Dejvice","sub_type":"'${ent}'","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","id":"item","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. Request document doesn't contain all required data *******************************"
echo "***************************************************************************************************"
test_it "Request_document_doesn't_contain_all_required_data"
PARAM='{"name":"'${ent}'_0","type":"device","sub_type":"'${ent}'","location":"'${loc}'","business_critical":"yes","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

echo "********* asset_create_one_device.sh **************************************************************"
echo "********* ${No}. location_w_pos is not specified for epdu/pdu *************************************"
echo "***************************************************************************************************"
test_it "location_w_pos_is_not_specified_for_epdu/pdu"
PARAM='{"name":"epdu_1","type":"device","sub_type":"epdu","location":"'${loc}'","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST00'${SEQUE}'","address":"ASDF","serial_no":"ABCD00'${SEQUE}'","ip.1":"10.229.5.'${SEQUE}'"}}'
api_auth_post_json "/asset" "${PARAM}" >&5
print_result $?
SEQUE="$(expr $SEQUE + 1)"
No="$(expr $No + 1)"

curlfail_pop
### End of expected ERRORS

echo
echo "###################################################################################################"
echo "********* asset_create_one_device.sh *************** END ******************************************"
echo "###################################################################################################"
echo

