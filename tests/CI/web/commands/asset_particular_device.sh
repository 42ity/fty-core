
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


#!
# \file asset_particular_device.sh
# \author Michal Hrusecky <MichalHrusecky@Eaton.com>
# \author Jim Klimov <JimKlimov@Eaton.com>
# \brief Not yet documented file

test_it
RES=0
for i in 5 6 7 8; do
    api_get_json /asset/device/$i >&5 || RES=$?
done
print_result $RES
