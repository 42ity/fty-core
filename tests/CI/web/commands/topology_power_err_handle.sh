
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


#! \file   topology_power_err_handle.sh
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief Not yet documented file

curlfail_push_expect_400

echo
echo "###################################################################################################"
echo "********* topology_power_err_handle.sh ************* START ****************************************"
echo "###################################################################################################"

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 1. from_negative_value ******************************************************************"
echo "***************************************************************************************************"
test_it "from_negative_value"
api_get_json '/topology/power?from=-1' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 2. from_non_integer_value ***************************************************************"
echo "***************************************************************************************************"
test_it "from_non_integer_value"
api_get_json '/topology/power?from=x' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 3. from_too_big_value *******************************************************************"
echo "***************************************************************************************************"
test_it "from_too_big_value"
api_get_json '/topology/power?from=21474836470000000000000000' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 4. from_empty_value *******************************************************************"
echo "***************************************************************************************************"
test_it "from_empty_value"
api_get_json '/topology/power?from=' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 5. to_negative_value *******************************************************************"
echo "***************************************************************************************************"
test_it "to_negative_value"
api_get_json '/topology/power?to=-1' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 6. to_non_integer_value *****************************************************************"
echo "***************************************************************************************************"
test_it "to_non_integer_value"
api_get_json '/topology/power?to=x' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 7. to_too_big_value *********************************************************************"
echo "***************************************************************************************************"
test_it "to_too_big_value"
api_get_json '/topology/power?to=21474836470000000000000000' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 8. to_empty_value *********************************************************************"
echo "***************************************************************************************************"
test_it "to_empty_value"
api_get_json '/topology/power?to=' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 9. from_x_to_y *************************************************************************"
echo "***************************************************************************************************"
test_it "from_x_to_y"
api_get_json '/topology/power?to=x\&from=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 10. to=x_filter_dc=y ********************************************************************"
echo "***************************************************************************************************"
test_it "to=x_filter_dc=y"
api_get_json '/topology/power?to=x\&filter_dc=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 11. to=x_filter_group=y ********************************************************************"
echo "***************************************************************************************************"
test_it "to=x_filter_group=y"
api_get_json '/topology/power?to=x\&filter_group=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 12. to=x_filter_group=y ********************************************************************"
echo "***************************************************************************************************"
test_it "from=x_filter_dc=y"
api_get_json '/topology/power?from=x\&filter_dc=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 13. from=x_filter_group=y ********************************************************************"
echo "***************************************************************************************************"
test_it "from=x_filter_group=y"
api_get_json '/topology/power?from=x\&filter_group=y' >&5
print_result $?


echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 14. from=x\&to=y ********************************************************************"
echo "***************************************************************************************************"
test_it "from=x_to=y"
api_get_json '/topology/power?from=x&to=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 15. filter_dc=x\&to=y ********************************************************************"
echo "***************************************************************************************************"
test_it "filter_dc=x_to=y"
api_get_json '/topology/power?filter_dc=x&to=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 16. filter_group=x\&to=y *****************************************************************"
echo "***************************************************************************************************"
test_it "filter_group=x_to=y"
api_get_json '/topology/power?filter_group=x&to=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 17. filter_dc=x\&from=y ******************************************************************"
echo "***************************************************************************************************"
test_it "filter_dc=x_from=y"
api_get_json '/topology/power?filter_dc=x&from=y' >&5
print_result $?

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 18. filter_group=x\&from=y ***************************************************************"
echo "***************************************************************************************************"
test_it "filter_group=x_from=y"
api_get_json '/topology/power?filter_group=x&from=y' >&5
print_result $?

# end the long line of expected HTTP-400's

curlfail_pop
curlfail_push_expect_400

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 19. from_with_white_char_value **********************************************************"
echo "***************************************************************************************************"
test_it "from_with_white_char_value"
api_get_json '/topology/power?from=%20 5' >&5
print_result $?

curlfail_pop
curlfail_push_expect_400

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 20. to_with_white_char_value ************************************************************"
echo "***************************************************************************************************"
test_it "to_with_white_char_value"
api_get_json '/topology/power?to=%20 5' >&5
print_result $?

curlfail_pop
curlfail_push_expect_404

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 21. from=id_does_not_exist **************************************************************"
echo "***************************************************************************************************"
test_it "from_id_does_not_exist"
api_get_json '/topology/power?from=5019' >&5
print_result $?
curlfail_pop

curlfail_push_expect_400

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 22. from=empty_power_group **************************************************************"
echo "***************************************************************************************************"
test_it "from_empty_power_group"
api_get_json '/topology/power?from=4998' >&5
print_result $?
curlfail_pop

curlfail_push_expect_404

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 23. to=id_does_not_exist ****************************************************************"
echo "***************************************************************************************************"
test_it "to_id_does_not_exist"
api_get_json '/topology/power?to=5019' >&5
print_result $?
curlfail_pop

curlfail_push_expect_400

echo "********* topology_power_err_handle.sh ************************************************************"
echo "********* 24. to=empty_power_group ****************************************************************"
echo "***************************************************************************************************"
test_it "to_id_does_not_exist"
api_get_json '/topology/power?to=4998' >&5
print_result $?
curlfail_pop
