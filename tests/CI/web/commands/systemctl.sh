
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


#! \file systemctl.sh
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief  Runs a number of scenarios for systemctl valid and invalid calls

[ -n "$CMPJSON_SH" ] && [ -x "$CMPJSON_SH" ] || \
    logmsg_error "CMPJSON_SH not defined properly!"

expected() {
    # print 'systemctl show' in format of REST API call to help comparing
    # $1 = systemd service name
    echo "{\"$1\": {"
    sudo systemctl show "$1" \
        -p ActiveState -p SubState -p LoadState -p UnitFileState \
    | awk -F= 'NR>1 {print ", "}{print "\""$1"\" : \""$2"\""}'
    echo "} }"
}

# Not used right now
read -r -d '' LIST_OUT <<'EOF-TMPL'
{
    "systemctl_services" : [
        ##SERVICES##
    ]
}
EOF-TMPL

received=
HTTP_CODE=

test_it "Not authorized"
simple_get_json_code "/admin/systemctl/list" received HTTP_CODE
[ $HTTP_CODE -eq 401 ]
print_result $?

# pick a service that is guaranteed to be there
test_it "Not authorized 2"
simple_get_json_code "/admin/systemctl/status/mysql" received HTTP_CODE
[ $HTTP_CODE -eq 401 ]
print_result $?

test_it "Not authorized 3"
simple_get_json_code "/admin/systemctl/status/malamute" received HTTP_CODE
[ $HTTP_CODE -eq 401 ]
print_result $?

test_it "Not authorized 4"
simple_post_code '/admin/systemctl/restart' '{ "service_name" : "mysql" }' received HTTP_CODE
[ $HTTP_CODE -eq 401 ]
print_result $?

test_it "Not authorized 5"
simple_post_code '/admin/systemctl/disable' '{ "service_name" : "mysql" }' received HTTP_CODE
[ $HTTP_CODE -eq 401 ]
print_result $?

test_it "Accept license now"
api_auth_post /license 

test_it "Authorized status"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
"$CMPJSON_SH" -s "$received" "$tmp"
print_result $?

test_it "Stop mysql"
sudo systemctl stop mysql
print_result $?

test_it "Authorized status 2"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
"$CMPJSON_SH" -s "$received" "$tmp"
print_result $?

test_it "Enable mysql"
sudo systemctl enable mysql
print_result $?

test_it "Authorized status 3"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
"$CMPJSON_SH" -s "$received" "$tmp"
print_result $?

test_it "Disable mysql"
sudo systemctl disable mysql
print_result $?

test_it "Authorized status 4"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
"$CMPJSON_SH" -s "$received" "$tmp"
print_result $?

test_it "Start mysql"
sudo systemctl start mysql
print_result $?

test_it "Authorized status 5"
simple_auth_get_code '/admin/systemctl/status/mysql' received HTTP_CODE
tmp="`expected mysql`"
"$CMPJSON_SH" -s "$received" "$tmp"
print_result $?

# Now that /status/<service_name> is properly tested, we'll use it for further testing

# Work in progress
#test_it "Systemctl post 1"
#simple_auth_post_code '/admin/systemctl/stop' '{ "service_name" : "mysql" }' received HTTP_CODE
#[ $HTTP_CODE -eq 200 ]
#print_result $?
#
#test_it "Systemctl post 1 - code compare"
#simple_auth_get_code '/admin/systemctl/status/mysql' tmp HTTP_CODE
#[ $HTTP_CODE -eq 200 ]
#print_result $?
#
#test_it "Systemctl post 1 - string compare"
#"$CMPJSON_SH" -s "$received" "$tmp"
#print_result $?



