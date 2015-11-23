
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

echo
echo "###################################################################################################"
echo "********* 00_license.sh **************************** START ****************************************"
echo "###################################################################################################"
echo

sut_run "rm -f /var/lib/bios/license $CHECKOUTDIR/var/bios/license" || true

echo "********* 00_license.sh ***************************************************************************"
echo "********* 1. license_status_not_ok ****************************************************************"
echo "***************************************************************************************************"
test_it "license_status_not_ok"
api_get_json '/admin/license/status' >&5
print_result $?

echo "********* 00_license.sh ***************************************************************************"
echo "********* 2. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request without license"
curlfail_push_expect_401
api_get_json /admin/systemctl/list >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 3. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request without license"
curlfail_push_expect_403
api_get_json /admin/alerts >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 4. license_acceptance_unauthorised ******************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
curlfail_push_expect_401
api_post_json '/admin/license' "foobar" >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 5. license_acceptance *******************************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
api_auth_post_json '/admin/license' "foobar" >&5
print_result $?

echo "********* 00_license.sh ***************************************************************************"
echo "********* 6. license_status_ok ********************************************************************"
echo "***************************************************************************************************"
test_it "license_status_ok"
api_get_json '/admin/license/status' | sed 's|\(accepted_at":"\)[0-9]*"|\1XXX"|' >&5
print_result $?

echo "********* 00_license.sh ***************************************************************************"
echo "********* 7. license_text *************************************************************************"
echo "***************************************************************************************************"
test_it "license_text"
### This GET should return plaintext license text "as is"
TEXT="`api_get_content '/admin/license' | grep GNU | wc -l`"
echo "TEXT = $TEXT (lines in license text)"
if [ "$TEXT" -gt 0 ]; then
   echo '{"text":"yes"}'
else
   echo '{"text":"no"}'
fi >&5
[ "$TEXT" -gt 0 ]
print_result $?

echo "********* 00_license.sh ***************************************************************************"
echo "********* 8. missing_license_text *****************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#* 00_license.sh - subtest 8 - TODO, 500?
test_it "missing_license_text"
curlfail_push_expect_500
echo "BUILDSUBDIR =     $BUILDSUBDIR"
mv $BUILDSUBDIR/COPYING $BUILDSUBDIR/org-COPYING
### This GET should produce an error message in JSON about missing file
api_get_json '/admin/license' >&5
RES=$?
curlfail_pop
mv $BUILDSUBDIR/org-COPYING $BUILDSUBDIR/COPYING
print_result $RES

echo "********* 00_license.sh ***************************************************************************"
echo "********* 9. disabled_method_delete ***************************************************************"
echo "***************************************************************************************************"
test_it "disabled_method_delete "
curlfail_push_expect_405
api_auth_delete_json '/admin/license/status' >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 10. cannot save the license *************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#* 00_license.sh - subtest 10 - TODO, 500?
test_it "cannot save the license"
curlfail_push_expect_500
rm -f /var/lib/bios/license $CHECKOUTDIR/var/bios/license
rm -rf $CHECKOUTDIR/var/bios;touch $CHECKOUTDIR/var/bios
api_auth_post_json '/admin/license' "foobar" >&5
RES=$?
curlfail_pop
rm -f $CHECKOUTDIR/var/bios;mkdir $CHECKOUTDIR/var/bios
print_result $RES

echo "********* 00_license.sh ***************************************************************************"
echo "********* 11. license_acceptance ******************************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance"
api_auth_post_json '/admin/license' "foobar" >&5
print_result $?

echo
echo "###################################################################################################"
echo "********* 00_license.sh **************************** END ******************************************"
echo "###################################################################################################"
echo
