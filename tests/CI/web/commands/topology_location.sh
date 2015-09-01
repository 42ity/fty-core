
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


#! \file topology_location.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \author Alena Chernikava <AlenaChernikava@Eaton.com>
#  \brief Not yet documented file

curlfail_push_expect_400

test_it "topology/location__bad_input__1"
api_get_json '/topology/location?from=x' >/dev/null
print_result $?

test_it "topology/location__bad_input__2"
api_get_json '/topology/location?from=21474836470000000000000000' >/dev/null
print_result $?

test_it "topology/location__bad_input__3"
api_get_json '/topology/location?from=' >/dev/null
print_result $?

test_it "topology/location__bad_input__4"
api_get_json '/topology/location?from=asd54' >/dev/null
print_result $?

test_it "topology/location__bad_input__5"
api_get_json '/topology/location?from=%20+%20' >/dev/null
print_result $?

test_it "topology/location__bad_input__6"
api_get_json '/topology/location?to=x' >/dev/null
print_result $?

test_it "topology/location__bad_input__7"
api_get_json '/topology/location?to=21474836470000000000000000' >/dev/null
print_result $?

test_it "topology/location__bad_input__8"
api_get_json '/topology/location?to=' >/dev/null
print_result $?

test_it "topology/location__bad_input__9"
api_get_json '/topology/location?to=asd54' >/dev/null
print_result $?

test_it "topology/location__bad_input__10"
api_get_json '/topology/location?to=%20+%20' >/dev/null
print_result $?

test_it "topology/location__bad_input__11"
curlfail_push_expect_400
api_get_json '/topology/location?from=5\&to=53' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__12"
curlfail_push_expect_400
api_get_json '/topology/location?from=&to=' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__13"
curlfail_push_expect_400
api_get_json '/topology/location?from&to' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__14"
curlfail_push_expect_400
api_get_json '/topology/location?to=1&recursive=yes' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__15"
curlfail_push_expect_400
api_get_json '/topology/location?to=1&recursive=no' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__16"
curlfail_push_expect_400
api_get_json '/topology/location?to=41&recursive=x' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__17"
curlfail_push_expect_400
api_get_json '/topology/location?to=0&filter=x' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__18"
curlfail_push_expect_400
api_get_json '/topology/location?to=1&filter=rooms' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__19"
curlfail_push_expect_400
api_get_json '/topology/location?to=1&filter=rows&recursive=yes' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__20"
api_get_json '/topology/location?from=1234&filter=datacenters' >/dev/null
print_result $?

test_it "topology/location__bad_input__21"
api_get_json '/topology/location?from=4321&filter=adys' >/dev/null
print_result $?

test_it "topology/location__bad_input__22"
api_get_json '/topology/location?from=4321&filter=roo%20ms' >/dev/null
print_result $?

test_it "topology/location__bad_input__23"
api_get_json '/topology/location?from=4321&filter=row+s' >/dev/null
print_result $?

test_it "topology/location__bad_input__24"
api_get_json '/topology/location?from=1111&recursive=s' >/dev/null
print_result $?

test_it "topology/location__bad_input__25"
api_get_json '/topology/location?from=5&filter=groups&recursive=ysfd' >/dev/null
print_result $?

test_it "topology/location__bad_input__26"
api_get_json '/topology/location?from=1234&filter=datacenters&recursive=yes' >/dev/null
print_result $?

test_it "topology/location__bad_input__27"
api_get_json '/topology/location?from=1234&filter=datacenters&recursive=no' >/dev/null
print_result $?

test_it "topology/location__bad_input__28"
api_get_json '/topology/location?from=1234&filter=asd&recursive=yes' >/dev/null
print_result $?

test_it "topology/location__bad_input__29"
api_get_json '/topology/location?from=1234&filter=d&recursive=no' >/dev/null
print_result $?
curlfail_pop


curlfail_push_expect_404
test_it "topology/location__bad_input__30"
api_get_json '/topology/location?from=5019' >&5
print_result $?
curlfail_pop

test_it "topology/location__bad_input__31"
curlfail_push_expect_400
api_get_json '/topology/location?to=1&feed_by=1' >&5
print_result $?
curlfail_pop
