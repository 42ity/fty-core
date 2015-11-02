#
# Copyright (c) 2015 Eaton
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
#! \file   asset_virtual.sh
#  \brief  CI tests for asset device/virtual
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>

echo
echo "###################################################################################################"
echo "********* asset_virtual.sh ******************* START **********************************************"
echo "###################################################################################################"
echo

echo "********* 1. Create device/virtual            *****************************************************"
echo "***************************************************************************************************"
test_it "Create device/virtual"
curlfail_push_expect_noerrors
api_auth_post_json '/asset' '{"name":"dvc_virtual_test1","type":"device","sub_type":"virtual","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0003","address":"ASDF","serial_no":"ABCD0003","ip.1":"10.229.5.11"}}' > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop

echo "********* 2. Read device/virtual              *****************************************************"
echo "***************************************************************************************************"
test_it "Read device/virtual"
curlfail_push_expect_noerrors
api_auth_get_json '/asset/36' >&5
print_result $?
curlfail_pop

echo "********* 2. Export device/virtual            *****************************************************"
echo "***************************************************************************************************"
test_it "Export device/virtual"
curlfail_push_expect_noerrors
# HACK, the cmp_json does expect the json content in res files
printf '{"csv":"%s"}\n' "`api_auth_get '/asset/export' | grep '36$'`" >&5
print_result $?
curlfail_pop

echo
echo "###################################################################################################"
echo "********* asset_virtual.sh ******************* END ************************************************"
echo "###################################################################################################"
echo
