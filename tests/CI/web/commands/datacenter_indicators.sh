
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


#! \file datacenter_indicators.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \brief Not yet documented file

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

test_it
RES=0
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=power | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=avg_power_last_day,avg_power_last_week | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=10,19\&arg2=avg_humidity_last_day,avg_humidity_last_week | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?

curlfail_push_expect_404
api_get_json /metric/computed/datacenter_indicators?arg1=4000\&arg2=power,blablabla >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=4000,10\&arg2=power >&5 || RES=$?
curlfail_pop

print_result $RES
