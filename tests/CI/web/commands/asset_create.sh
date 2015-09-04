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
#! \file   asset_create.sh
#  \brief  CI tests for asset create and delete calls
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>

curlfail_push_expect_400
test_it "asset_no_content"
api_auth_post /asset '' >/dev/null
print_result $?
curlfail_pop

curlfail_push_expect_400
test_it "asset_no_content2"
api_auth_post /asset '{}' >/dev/null
print_result $?
curlfail_pop

test_it "asset_create"
api_auth_post /asset '{"name":"dc_name_test","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B123","address":"ASDF"}}' >/dev/null
PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x id`
#test the reply
echo "${PARSED_REPLY}" | cut -d '"' -f 4 | grep -q -E '^[0-9]+$'
print_result $?

curlfail_push_expect_400
test_it "asset_create_dup" >/dev/null
api_auth_post /asset '{"name":"dc_name_test","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B123","address":"ASDF"}}' >/dev/null
print_result $?
curlfail_pop

# remove the asset - it'll fail if previous attempt will fail too
test_it "asset_delete_previously_inserted"
ID=`echo "${PARSED_REPLY}" | cut -d '"' -f 4`
api_auth_delete /asset/${ID} >/dev/null
print_result $?
