
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


#! \file topology_location_001.sh
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief Not yet documented file

echo "********* 1. From_top_but_not_recursive ***********************************************************"
echo "***************************************************************************************************"
test_it "From_top_but_not_recursive"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000 >&5
print_result $?
curlfail_pop

echo "********* 2. From_top_but_and_recursive ***********************************************************"
echo "***************************************************************************************************"
test_it "From_top_but_and_recursive"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes >&5
print_result $?
curlfail_pop

echo "********* 3. Top_recursive_and_filter_rooms *******************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter_rooms"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes\&filter=rooms >&5
print_result $?
curlfail_pop

echo "********* 4. Top_and_filter_rooms *****************************************************************"
echo "***************************************************************************************************"
test_it "Top_and_filter_rooms"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=rooms >&5
print_result $?
curlfail_pop

echo "********* 5. Top_recursive_and_filter_devices *****************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter_devices"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* 6. Top_not_recursive_and_filter_devices *************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=no\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* 7.Top_and_filter_rows *******************************************************************"
echo "***************************************************************************************************"
test_it "Top_and_filter_rows"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=rows >&5
print_result $?
curlfail_pop

echo "********* 8. Top_recursive_and_filter_rack ********************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter_rack"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=racks\&recursive=yes >&5
print_result $?
curlfail_pop

echo "********* 9. Top_and_filter_groups ****************************************************************"
echo "***************************************************************************************************"
test_it "Top_and_filter_groups"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=groups >&5
print_result $?
curlfail_pop

echo "********* 10. Top_and_filter_devices **************************************************************"
echo "***************************************************************************************************"
test_it "Top_and_filter_devices"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* 11. Top_recursive_and_filter_devices ****************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter_devices"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&filter=devices\&recursive=yes >&5
print_result $?
curlfail_pop

echo "********* 12. End_7032 ****************************************************************************"
echo "***************************************************************************************************"
test_it "End_7032"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7032 >&5
print_result $?
curlfail_pop

echo "********* 13. End_7032_and_recursive=yes **************************************************************"
echo "***************************************************************************************************"
test_it "End_7032_and_recursive=yes"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7032\&recursive=yes >&5
print_result $?
curlfail_pop

echo "********* 14. End_7032_and_recursive=yes_and_filter_rows ******************************************"
echo "***************************************************************************************************"
test_it "End_7032_and_recursive=yes_and_filter_rows"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7032\&recursive=yes\&filter=rows >&5
print_result $?
curlfail_pop

echo "********* 15. End_7032_and_recursive=no_and_filter_rows ******************************************"
echo "***************************************************************************************************"
test_it "End_7032_and_recursive=no_and_filter_rows"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7032\&recursive=no\&filter=rows >&5
print_result $?
curlfail_pop

echo "********* 16. End_7026_and_recursive=yes_and_filter_rows ******************************************"
echo "***************************************************************************************************"
test_it "End_7026_and_recursive=yes_and_filter_rows"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7026\&recursive=yes\&filter=rows >&5
print_result $?
curlfail_pop

echo "********* 17. End_7026_and_recursive=yes_and_filter_rooms *****************************************"
echo "***************************************************************************************************"
test_it "End_7026_and_recursive=yes_and_filter_rooms"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7026\&recursive=yes\&filter=rooms >&5
print_result $?
curlfail_pop

echo "********* 18. End_7026_and_recursive=yes_and_filter_racks *****************************************"
echo "***************************************************************************************************"
test_it "End_7026_and_recursive=yes_and_filter_racks"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7026\&recursive=yes\&filter=racks >&5
print_result $?
curlfail_pop

echo "********* 19. End_7026_and_recursive=yes_and_filter_devices ***************************************"
echo "***************************************************************************************************"
test_it "End_7026_and_recursive=yes_and_filter_devices"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7026\&recursive=yes\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* 20. End_7026_no_recursive=yes_and_filter_racks ******************************************"
echo "***************************************************************************************************"
test_it "End_7026_and_recursive=no_and_filter_racks"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7026\&recursive=yes\&filter=racks >&5
print_result $?
curlfail_pop

echo "********* 21. Top_7004_and_recursive=no_and_filter_devices ****************************************"
echo "***************************************************************************************************"
test_it "Top_7004_and_recursive=no_and_filter_devices"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7004\&recursive=no\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* 22. Top_and_recursive=yes_and_filter_rows ***********************************************"
echo "***************************************************************************************************"
test_it "Group_and_recursive=yes_and_filter_rows"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes\&filter=rows >&5
print_result $?
curlfail_pop

echo "********* 23. Group_and_recursive=yes *************************************************************"
echo "***************************************************************************************************"
test_it "Top_and_recursive=yes_and_filter_groups"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes >&5
print_result $?
curlfail_pop

echo "********* 24. Top_and_recursive=yes_and_filter_groups *********************************************"
echo "***************************************************************************************************"
test_it "Top_and_recursive=yes_and_filter_groups"
curlfail_push_expect_noerrors
api_get_json /topology/location?from=7000\&recursive=yes\&filter=groups >&5
print_result $?
curlfail_pop

echo "********* 25. to_top ******************************************************************************"
echo "***************************************************************************************************"
test_it "to_top"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7000 >&5
print_result $?
curlfail_pop

echo "********* 26. to_srv_LOC_50 ***********************************************************************"
echo "***************************************************************************************************"
test_it "to_srv_LOC_50"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7023 >&5
print_result $?
curlfail_pop

echo "********* 27. to_ROW_LOC_20 ***********************************************************************"
echo "***************************************************************************************************"
test_it "to_ROW_LOC_20"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7005 >&5
print_result $?
curlfail_pop

echo "********* 28. to_ups_LOC_010 **********************************************************************"
echo "***************************************************************************************************"
test_it "to_ups_LOC_010"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7022
 >&5
print_result $?
curlfail_pop

echo "********* 29. to_RACK_LOC_1 ***********************************************************************"
echo "***************************************************************************************************"
test_it "to_RACK_LOC_1"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7014
 >&5
print_result $?
curlfail_pop

echo "********* 30. to_ROOM_LOC_01 **********************************************************************"
echo "***************************************************************************************************"
test_it "to_ROOM_LOC_01"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7002
 >&5
print_result $?
curlfail_pop

echo "********* 31. to_inputpowergroup DC_LOC_01 ********************************************************"
echo "***************************************************************************************************"
test_it "to_ROOM_LOC_01"
curlfail_push_expect_noerrors
api_get_json /topology/location?to=7025
 >&5
print_result $?
curlfail_pop


