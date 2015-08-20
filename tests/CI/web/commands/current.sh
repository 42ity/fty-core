
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
# \file current.sh
# \author Jim Klimov
# \author Michal Vyskocil
# \brief Not yet documented file

test_it dev=6
api_get_json '/metric/current?dev=6' >&5
print_result $?

test_it dev=7
api_get_json '/metric/current?dev=7' >&5
print_result $?

test_it dev=3
curlfail_push_expect_404
api_get_json '/metric/current?dev=3' >&5
print_result $?
curlfail_pop

test_it default
curlfail_push_expect_400
api_get_json '/metric/current' >&5
print_result $?
curlfail_pop
