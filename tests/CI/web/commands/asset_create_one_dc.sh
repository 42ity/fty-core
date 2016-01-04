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

echo
echo "###################################################################################################"
echo "********* asset_create_one_dc.sh ******************* START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 1. Asset_without_parameter **************************************************************"
echo "***************************************************************************************************"
test_it "Asset_without_parameter"
curlfail_push_expect_400
api_auth_post_json /asset '' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 2. Asset_with_empty_parameter ***********************************************************"
echo "***************************************************************************************************"
test_it "Asset_with_empty_parameter"
curlfail_push_expect_400
api_auth_post_json '/asset' "{}" >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 3. Create_DC_element ********************************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_element"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dc_name_test_0","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0003","address":"ASDF","serial_no":"ABCD0003","ip.1":"10.229.5.11"}}' >&5
RES=$?
PARSED_REPLY="$(echo "${OUT_CURL}" | $JSONSH -x id)"
ID_DC="$(echo "${PARSED_REPLY}" | cut -d '"' -f 4)"
print_result $RES
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 4. Create_DC_with_the_duplicate_asset_name **********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicate_asset_name"
curlfail_push_expect_409
api_auth_post_json '/asset' '{"name":"dc_name_test_0","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0004","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 5. Create_DC_with_the_duplicate_asset_tag ***********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicate_asset_tag"
api_auth_post_json '/asset' '{"name":"dc_name_test_1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0003","address":"ASDF"}}' >&5
print_result $?

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 6. Create_DC_with_the_duplicate_serial_no ***********************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicate_serial_no"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_6","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0006","address":"ASDF","serial_no":"ABCD0003"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 7. Create_DC_with_the_duplicate_ip1 *****************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_the_duplicate_ip1"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dc_name_test_7","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0007","address":"ASDF","serial_no":"ABCD0007","ip.1":"10.229.5.11"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 8. Delete_DC2_with_child_element ********************************************************"
echo "***************************************************************************************************"
test_it "Delete_DC2_with_child_element"
curlfail_push_expect_409
api_auth_delete_json /asset/10 >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 9. Delete_dc_name_test_0_without_child_element ******************************************"
echo "***************************************************************************************************"
test_it "Delete_dc_name_test_0_without_child_element"
curlfail_push_expect_noerrors
api_auth_delete_json /asset/$ID_DC >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 10. Create_DC_with_too_short_asset_tag **************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_too_short_asset_tag"
api_auth_post_json '/asset' '{"name":"dc_name_test_10","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A","address":"ASDF"}}' >&5
print_result $?

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 11. Create_DC_with_empty_asset_tag ******************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_empty_asset_tag"
#curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_11","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"","address":"ASDF"}}' >&5
print_result $?
#curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 12. Create_DC_with_too_long_asset_tag ***************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_too_long_asset_tag"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_2","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST56789012345678901234567890123456789012345678901","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 13. Create_DC_with_chinese_chars_name ***************************************************"
echo "***************************************************************************************************"
test_it "Create_DC_with_chinese_chars_name"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"中國文字的罰款","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0013","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 14. Create_DC_with_too_long_keytag ******************************************************"
echo "***************************************************************************************************"
#*#*#*#*#* asset_create_one_dc.sh - subtest 14 - TODO, should not be response 500 internal error?
test_it "Create_DC_with_too_long_keytag"
curlfail_push_expect_500
api_auth_post_json '/asset' '{"name":"A123456789B123456789C123456789D123456789E123456789F1","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0014","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 15. Item_type_is_empty ******************************************************************"
echo "***************************************************************************************************"
test_it "type_is_empty"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_15","type":"","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0015","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 16. Item_type_is_missing ****************************************************************"
echo "***************************************************************************************************"
test_it "item_type_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_16","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0016","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 17. Item_sub_type_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_sub_type_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_17","type":"datacenter","location":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0017","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 18. Item_location_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_location_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_18","type":"datacenter","sub_type":"","status":"active","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0018","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 19. Item_status_is_missing **************************************************************"
echo "***************************************************************************************************"
test_it "Item_status_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_19","type":"datacenter","sub_type":"","location":"","business_critical":"yes","priority":"P16","ex":{"asset_tag":"TEST0019","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 20. Item_business_critical_is_missing ***************************************************"
echo "***************************************************************************************************"
test_it "Item_business_critical_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_20","type":"datacenter","sub_type":"","location":"","status":"active","priority":"P16","ex":{"asset_tag":"TEST0020","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 21. Item_priority_is_missing ************************************************************"
echo "***************************************************************************************************"
test_it "Item_priority_is_missing"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_21","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","ex":{"asset_tag":"TEST0021","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 22. Item_asset_tag_is_missing ***********************************************************"
echo "***************************************************************************************************"
test_it "Item_asset_tag_is_missing"
#curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_22","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ex":{"address":"ASDF"}}' >&5
print_result $?
#curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 23. Item_status_is_empty ****************************************************************"
echo "***************************************************************************************************"
test_it "status_is_empty"
curlfail_push_expect_400
api_auth_post_json '/asset' '{"name":"dc_name_test_23","type":"datacenter","sub_type":"","location":"","status":"","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0023","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 24. Item_business_critical_is_empty *****************************************************"
echo "***************************************************************************************************"
test_it "business_critical_is_empty"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dc_name_test_24","type":"datacenter","sub_type":"","location":"","status":"nonactive","business_critical":"","priority":"P1","ext":{"asset_tag":"TEST0024","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo "********* asset_create_one_dc.sh ******************************************************************"
echo "********* 25. Unauthorized_create_operation   *****************************************************"
echo "***************************************************************************************************"
test_it "Unauthorized_create_operation"
curlfail_push_expect_401
api_post_json '/asset' '{"name":"dc_name_test_25","type":"datacenter","sub_type":"","location":"","status":"nonactive","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0025","address":"ASDF"}}' >&5
print_result $?
curlfail_pop

echo
echo "###################################################################################################"
echo "********* asset_create_one_dc.sh ******************* END ******************************************"
echo "###################################################################################################"
echo
