
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
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \brief Not yet documented file

curlfail_push_expect_400

test_it "topology/power__bad_input__1"
api_get '/topology/power?from=x' > /dev/null
print_result $?

test_it "topology/power__bad_input__2"
api_get '/topology/power?from=21474836470000000000000000' > /dev/null
print_result $?

test_it "topology/power__bad_input__3"
api_get '/topology/power?from=' >/dev/null
print_result $?

test_it "topology/power__bad_input__4"
api_get '/topology/power?from=asd54' >/dev/null
print_result $?

test_it "topology/power__bad_input__5"
api_get '/topology/power?from=%20+%20' >/dev/null
print_result $?

test_it "topology/power__bad_input__6"
api_get '/topology/power?to=x' >/dev/null
print_result $?

test_it "topology/power__bad_input__7"
api_get '/topology/power?to=21474836470000000000000000' >/dev/null
print_result $?

test_it "topology/power__bad_input__8"
api_get '/topology/power?to=' >/dev/null
print_result $?

test_it "topology/power__bad_input__9"
api_get '/topology/power?to=asd54' >/dev/null
print_result $?

test_it "topology/power__bad_input__10"
api_get '/topology/power?to=%20+%20' >/dev/null
print_result $?



test_it "topology/power__bad_input__11"
api_get '/topology/power?to=x&from=y' >/dev/null
print_result $?

test_it "topology/power__bad_input__12"
api_get '/topology/power?to=x&filter_dc=y' >/dev/null
print_result $?

test_it "topology/power__bad_input__13"
api_get '/topology/power?to=x&filter_group=y' >/dev/null
print_result $?



test_it "topology/power__bad_input__14"
api_get '/topology/power?from=x&filter_dc=y' >/dev/null
print_result $?

test_it "topology/power__bad_input__15"
api_get '/topology/power?from=x&filter_group=y' >/dev/null
print_result $?


test_it "topology/power__bad_input__16"
api_get '/topology/power?from=x&to=y' >/dev/null
print_result $?


test_it "topology/power__bad_input__17"
api_get '/topology/power?filter_dc=x&to=y' >/dev/null
print_result $?

test_it "topology/power__bad_input__18"
api_get '/topology/power?filter_group=x&to=y' >/dev/null
print_result $?


test_it "topology/power__bad_input__19"
api_get '/topology/power?filter_dc=x&from=y' >/dev/null
print_result $?

test_it "topology/power__bad_input__20"
api_get '/topology/power?filter_group=x&from=y' >/dev/null
print_result $?

# end the long line of expected HTTP-400's
curlfail_pop



curlfail_push_expect_404
test_it "topology/power__bad_input__21"
api_get '/topology/power?from=5019' >/dev/null
print_result $?
curlfail_pop

curlfail_push_expect_400
test_it "topology/power__bad_input__22"
api_get '/topology/power?from=4998' >/dev/null
print_result $?
curlfail_pop

curlfail_push_expect_404
test_it "topology/power__bad_input__23"
api_get '/topology/power?to=5019' >/dev/null
print_result $?
curlfail_pop

curlfail_push_expect_400
test_it "topology/power__bad_input__24"
api_get '/topology/power?to=4998' >/dev/null
print_result $?
curlfail_pop

