
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


#! \file asset_one_group.sh
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* asset_one_group.sh *********************** START ****************************************"
echo "###################################################################################################"
echo

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 1. group_with_id_0 **********************************************************************"
echo "***************************************************************************************************"
test_it "group_with_id_0"
curlfail_push_expect_400
api_get_json /asset/group/0 >&5
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 2. group_with_non_exist_id **************************************************************"
echo "***************************************************************************************************"
test_it "group_with_non_exist_id"
curlfail_push_expect_404
api_get_json /asset/group/100 >&5
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 3. group_with_negative_id ***************************************************************"
echo "***************************************************************************************************"
test_it "group_with_negative_id"
curlfail_push_expect_400
api_get_json /asset/group/-1 >&5
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 4. group_without_id *********************************************************************"
echo "***************************************************************************************************"
test_it "group_without_id"
curlfail_push_expect_400
api_get_json /asset/group/ >&5 
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 5. group_with_wrong_id ******************************************************************"
echo "***************************************************************************************************"
test_it "group_with_wrong_id"
curlfail_push_expect_400
api_get_json /asset/group/abcd >&5
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 6. group_with_id_of_DC ******************************************************************"
echo "***************************************************************************************************"
test_it "group_with_id_of_DC"
curlfail_push_expect_400
api_get_json /asset/group/1 >&5
print_result $?
curlfail_pop

echo "********* asset_one_group.sh **********************************************************************"
echo "********* 7. group_with_id_of_group ***************************************************************"
echo "***************************************************************************************************"
PARSED_REPLY=$(echo $(api_get_json /asset/groups) | $JSONSH -x id)
ID_1=$(echo "${PARSED_REPLY}" | cut -d '"' -f 6)
ID_GRP_2=`echo $ID_1 | cut -d ' ' -f 2`
test_it "group_with_id_of_group"
curlfail_push_expect_noerrors
api_get_json /asset/group/${ID_GRP_2} >&5
print_result $?
curlfail_pop

echo
echo "###################################################################################################"
echo "********* asset_one_group.sh *********************** END ******************************************"
echo "###################################################################################################"
echo
