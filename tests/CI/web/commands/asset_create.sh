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
#! \file   asset_create.sh
#  \brief  CI tests for asset create and delete calls
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>

# asset element function
api_auth_post_max_one_asset_error() {
        # 1 = POST content
        # 2 = Expected error message
        # 3 = Expected error code
        # 4 = /ID
	api_auth_post /asset$4 "$1" >/dev/null
        echo OUT_CURL = "${OUT_CURL}"
        echo "${OUT_CURL}"|egrep "$2" && \
        echo "${OUT_CURL}"|egrep "$3"
}

# delete element from DB
api_auth_delete_max_one_asset_error() {
        # 1 = ID
        # 2 = Expected error message
        # 3 = Expected error code
        api_auth_delete /asset/$1 >&5
        echo OUT_CURL = "${OUT_CURL}"
        echo "${OUT_CURL}"|egrep "$2" && \
        echo "${OUT_CURL}"|egrep "$3"
}

# Unautorized variants
# Unauthorized asset
api_post_max_one_asset_error() {
        # 1 = POST content
        # 2 = Expected error message
        # 3 = Expected error code
        api_post /asset "$1" >&5
        echo OUT_CURL = "${OUT_CURL}"
        echo "${OUT_CURL}"|egrep "$2" && \
        echo "${OUT_CURL}"|egrep "$3"
}

echo "********* 1. Asset_without_parameter **************************************************************"
echo "***************************************************************************************************"
test_it "Asset_without_parameter"
curlfail_push_expect_400
api_auth_post_json /asset '' >&5
print_result $?
curlfail_pop

echo "********* 2. Asset_with_empty_parameter ***********************************************************"
echo "***************************************************************************************************"
test_it "Asset_with_empty_parameter."
curlfail_push_expect_400
api_auth_post_json '/asset' "{}" >&5
print_result $?
curlfail_pop

