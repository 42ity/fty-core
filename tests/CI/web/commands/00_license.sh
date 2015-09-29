
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


#! \file 00_license.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief Not yet documented file

# remove the licence file, if exists, license not accepted in result
sut_run "rm -f /var/lib/bios/license $CHECKOUTDIR/var/bios/license" || true

# test that the license is not accepted
test_it "license_status_not_ok"
api_get_json '/admin/license/status' >&5
print_result $?

test_it "license_acceptance"
api_auth_post_content '/admin/license' "foobar" >&5
print_result $?

test_it "license_status_ok"
api_get_json '/admin/license/status' | sed 's|\(accepted_at":"\)[0-9]*"|\1XXX"|' >&5
print_result $?

test_it "license_text"
echo api_get '/admin/license'
TEXT=`api_get '/admin/license' | grep GNU | wc -l`
echo TEXT = $TEXT
if [ $TEXT -gt 0 ]; then
   echo '{"text":"yes"}'
else
   echo '{"text":"no"}'
fi
[ $TEXT -gt 0 ]
print_result $?
