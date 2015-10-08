
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


#! \file asset_particular_device.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief Not yet documented file

echo "********* 1. device_with_id_0 *********************************************************************"
echo "***************************************************************************************************"
test_it "device_with_id_0"
curlfail_push_expect_400
api_get_json /asset/device/0 >&5
print_result $?
curlfail_pop

echo "********* 2. device_with_non_exist_id *************************************************************"
echo "***************************************************************************************************"
test_it "device_with_non_exist_id"
curlfail_push_expect_404
api_get_json /asset/device/100 >&5
print_result $?
curlfail_pop

echo "********* 3. device_with_negative_id **************************************************************"
echo "***************************************************************************************************"
test_it "device_with_negative_id"
curlfail_push_expect_400
api_get_json /asset/device/-1 >&5
print_result $?
curlfail_pop

echo "********* 4. device_without_id ********************************************************************"
echo "***************************************************************************************************"
test_it "device_without_id"
curlfail_push_expect_400
api_get_json /asset/device/ >&5 
print_result $?
curlfail_pop

echo "********* 5. device_with_wrong_id *****************************************************************"
echo "***************************************************************************************************"
test_it "device_with_wrong_id"
curlfail_push_expect_400
api_get_json /asset/device/abcd >&5
print_result $?
curlfail_pop

echo "********* 6. device_with_id_of_DC *****************************************************************"
echo "***************************************************************************************************"
test_it "device_with_id_of_DC"
curlfail_push_expect_400
api_get_json /asset/device/1 >&5
print_result $?
curlfail_pop

echo "********* 7. device_with_id_of_device *************************************************************"
echo "***************************************************************************************************"
test_it "device_with_id_of_device"
curlfail_push_expect_noerrors
api_get_json /asset/device/23 >&5
print_result $?
curlfail_pop
