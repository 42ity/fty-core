
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


echo
echo "###################################################################################################"
echo "********* 00_license.sh **************************** START ****************************************"
echo "###################################################################################################"
echo

# remove the license file, if exists, license not accepted in result
sut_run "rm -f /var/lib/bios/license $BUILDSUBDIR/var/bios/license $CHECKOUTDIR/var/bios/license" || true

echo "********* 00_license.sh ***************************************************************************"
echo "********* 1. license_status_not_ok ****************************************************************"
echo "***************************************************************************************************"
test_it "license_status_not_ok"
api_get_json '/admin/license/status' >&5
print_result $?

echo "********* 00_license.sh ***************************************************************************"
echo "********* 2. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request_without_license::systemctl_list"
curlfail_push_expect_401
api_get_json /admin/systemctl/list >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 3. request without license **************************************************************"
echo "***************************************************************************************************"
test_it "request_without_license::alerts"
curlfail_push_expect_403
api_get_json /admin/alerts >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 4. license_acceptance_unauthorised ******************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance_notauthed"
curlfail_push_expect_401
api_post_json '/admin/license' "foobar" >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 5. license_acceptance *******************************************************************"
echo "***************************************************************************************************"
test_it "license_acceptance_authed"
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
TEXT="`api_get_content '/admin/license' | egrep -ic 'GNU|EATON'`"
echo "TEXT = $TEXT (matched lines in license text)"
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
logmsg_info "Prepare test conditions: remove the license text file"
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    sut_run "mv -f /usr/share/bios/license/current /usr/share/bios/license/org-current ; mv -f /usr/share/bios/license/1.0 /usr/share/bios/license/org-1.0"
else
    echo "BUILDSUBDIR =     $BUILDSUBDIR"
    # Tests for local-source builds: license data are in $BUILDSUBDIR/tests/fixtures/license and are symlinks to the ../../../COPYING file
    mv -f "$BUILDSUBDIR/COPYING" "$BUILDSUBDIR/org-COPYING"
fi # SUT_IS_REMOTE
### This GET should produce an error message in JSON about missing file
curlfail_push_expect_500
logmsg_info "Try to read license (should fail)"
CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_get_json '/admin/license' >&5
RES=$?
curlfail_pop
logmsg_info "Clean up after test (restore license file)..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    sut_run "mv -f /usr/share/bios/license/org-current /usr/share/bios/license/current ; mv -f /usr/share/bios/license/org-1.0 /usr/share/bios/license/1.0"
else
    mv -f "$BUILDSUBDIR/org-COPYING" "$BUILDSUBDIR/COPYING"
fi # SUT_IS_REMOTE
print_result $RES

echo "********* 00_license.sh ***************************************************************************"
echo "********* 9. disabled_method_delete ***************************************************************"
echo "***************************************************************************************************"
test_it "disabled_method_delete"
curlfail_push_expect_405
api_auth_delete_json '/admin/license/status' >&5
print_result $?
curlfail_pop

echo "********* 00_license.sh ***************************************************************************"
echo "********* 10. cannot save the license *************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#* 00_license.sh - subtest 10 - TODO, 500?
#    echo '{"errors":[{"message":"Internal Server Error. Error saving license acceptance or getting license version, check integrity of storage.","code":42}]}' >&5

test_it "cannot save the license"
curlfail_push_expect_500
# Make it a file instead of directory (so no file can be created under it)
# TODO: Manupulations with /var/lib/bios directory should be better locked
# against intermittent errors (test if src/tgt dirs exist, etc.)
logmsg_info "Prepare test conditions: location for license becomes a file, not directory..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    sut_run "rm -f /var/lib/bios/license ; mv -f /var/lib/bios /var/lib/bios.x ; touch /var/lib/bios || true"
else
    echo "CHECKOUTDIR =     $CHECKOUTDIR"
    echo "BUILDSUBDIR =     $BUILDSUBDIR"
    # Tests for local-source builds: license data are in $BUILDSUBDIR/tests/fixtures/license and are symlinks to the ../../../COPYING file
    rm -f /var/lib/bios/license $BUILDSUBDIR/var/bios/license $CHECKOUTDIR/var/bios/license; rm -rf $BUILDSUBDIR/var/bios $CHECKOUTDIR/var/bios; mv -f /var/lib/bios /var/lib/bios.x ; touch /var/lib/bios $CHECKOUTDIR/var/bios $BUILDSUBDIR/var/bios || true
fi # SUT_IS_REMOTE
logmsg_info "Try to accept license (should fail)"
CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_auth_post_json '/admin/license' "foobar" >&5
RES=$?
curlfail_pop
logmsg_info "Clean up after test (restore directory for license and other data)..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    sut_run "rm -f /var/lib/bios; mv -f /var/lib/bios.x /var/lib/bios"
else
    echo "CHECKOUTDIR =     $CHECKOUTDIR"
    echo "BUILDSUBDIR =     $BUILDSUBDIR"
    rm -f /var/lib/bios $BUILDSUBDIR/var/bios $CHECKOUTDIR/var/bios;mkdir $CHECKOUTDIR/var/bios $BUILDSUBDIR/var/bios || true; mv -f /var/lib/bios.x /var/lib/bios
fi # SUT_IS_REMOTE
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
