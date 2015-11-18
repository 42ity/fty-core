
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


#! \file rack_total.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \brief Not yet documented file

test_it
RES=0
api_get_json /metric/computed/rack_total?arg1=21\&arg2=total_power | \
    sed -r -e 's/"total_power": *[-+eE.0-9]+/"total_power":10.0/' >&5 || RES=$?

if false;then
curlfail_push_expect_404
api_get_json /metric/computed/rack_total?arg1=424242\&arg2=total_power >&5 || RES=$?
curlfail_pop

api_get_json /metric/computed/rack_total?arg1=21,4\&arg2=avg_power_last_week,avg_power_last_day | \
  sed -r -e 's/"avg_power_last_week": *[-+eE.0-9]+/"avg_power_last_week":10.0/g' \
         -e 's/"avg_power_last_day": *[-+eE.0-9]+/"avg_power_last_day":23.4/g'  >&5 || RES=$?
fi
print_result $RES
