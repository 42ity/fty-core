
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
#! \file asset_one_datacenter.sh
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* asset_one_datacenter.sh ****************** START ****************************************"
echo "###################################################################################################"
echo

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 1. read_DC_with_id_of_another_element ***************************************************"
echo "***************************************************************************************************"
test_it "read_DC_with_id_of_another_element"
curlfail_push_expect_400
api_get_json /asset/datacenter/2 >&5
print_result $?
curlfail_pop

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 2. request_to existing_DC ***************************************************************"
echo "***************************************************************************************************"
PARSED_REPLY=$(echo $(api_get_json /asset/datacenters) | $JSONSH -x id)
ID_1=$(echo "${PARSED_REPLY}" | cut -d '"' -f 6)
ID_DC_2=$(echo $ID_1 | cut -d ' ' -f 2)
test_it "request_to existing_DC"
api_get_json /asset/datacenter/${ID_DC_2} >&5
print_result $?


echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 3. request_to_not_existing_id ***********************************************************"
echo "***************************************************************************************************"
test_it "request_to_not_existing_id"
curlfail_push_expect_404
api_get_json /asset/datacenter/100 >&5
print_result $?
curlfail_pop

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 4. request_to_id_0 **********************************************************************"
echo "***************************************************************************************************"
test_it "request_to_id_0"
curlfail_push_expect_400
api_get_json /asset/datacenter/0 >&5
print_result $?
curlfail_pop

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 5. request_to_too_long_id ***************************************************************"
echo "***************************************************************************************************"
test_it "request_to_too_long_id"
curlfail_push_expect_400
api_get_json /asset/datacenter/12345678901234567890 >&5
print_result $?
curlfail_pop

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 6. request_to_not_valid_id **************************************************************"
echo "***************************************************************************************************"
test_it "request_to_not_valid_id"
curlfail_push_expect_404
api_get_json /asset/datacenter/123456x78901234567890 >&5
print_result $?
curlfail_pop

echo "********* asset_one_datacenter.sh *****************************************************************"
echo "********* 7. request_to_missing_id ****************************************************************"
echo "***************************************************************************************************"
test_it "request_to_missing_id"
curlfail_push_expect_400
api_get_json /asset/datacenter/ >&5
print_result $?
curlfail_pop

echo
echo "###################################################################################################"
echo "********* asset_one_datacenter.sh ****************** END ******************************************"
echo "###################################################################################################"
echo
