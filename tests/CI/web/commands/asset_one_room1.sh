
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

test_it
curlfail_push_expect_404
api_get_json /asset/room/1 >&5
print_result $?
curlfail_pop
