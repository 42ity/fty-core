
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
#! \file asset_devices.sh
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* asset_devices.sh ************************* START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"

# Assumption: initdb + load_data files are uploaded.
# So, make sure this is true:
init_script_sampledata || exit $?

# Need to check number of expected rows in the table
test_it "verify_number_of_sample_assets"
ASSETS_NUMBER="$(do_select 'select count(id) as assets_count from v_bios_asset_element')"
logmsg_info "ASSETS_NUMBER: $ASSETS_NUMBER,    expected: 35"
[ "$ASSETS_NUMBER" -eq 35 ]
print_result $?

echo "********* asset_devices.sh ************************************************************************"
echo "********* 1. list_of_all_devices ******************************************************************"
echo "***************************************************************************************************"
curlfail_push_expect_noerrors
test_it "list_of_all_devices"
api_get_json /asset/devices >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 2. list_of_devices_with_subtype_ups *****************************************************"
echo "***************************************************************************************************"
curlfail_push_expect_noerrors
test_it "list_of_devices_with_subtype_ups"
api_get_json /asset/devices?subtype="ups" >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 3. list_of_devices_with_subtype_feed ****************************************************"
echo "***************************************************************************************************"
curlfail_push_expect_noerrors
test_it "list_of_devices_with_subtype_feed"
api_get_json /asset/devices?subtype="feed" >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 4. list_of_devices_with_subtype_server **************************************************"
echo "***************************************************************************************************"
curlfail_push_expect_noerrors
test_it "list_of_devices_with_subtype_server"
api_get_json /asset/devices?subtype=server >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 5. list_of_devices_with_subtype_epdu ****************************************************"
echo "***************************************************************************************************"
curlfail_push_expect_noerrors
test_it "list_of_devices_with_subtype_epdu"
api_get_json /asset/devices?subtype=epdu >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 6. list_of_devices_with_wrong_subtype_upsx **********************************************"
echo "***************************************************************************************************"
curlfail_push_expect_400
test_it "list_of_devices_with_wrong_subtype_upsx"
api_get_json /asset/devices?subtype=upsx >&5
print_result $?
curlfail_pop

echo "********* asset_devices.sh ************************************************************************"
echo "********* 7. list_of_devices_with_two_subtypes ****************************************************"
echo "***************************************************************************************************"
test_it "list_of_devices_with_two_subtypes"
api_get_json /asset/devices?subtype="ups","feed" >&5
print_result $?

echo "********* asset_devices.sh ************************************************************************"
echo "********* 8. list_of_devices_with_empty_subtype ***************************************************"
echo "***************************************************************************************************"
test_it "8. list_of_devices_with_empty_subtype"
api_get_json /asset/devices?subtype= >&5
print_result $?

echo "********* asset_devices.sh ************************************************************************"
echo "********* 9. list_of_devices_with_missing_subtype *************************************************"
echo "***************************************************************************************************"
test_it "list_of_devices_with_missing_subtype"
api_get_json /asset/devices?subtype= >&5
print_result $?

echo "********* asset_devices.sh ************************************************************************"
echo "********* 10. list_of_devices_with_wrong_format ***************************************************"
echo "***************************************************************************************************"
test_it "list_of_devices_with_missing_subtype"
api_get_json /asset/devices?subXtype=epdu >&5
print_result $?

echo "********* asset_devices.sh ************************************************************************"
echo "********* 11. list_of_OK_argument_and_empty_list_as_result ****************************************"
echo "***************************************************************************************************"
test_it "list_of_OK_arguments_and_empty_list_as_result"
REZ=0
for i in switch storage genset N_A sts;do
   if [ "$i" != "N_A" ];then
       api_get_json /asset/devices?subtype=$i >&5
       REZ=$(expr $REZ + $?)
   else
       curlfail_push_expect_400
       api_get_json /asset/devices?subtype=$i >&5
       REZ=$(expr $REZ + $?)
       curlfail_pop
   fi
done
print_result $REZ


echo "********* asset_devices.sh ************************************************************************"
echo "********* 12. no_devices_present ******************************************************************"
echo "***************************************************************************************************"
# delete all assets, no DC are present
loaddb_initial

test_it "no_devices_present"
api_get_json /asset/devices >&5
print_result $?

#fill DB again
loaddb_default

echo
echo "###################################################################################################"
echo "********* asset_devices.sh ************************* END ******************************************"
echo "###################################################################################################"
echo
