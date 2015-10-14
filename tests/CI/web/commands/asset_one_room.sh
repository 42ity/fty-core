
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


#! \file asset_one_room1.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Alena Chernikava <AlenaChernikava@Eaton.com>
#  \brief Not yet documented file

echo "********* 1. read_room_with_id_of_another_element ***************************************************"
echo "***************************************************************************************************"
test_it "read_room_with_id_of_another_element"
curlfail_push_expect_400
api_get_json /asset/room/1 >&5
print_result $?
curlfail_pop

echo "********* 2. request_to existing_room ***************************************************************"
echo "***************************************************************************************************"
test_it "request_to existing_room"
api_get_json /asset/room/2 >&5
print_result $?

echo "********* 3. request_to_not_existing_id ***********************************************************"
echo "***************************************************************************************************"
test_it "request_to_not_existing_id"
curlfail_push_expect_404
api_get_json /asset/room/100 >&5
print_result $?
curlfail_pop

echo "********* 4. request_to_id_0 **********************************************************************"
echo "***************************************************************************************************"
test_it "request_to_id_0"
curlfail_push_expect_400
api_get_json /asset/room/0 >&5
print_result $?
curlfail_pop

echo "********* 5. request_to_too_long_id ***************************************************************"
echo "***************************************************************************************************"
test_it "request_to_too_long_id"
curlfail_push_expect_400
api_get_json /asset/room/12345678901234567890 >&5
print_result $?
curlfail_pop

echo "********* 6. request_to_not_valid_id **************************************************************"
echo "***************************************************************************************************"
test_it "request_to_not_valid_id"
curlfail_push_expect_404
api_get_json /asset/room/123456x78901234567890 >&5
print_result $?
curlfail_pop

echo "********* 7. request_to_missing_id ****************************************************************"
echo "***************************************************************************************************"
test_it "request_to_missing_id"
curlfail_push_expect_400
api_get_json /asset/room/ >&5
print_result $?
curlfail_pop
