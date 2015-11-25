
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


#! \file metric_computed_average__expected_failures.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* metric_computed_average__expected_failures.sh ***************** START *******************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script


curlfail_push_expect_400

test_it "metric_computed_average-missing__1"
api_get '/metric/computed/average?end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__2"
api_get '/metric/computed/average?start_ts=20130101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__3"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__4"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__5"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26' | grep '400 Bad Request'
print_result $?


test_it "metric_computed_average-wrong__1"
api_get '/metric/computed/average?start_ts=0&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__2"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=0&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__3"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=american_average&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__4"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=5ft&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

curlfail_pop

echo
echo "###################################################################################################"
echo "********* metric_computed_average__expected_failures.sh ****************** END ********************"
echo "###################################################################################################"
echo

