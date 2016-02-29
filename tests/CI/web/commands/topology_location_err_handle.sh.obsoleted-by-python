
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


#! \file topology_location_err_handle.sh
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* topology_location_err_handle.sh ********** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script_topo_loc

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 1. Negative_value ***********************************************************************"
echo "***************************************************************************************************"
test_it "Negative_value"
curlfail_push_expect_400
api_get_json /topology/location?from=-1 >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 2. From_to_conflict *********************************************************************"
echo "***************************************************************************************************"
test_it "From_to_conflict"
curlfail_push_expect_400
api_get_json /topology/location?from=7000\&to=7029 >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 3. Recursive_to_conflict ****************************************************************"
echo "***************************************************************************************************"
test_it "Recursive_to_conflict"
curlfail_push_expect_400
api_get_json /topology/location?to=7022\&recursive=yes\&filter=rooms >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 4. Devices_to_conflict ******************************************************************"
echo "***************************************************************************************************"
test_it "Devices_to_conflict"
curlfail_push_expect_400
api_get_json /topology/location?to=7025\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 5. filter_bad_value *********************************************************************"
echo "***************************************************************************************************"
test_it "filter_bad_value"
curlfail_push_expect_400
api_get_json /topology/location?from=7031\&filter=abcd >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 6. Element_not_found ********************************************************************"
echo "***************************************************************************************************"
test_it "Element_not_found"
curlfail_push_expect_404
api_get_json /topology/location?to=709 >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 7.feed_by_no_devices ********************************************************************"
echo "***************************************************************************************************"
test_it "feed_by_no_devices"
curlfail_push_expect_400
api_get_json /topology/location?from=7000\&feed_by=7016 >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 8. feed_by_no_device_type ***************************************************************"
echo "***************************************************************************************************"
test_it "feed_by_no_device_type"
curlfail_push_expect_400
api_get_json /topology/location?from=7000\&feed_by=7031\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 9. filter_and_to ************************************************************************"
echo "***************************************************************************************************"
test_it "ffilter_and_to"
curlfail_push_expect_400
api_get_json /topology/location?to=7019\&filter=groups >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 10. feed_by_and_rack ********************************************************************"
echo "***************************************************************************************************"
test_it "feed_by_and_rack"
curlfail_push_expect_400
api_get_json /topology/location?from=1\&recursive=yes\&feed_by=7\&filter=racks >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 11. feed_by_and_none ********************************************************************"
echo "***************************************************************************************************"
test_it "Top_recursive_and_filter_devices"
curlfail_push_expect_400
api_get_json /topology/location?from=none\&recursive=yes\&feed_by=7023\&filter=devices >&5
print_result $?
curlfail_pop

echo "********* topology_location_err_handle.sh *********************************************************"
echo "********* 12. from or to is mandatory *************************************************************"
echo "***************************************************************************************************"
test_it "from or to is mandatory"
curlfail_push_expect_400
api_get_json /topology/location >&5
print_result $?
curlfail_pop

echo
echo "###################################################################################################"
echo "********* topology_location_error_handle.sh ******** END ******************************************"
echo "###################################################################################################"
echo