echo "********* 3. Create_DC_element ********************************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_element."
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dc_name_test_0","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0003","address":"ASDF","serial_no":"ABCD0003","ip.1":"10.229.5.11"}}' >&5
RES=$?
PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x id`
ID_DC=`echo "${PARSED_REPLY}" | cut -d '"' -f 4`
print_result $RES
curlfail_pop

echo "********* 4. Create_DC_with_the_duplicite_asset_name **********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicite_asset_name"
curlfail_push_expect_4xx5xx
api_auth_post_json '/asset' '{"name":"dc_name_test_0","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0004","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 5. Create_DC_with_the_duplicite_asset_tag ***********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicite_asset_tag"
curlfail_push_expect_4xx5xx
api_auth_post_json '/asset' '{"name":"dc_name_test_1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0003","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 6. Create_DC_with_the_duplicite_serial_no ***********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicite_serial_no"
curlfail_push_expect_4xx5xx
api_auth_post_json '/asset' '{"name":"dc_name_test_1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0006","address":"ASDF","serial_no":"ABCD0003"}}' >&5
print_result $?
curlfail_pop

echo "********* 7. Create_DC_with_the_duplicite_ip1 ***********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicite_ip1"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dc_name_test_7","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0007","address":"ASDF","serial_no":"ABCD0007","ip.1":"10.229.5.11"}}' >&5
print_result $?
curlfail_pop

echo "********* 8. Create_FEED_element ******************************************************************"
echo "***************************************************************************************************"
test_it "Create_FEED_element"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"FEED1","type":"device","sub_type":"feed","location":"dc_name_test_0","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0008","address":"ASDF"}}' >&5
print_result $RES
curlfail_pop

echo "********* 9. Delete_DC2_with_child_element. *******************************************************"
echo "***************************************************************************************************"
test_it "Delete_DC2_with_child_element."
curlfail_push_expect_409
api_auth_delete /asset/10 > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop

echo "********* 10. Delete_dc_name_test_0_without_child_element. *****************************************"
echo "***************************************************************************************************"
test_it "Delete_dc_name_test_0_without_child_element."
curlfail_push_expect_noerrors
api_auth_delete /asset/$ID_DC > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop

echo "********* 11. Create_DC_with_too_short_asset_tag ***************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_too_short_asset_tag"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST1","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 12. Create_DC_with_too_long_asset_tag ***************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_too_long_asset_tag"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_2","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST5678901","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 13. Create_DC_with_chinese_chars_name ***************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_chinese_chars_name"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"中國文字的罰款","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0013","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 14. Create_DC_with_too_long_keytag ******************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_too_long_keytag"
curlfail_push_expect_4xx5xx
api_auth_post_json '/asset' '{"name":"A123456789B123456789C123456789D123456789E123456789F1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0014","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 15. Item_type_is_empty ******************************************************************"
echo "***************************************************************************************************"
test_it "type_is_empty"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_15","type":"","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0015","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 16. Item_type_is_missing ****************************************************************"
echo "***************************************************************************************************"
test_it "item_type_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_16","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0016","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 17. Item_sub_type_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_sub_type_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_17","type":"datacenter","location":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0017","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 18. Item_location_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_location_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_18","type":"datacenter","sub_type":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0018","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 19. Item_status_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_status_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_18","type":"datacenter","sub_type":"","location":"","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0019","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 20. Item_business_critical_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_business_critical_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_20","type":"datacenter","sub_type":"","location":"","status":"active","priority":"P16","ex":{"asset_tag":"TEST0020","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 21. Item_priority_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_priority_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_21","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","ex":{"asset_tag":"TEST0021","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* 22. Item_asset_tag_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_asset_tag_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_22","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ex":{"asset_tag":"TEST0022","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

if false;then
echo "***************************************************************************************************"
test_it "ePDU_without_location_w_pos_parameter"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"ePDU1","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B125","address":"ASDF"}}' "Parameter.*location_w_pos.*for epdu/pdu.*is required." "46"
api_auth_post_json '/asset' '{"name":"ePDU1","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B125","address":"ASDF"}}' >&5
print_result $?

echo "***************************************************************************************************"
test_it "ePDU_with_the_duplicite_asset_tag"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"ePDU2","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"A123B124","address":"ASDF"}}' "Parameter 'insertion was unsuccessful' is required." "46"
api_auth_post_json '/asset' '{"name":"ePDU2","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"A123B124","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "ePDU_with_too_short_asset_tag"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"ePDU3","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"5char","address":"ASDF"}}' "Parameter 'Parameter 'asset_tag' has bad value. Received '<to short>'. Expected '<unique string from 6 to 10 characters>'' is required." "46"
api_auth_post_json '/asset' '{"name":"ePDU3","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"5char","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "ePDU_with_too_long_asset_tag"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"ePDU3","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"11_characte","address":"ASDF"}}' "Parameter 'Parameter 'asset_tag' has bad value. Received '<to long>'. Expected '<unique string from 6 to 10 characters>'' is required." "46"
api_auth_post_json '/asset' '{"name":"ePDU3","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"left","asset_tag":"11_characte","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "ePDU_with_too_long_keytag"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"ePDU2","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B126","address":"ASDF","it_is_41_chars_long_keytag_forbiden_lengt":"I MUSTNOT BE IN DB."}}' "Parameter 'insertion was unsuccessful' is required." "46"
api_auth_post_json '/asset' '{"name":"ePDU2","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B126","address":"ASDF","it_is_41_chars_long_keytag_forbiden_lengt":"I MUSTNOT BE IN DB."}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "ePDU_with_chinese_chars_name"
curlfail_push_expect_noerrors
#api_auth_post_max_one_asset_error '{"name":"中國文字的罰款","type":"device","sub_type":"epdu","location":"DC1","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B128","address":"ASDF","it_is_normal_keytag":"I MAY BE IN DB."}}' "id.*4[0-9]"
api_auth_post_json '/asset' '{"name":"中國文字的罰款","type":"device","sub_type":"epdu","location":"DC1","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B128","address":"ASDF","it_is_normal_keytag":"I MAY BE IN DB."}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "not_accepted_character_%_in_name"
curlfail_push_expect_noerrors
#api_auth_post_max_one_asset_error '{"na%me":"werty","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B138","address":"ASDF","it_is_normal_keytag":"I MAY BE IN DB."}}' "id.*4[0-9]"
api_auth_post_json '/asset' '{"na%me":"werty","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"location_w_pos":"right","asset_tag":"A123B138","address":"ASDF","it_is_normal_keytag":"I MAY BE IN DB."}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "type_is_empty"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"dc_name_test1","type":"","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B223","address":"ASDF"}}' "Parameter 'Parameter 'type' has bad value. Received ''. Expected 'datacenter, device, group, rack, room, row'' is required." "46"
api_auth_post_json '/asset' '{"name":"dc_name_test1","type":"","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B223","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "item_type_is_missing"
curlfail_push_expect_400
#api_auth_post_max_one_asset_error '{"name":"dc_name_test20","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123F220","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'type'. Expected all of those name, type, sub_type, location, status, business_critical, priority, asset_tag" "47"
api_auth_post_json '/asset' '{"name":"dc_name_test20","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123F220","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

test_it "item_sub_type_is_missing"
curlfail_push_expect_400
#e_asset_error '{"name":"dc_name_test21","type":"datacenter","location":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"A123F221","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'sub_type'. Expected all of those name, type, sub_type, location, status, business_critical, priority, asset_tag" "47"
api_auth_post_json '/asset' '{"name":"dc_name_test21","type":"datacenter","location":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"A123F221","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

test_it "item_location_is_missing"
curlfail_push_expect_400
api_auth_post_max_one_asset_error '{"name":"dc_name_test22","type":"datacenter","sub_type":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"A123F222","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'location'. Expected all of those name, type, sub_type, location, status, business_critical, priority, asset_tag" "47"
api_auth_post_json '/asset' '{"name":"dc_name_test22","type":"datacenter","sub_type":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"A123F222","address":"ASDF"}}' >&5
print_result $?
curlfail_pop


api_auth_post_max_one_asset_error '{"name":"dc_name_test23","type":"datacenter","sub_type":"","location":"","business_critical":"yes","priority":"P16","ex":{"asset_tag":"A123F223","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'status'. Expected all of those name, type, suult $?
curlfail_pop
_type, location, status, business_critical, priority, asset_tag" "47"
RES=$(($RES+$?))
api_auth_post_max_one_asset_error '{"name":"dc_name_test23","type":"datacenter","sub_type":"","status":"active","location":"","priority":"P16","ex":{"asset_tag":"A123F223","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'business_critical'. Expected all of those name, type, sub_type, location, status, business_critical, priority, asset_tag" "47"
RES=$(($RES+$?))
api_auth_post_max_one_asset_error '{"name":"dc_name_test23","type":"datacenter","sub_type":"","status":"active","location":"","business_critical":"yes","ex":{"asset_tag":"A123F223","address":"ASDF"}}' "Parameter '<mandatory column name>' has bad value. Received 'priority'. Expected all of those name, type, sub_type, location, status, business_critical, priority, asset_tag" "47"
RES=$(($RES+$?))
print_result $RES
curlfail_pop


maximum_number_racks
echo "***************************************************************************************************"
test_it "business_critical_priority_is_empty"
curlfail_push_expect_noerrors
api_auth_post_max_one_asset_error '{"name":"dc_name_test3","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"","priority":"P1","ext":{"asset_tag":"A123B220","address":"ASDF"}}' "id.*4[0-9]" && \
api_auth_post_max_one_asset_error '{"name":"dc_name_test4","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"","ext":{"asset_tag":"A123B293","address":"ASDF"}}' "id.*4[0-9]"
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "priority_is_P16_16_p16"
curlfail_push_expect_noerrors
api_auth_post_max_one_asset_error '{"name":"dc_name_test5","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P16","ext":{"asset_tag":"A123E220","address":"ASDF"}}' "id.*4[0-9]" && \
api_auth_post_max_one_asset_error '{"name":"dc_name_test6","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"16","ext":{"asset_tag":"A123F220","address":"ASDF"}}' "id.*4[0-9]" && \
api_auth_post_max_one_asset_error '{"name":"dc_name_test7","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"p16","ext":{"asset_tag":"A123F221","address":"ASDF"}}' "id.*4[0-9]" && \
print_result $?
curlfail_pop

test_it "priority_is_0_p0"
#curlfail_push_expect_noerrors
curlfail_push_expect_400
api_auth_post_max_one_asset_error '{"name":"dc_name_test5","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"0","ext":{"asset_tag":"A123E220","address":"ASDF"}}' "Parameter 'insertion was unsuccessful' is required." "46"
RES=$?
api_auth_post_max_one_asset_error '{"name":"dc_name_test5","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"p0","ext":{"asset_tag":"A123E220","address":"ASDF"}}' "Parameter 'insertion was unsuccessful' is required." "46"
RES=$(($RES+$?))
echo RES = $RES
print_result $RES
curlfail_pop
exit 0
echo "***************************************************************************************************"
test_it "Create_already_asseted_element"
curlfail_push_expect_400
api_auth_post_max_one_asset_error '{"name":"dc_name_test","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B123","address":"ASDF"}}' "Parameter 'insertion was unsuccessful' is required." "46"
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "Delete_element_with_child_element."
curlfail_push_expect_4xx5xx
api_auth_delete_max_one_asset_error "$ID_DC" "Element.*cannot be processed because of conflict. Asset has elements inside, DELETE them first!" "50"
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "Delete_element_with_not_exist_id"
curlfail_push_expect_404
api_auth_delete_max_one_asset_error "1000" "Element '1000' not found." "44"
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "Delete_element_successfully"
curlfail_push_expect_noerrors
api_auth_delete_max_one_asset_error "$ID_FEED" "{}" ""
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "Unauthorized_create_DC_element."
curlfail_push_expect_401
api_post_max_one_asset_error '{"name":"dc_name_unauthorizes","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"UNA1234","address":"ASDF"}}' "Not authorized"
print_result $?
fi

