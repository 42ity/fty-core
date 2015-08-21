
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
# \file systemctl.sh
# \author Karol Hrdina <KarolHrdina@Eaton.com>
# \brief Not yet documented file

read -r -d '' SERVICE_OUT <<'EOF-TMPL'
{"##SERVICE_NAME##":{"ActiveState":"##ACTIVE_STATE##","SubState":"##SUB_STATE##","LoadState":"##LOAD_STATE##","UnitFileState":"##UNIT_FILE_STATE##"}}
EOF-TMPL

# Not used right now
read -r -d '' LIST_OUT <<'EOF-TMPL'
{
    "systemctl_services" : [
        ##SERVICES##
    ]
}
EOF-TMPL

expected () {
    local expected=
    # $1 valid service name
    expected="$SERVICE_OUT"
    expected=${expected/\#\#SERVICE_NAME\#\#/${1}}

    tmp=`sudo systemctl show ${1} -p ActiveState | sed -re 's/\s*ActiveState\s*=//'`
    expected=${expected/\#\#ACTIVE_STATE\#\#/$tmp}

    tmp=`sudo systemctl show ${1} -p SubState | sed -re 's/\s*SubState\s*=//'`
    expected=${expected/\#\#SUB_STATE\#\#/$tmp}

    tmp=`sudo systemctl show ${1} -p LoadState | sed -re 's/\s*LoadState\s*=//'`
    expected=${expected/\#\#LOAD_STATE\#\#/$tmp}
    
    tmp=`sudo systemctl show ${1} -p UnitFileState | sed -re 's/\s*UnitFileState\s*=//'`
    expected=${expected/\#\#UNIT_FILE_STATE\#\#/$tmp}

    echo "${expected}"
    unset expected
}

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
[ "$received" == "$tmp" ]
print_result $?

test_it "Stop mysql"
sudo systemctl stop mysql
print_result $?

test_it "Authorized status 2"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
[ "$received" == "$tmp" ]
print_result $?

test_it "Enable mysql"
sudo systemctl enable mysql
print_result $?

test_it "Authorized status 3"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
[ "$received" == "$tmp" ]
print_result $?

test_it "Disable mysql"
sudo systemctl disable mysql
print_result $?

test_it "Authorized status 4"
simple_auth_get_code "/admin/systemctl/status/mysql" received HTTP_CODE
tmp="`expected mysql`"
[ "$received" == "$tmp" ]
print_result $?

test_it "Start mysql"
sudo systemctl start mysql
print_result $?

test_it "Authorized status 5"
simple_auth_get_code '/admin/systemctl/status/mysql' received HTTP_CODE
tmp="`expected mysql`"
[ "$received" == "$tmp" ]
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
#[ "$received" == "$tmp" ]
#print_result $?



