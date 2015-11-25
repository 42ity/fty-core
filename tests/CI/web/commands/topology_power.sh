
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


#! \file   topology_power.sh
#  \author Radomir Vrajik <RadimirVrajik@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* topology_power.sh ************************ START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script_topo_pow

echo "********* topology_power.sh ***********************************************************************"
echo "********* 1. UPS - from - no I/O connected ********************************************************"
echo "***************************************************************************************************"
test_it "UPS_from_no_I_O_connected"
api_get_json /topology/power?from=5020 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 2. UPS - from - no output two inputs connected  *****************************************"
echo "***************************************************************************************************"
test_it "UPS_from_no_output_two_inputs_connected"
api_get_json /topology/power?from=5021 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 3. UPS - from - one output **************************************************************"
echo "***************************************************************************************************"
test_it "UPS_from_one_output"
api_get_json /topology/power?from=5024 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 4. UPS - from - three outputs two inputs and sockets used *******************************"
echo "***************************************************************************************************"
test_it "UPS_from_three_outputs_two_inputs_and_sockets_used"
api_get_json /topology/power?from=5028 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 5. UPS - from - three outputs ***********************************************************"
echo "***************************************************************************************************"
test_it "UPS_from_three_outputs"
api_get_json /topology/power?from=5034 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 6. UPS - from - one output **************************************************************"
echo "***************************************************************************************************"
test_it "UPS_from_one_output"
api_get_json /topology/power?from=5038 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 7. UPS - from - one output to oneself ***************************************************"
echo "***************************************************************************************************"
test_it "UPS_from_one_output_to_oneself"
api_get_json /topology/power?from=5040 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 8. UPS - from - two outputs the same dest one has socket specified other not ************"
echo "***************************************************************************************************"
test_it "UPS_from_two_outputs_the_same_dest_one_has_socket_specified_other_not"
api_get_json /topology/power?from=5041 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 9. UPS - from - sink sourced from UPS and UPS sourced from the same sink ****************"
echo "***************************************************************************************************"
test_it "UPS_from_sink_sourced_from_UPS_and_UPS_sourced_from_the_same_sink"
api_get_json /topology/power?from=5043 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 10. UPS - to - no I/O connected *********************************************************"
echo "***************************************************************************************************"
test_it "UPS_to_no_I_O_connected"
api_get_json /topology/power?to=5045 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 11. UPS - to - no input two ouputs connected ********************************************"
echo "***************************************************************************************************"
test_it "UPS_no_input_two_ouputs_connected"
api_get_json /topology/power?to=5046 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 12. UPS - to - two outputs two inputs ***************************************************"
echo "***************************************************************************************************"
test_it "UPS_to_two_outputs_two_inputs"
api_get_json /topology/power?to=5049 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 13. UPS - to - complex topology 1 *******************************************************"
echo "***************************************************************************************************"
test_it "UPS_to_complex_topology_1"
api_get_json /topology/power?to=5054 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 14. UPS - to - no sockets round to oneself **********************************************"
echo "***************************************************************************************************"
test_it "UPS_to_no_sockets_round_to_oneself"
api_get_json /topology/power?to=5061 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 15. UPS - to - cycled simple chain ******************************************************"
echo "***************************************************************************************************"
test_it "UPS_to_cycled_simple_chain"
api_get_json /topology/power?to=5062 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 16. UPS - to - complex topology 2 *******************************************************"
echo "***************************************************************************************************"
test_it "filter_group=x_to=y"
api_get_json /topology/power?to=5065 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 17. UPS - to - two connection one from named socket *************************************"
echo "***************************************************************************************************"
# TODO BUG: added the same two connection, but ignored in response
test_it "UPS_to_two_connection_one_from_named_socket"
api_get_json /topology/power?to=5074
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 18. UPS - to - connection to sink and back **********************************************"
echo "***************************************************************************************************"
test_it "UPS_to_connection_to_sink_and_back"
api_get_json /topology/power?to=5076 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 19. UPS - filter_dc - the whole powerchain in dc ****************************************"
echo "***************************************************************************************************"
test_it "UPS_filter_dc_the_whole_powerchain_in_dc"
api_get_json /topology/power?filter_dc=5078 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 20. Group has only one element **********************************************************"
echo "***************************************************************************************************"
test_it "Group_has_only_one_element"
api_get_json /topology/power?filter_group=5088 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 21. One element from PT is missing in group *********************************************"
echo "***************************************************************************************************"
test_it "One_element_from_PT_is_missing_in_group"
api_get_json /topology/power?filter_group=5089 >&5
print_result $?

echo "********* topology_power.sh ***********************************************************************"
echo "********* 22. The filter_dc with not DC id ********************************************************"
echo "***************************************************************************************************"
test_it "The filter_dc_with_not_DC_id"
api_get_json /topology/power?filter_dc=5000 >&5
print_result $?

echo
echo "###################################################################################################"
echo "********* topology_power.sh ************************* END *****************************************"
echo "###################################################################################################"
echo

