
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

echo "********* 1. license_status_not_ok ****************************************************************"
echo "***************************************************************************************************"
test_it "license_status_not_ok"
api_get_json '/admin/license/status' >&5
print_result $?

echo "********* 2. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request without license"
curlfail_push_expect_401
api_get_json /admin/systemctl/list >&5
print_result $?
curlfail_pop

echo "********* 3. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request without license"
curlfail_push_expect_403
api_get_json /admin/alerts >&5
print_result $?
curlfail_pop

echo "********* 4. license_acceptance_unauthorised ******************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
curlfail_push_expect_401
api_post '/admin/license' "foobar" > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop

echo "********* 5. license_acceptance *******************************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
api_auth_post '/admin/license' "foobar" >&5
print_result $?

echo "********* 6. license_status_ok ********************************************************************"
echo "***************************************************************************************************"
test_it "license_status_ok"
api_get_json '/admin/license/status' | sed 's|\(accepted_at":"\)[0-9]*"|\1XXX"|' >&5
print_result $?

echo "********* 7. license_text *************************************************************************"
echo "***************************************************************************************************"
test_it "license_text"
TEXT=`api_get '/admin/license' | grep GNU | wc -l`
echo TEXT = $TEXT
if [ $TEXT -gt 0 ]; then
   echo '{"text":"yes"}'
else
   echo '{"text":"no"}'
fi
[ $TEXT -gt 0 ]
print_result $?

echo "********* 8. missing_license_text *****************************************************************"
echo "***************************************************************************************************"
test_it "missing_license_text"
curlfail_push_expect_500
echo "BUILDSUBDIR =     $BUILDSUBDIR"
mv $BUILDSUBDIR/COPYING $BUILDSUBDIR/org-COPYING
#api_post '/asset' '{"name":"dc_name_test_25","type":"datacenter","sub_type":"","location":"","status":"nonactive","business_critical":"yes","priority":"P1","ext":{"asset_tag":"TEST0025","address":"ASDF"}}' > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5

api_get '/admin/license' > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop
mv $BUILDSUBDIR/org-COPYING $BUILDSUBDIR/COPYING

echo "********* 9. disabled_method_delete ***************************************************************"
echo "***************************************************************************************************"
test_it "disabled_method_delete "
curlfail_push_expect_405
api_auth_delete '/admin/license/status' > /dev/null && echo "$OUT_CURL" | $JSONSH -N
echo "$OUT_CURL" | $JSONSH -N | grep -e '{"errors":\[{"message":"Http method.*not allowed.*code":45'
#grep -e '{\"errors\":[{\"message\":\"Http method.*not allowed.\",\"code":45}]}'
print_result $?
curlfail_pop

echo "********* 10. cannot save the license *************************************************************"
echo "***************************************************************************************************"
test_it "cannot save the license"
curlfail_push_expect_500
rm -f /var/lib/bios/license $CHECKOUTDIR/var/bios/license
rm -rf $CHECKOUTDIR/var/bios;touch $CHECKOUTDIR/var/bios
api_auth_post '/admin/license' "foobar" > /dev/null && echo "$OUT_CURL" | $JSONSH -N  >&5
print_result $?
curlfail_pop
rm -f $CHECKOUTDIR/var/bios;mkdir $CHECKOUTDIR/var/bios

echo "********* 11. license_acceptance ******************************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
api_auth_post '/admin/license' "foobar" >&5
print_result $?
