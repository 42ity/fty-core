
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


#! \file   ui_properties.sh
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Alena Chernikava <AlenaChernikava@Eaton.com>
#  \author Gerald Guillaume <GeraldGuillaume@Eaton.com>
#  \brief  test a  GET and PUT method for REST API call
#           /api/v1/ui/properties

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

test_it "GET_ui_properties_1"
api_get_json /ui/properties >&5
print_result $?

test_it "PUT_ui_properties_1"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value42"}' >/dev/null
print_result $?

test_it "GET_ui_properties_2"
api_get_json /ui/properties >&5
print_result $?

test_it "PUT_ui_properties_2"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value2"}' >/dev/null
print_result $?
