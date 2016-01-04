
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

test_it "license-related directories should exist"
# TODO: Writability by *specific* account - not checked so far
RES=0
echo "SUT_IS_REMOTE =   $SUT_IS_REMOTE"
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && \
      [ -n "$DATADIR" ] && [ -n "$LICENSE_DIR" ] && [ -n "$TESTPASS" ] && \
      echo "DATADIR     =     $DATADIR" && \
      echo "LICENSE_DIR =     $LICENSE_DIR" && \
      echo "TESTPASS    =     $TESTPASS" && \
      sut_run "ls -al '${DATADIR}' '${LICENSE_DIR}' && [ -d '${DATADIR}' ] && [ -d '${LICENSE_DIR}' ] && [ -w '${DATADIR}' ] && [ -w '${LICENSE_DIR}' ]"
    ) || RES=$?
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && \
      [ -n "$DATADIR" ] && [ -n "$LICENSE_DIR" ] && [ -n "$TESTPASS" ] && \
      echo "DATADIR     =     $DATADIR" && \
      echo "LICENSE_DIR =     $LICENSE_DIR" && \
      echo "TESTPASS    =     $TESTPASS" && \
      ls -al "${DATADIR}" "${LICENSE_DIR}" && \
      [ -d "${DATADIR}" ] && [ -d "${LICENSE_DIR}" ] && \
      [ -w "${DATADIR}" ] && [ -w "${LICENSE_DIR}" ]
    ) || RES=$?
fi
[ "$RES" != 0 ] && logmsg_error "Something is wrong with 'run_tntnet*.env' settings files or with license-related directories or with access settings, tests below are likely to fail!"
print_result $RES

test_it "verify that license/current is a symlink to a not-empty readable file"
RES=0
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && [ -n "$LICENSE_DIR" ] && \
      sut_run "[ -s '${LICENSE_DIR}/current' ] && [ -r '${LICENSE_DIR}/current' ] && [ -h '${LICENSE_DIR}/current' ]"
    ) || RES=$?
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && [ -n "$LICENSE_DIR" ] && \
      [ -s "${LICENSE_DIR}/current" ] && \
      [ -r "${LICENSE_DIR}/current" ] && \
      [ -h "${LICENSE_DIR}/current" ]
    ) || RES=$?
fi
print_result $RES

test_it "verify that license/1.0 is a not-empty readable file (or symlink to one)"
RES=0
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && [ -n "$LICENSE_DIR" ] && \
      sut_run "[ -s '${LICENSE_DIR}/1.0' ] && [ -r '${LICENSE_DIR}/1.0' ]"
    ) || RES=$?
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && [ -n "$LICENSE_DIR" ] && \
      [ -s "${LICENSE_DIR}/1.0" ] && \
      [ -r "${LICENSE_DIR}/1.0" ]
    ) || RES=$?
fi
print_result $RES

logmsg_info "Removing the license file before test, if exists: license becomes not-accepted"
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && [ -n "$DATADIR" ] && \
      sut_run "rm -f '${DATADIR}/license'" ) || true
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && [ -n "$DATADIR" ] && \
      rm -f "${DATADIR}"/license ) || true
fi


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
logmsg_info "Prepare test conditions: remove the license text file (which the current symlink points to)"
RES=0
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    #sut_run "mv -f /usr/share/bios/license/current /usr/share/bios/license/org-current ; mv -f /usr/share/bios/license/1.0 /usr/share/bios/license/org-1.0"
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && \
      [ -n "$LICENSE_DIR" ] && \
      sut_run "mv -f '${LICENSE_DIR}/current' '${LICENSE_DIR}/x-current' && mv -f '${LICENSE_DIR}/1.0' '${LICENSE_DIR}/x-1.0' && [ ! -e '${LICENSE_DIR}/current' ]"
    ) || { RES=$?; logmsg_error "Could not prepare the remote/VTE test well"; }
else
    echo "BUILDSUBDIR =     $BUILDSUBDIR"
    # Tests for local-source builds: license data are in $BUILDSUBDIR/tests/fixtures/license and are symlinks to the ../../../COPYING file
    mv -f "$BUILDSUBDIR/COPYING" "$BUILDSUBDIR/org-COPYING" \
      || { RES=$?; logmsg_error "Could not prepare the local/CI test well"; }
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && \
      [ -n "$LICENSE_DIR" ] && \
      mv -f "${LICENSE_DIR}/current" "${LICENSE_DIR}/x-current" && \
      mv -f "${LICENSE_DIR}/1.0" "${LICENSE_DIR}/x-1.0" && \
      [ ! -e "${LICENSE_DIR}/current" ]
    ) || { RES=$?; logmsg_error "Could not prepare the local/CI test well"; }
fi # SUT_IS_REMOTE

### This GET should produce an error message in JSON about missing file
curlfail_push_expect_500
logmsg_info "Try to read license (should fail)"
CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_get_json '/admin/license' >&5 || RES=$?
curlfail_pop

logmsg_info "Clean up after test (restore license file)..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && \
      [ -n "$LICENSE_DIR" ] && \
      sut_run "mv -f '${LICENSE_DIR}/x-current' '${LICENSE_DIR}/current' && mv -f '${LICENSE_DIR}/x-1.0' '${LICENSE_DIR}/1.0'"
    ) || { RES=$?; logmsg_error "Could not un-prepare the remote/VTE test well"; }
else
    mv -f "$BUILDSUBDIR/org-COPYING" "$BUILDSUBDIR/COPYING" \
      || { RES=$?; logmsg_error "Could not un-prepare the local/CI test well"; }
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && \
      [ -n "$LICENSE_DIR" ] && \
      mv -f "${LICENSE_DIR}/x-current" "${LICENSE_DIR}/current" && \
      mv -f "${LICENSE_DIR}/x-1.0" "${LICENSE_DIR}/1.0"
    ) || { RES=$?; logmsg_error "Could not un-prepare the local/CI test well"; }
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
echo "********* 10a. cannot save the license if current_license file is not a symlink *******************"
echo "***************************************************************************************************"
#*#*#*#*#* 00_license.sh - subtest 10 - TODO, 500?
#    echo '{"errors":[{"message":"Internal Server Error. Error saving license acceptance or getting license version, check integrity of storage.","code":42}]}' >&5

test_it "cannot save the accepted_license if current_license is not a symlink"
curlfail_push_expect_500
RES=0
# Make ".../license/current" a file instead of symlink (so we can not find
# the license version number with our current approach and expectations).
# TODO: Manupulations with /var/lib/bios directory should be better locked
# against intermittent errors (test if src/tgt dirs exist, etc.)
logmsg_info "Prepare test conditions: current_license becomes a file, not symlink..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && \
      [ -n "$LICENSE_DIR" ] && \
      sut_run "mv -f '${LICENSE_DIR}/current' '${LICENSE_DIR}/x-current' && echo TEST > '${LICENSE_DIR}/current' && [ -s '${LICENSE_DIR}/current' ] && [ ! -h '${LICENSE_DIR}/current' ]"
    ) || { RES=$?; logmsg_error "Could not prepare the remote/VTE test well"; }
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && \
      [ -n "$LICENSE_DIR" ] && \
      mv -f "${LICENSE_DIR}/current" "${LICENSE_DIR}/x-current" && \
      echo TEST > "${LICENSE_DIR}/current" && \
      [ -s "${LICENSE_DIR}/current" ] && \
      [ ! -h "${LICENSE_DIR}/current" ]
    ) || { RES=$?; logmsg_error "Could not prepare the local/CI test well"; }
fi # SUT_IS_REMOTE
logmsg_info "Try to accept license (should fail)"
CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_auth_post_json '/admin/license' "foobar" >&5 || RES=$?
curlfail_pop
logmsg_info "Clean up after test (restore directory for license and other data)..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && \
      [ -n "$LICENSE_DIR" ] && \
      sut_run "rm -f '${LICENSE_DIR}/current' && mv -f  '${LICENSE_DIR}/x-current' '${LICENSE_DIR}/current' && [ -h '${LICENSE_DIR}/current' ] && [ -s '${LICENSE_DIR}/current' ] && [ -r '${LICENSE_DIR}/current' ]"
    ) || { RES=$?; logmsg_error "Could not un-prepare the remote/VTE test well"; }
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && \
      [ -n "$LICENSE_DIR" ] && \
      rm -f "${LICENSE_DIR}/current" && \
      mv -f "${LICENSE_DIR}/x-current" "${LICENSE_DIR}/current" && \
      [ -h "${LICENSE_DIR}/current" ] && \
      [ -s "${LICENSE_DIR}/current" ] && \
      [ -r "${LICENSE_DIR}/current" ]
    ) || { RES=$?; logmsg_error "Could not un-prepare the local/CI test well"; }
fi # SUT_IS_REMOTE
print_result $RES

echo "********* 00_license.sh ***************************************************************************"
echo "********* 10b. cannot save the license into a non-directory ***************************************"
echo "***************************************************************************************************"
#*#*#*#*#* 00_license.sh - subtest 10 - TODO, 500?
#    echo '{"errors":[{"message":"Internal Server Error. Error saving license acceptance or getting license version, check integrity of storage.","code":42}]}' >&5

test_it "cannot save the license into a non-directory"
curlfail_push_expect_500
RES=0
# Make it a file instead of directory (so no file can be created under it)
# TODO: Manupulations with /var/lib/bios directory should be better locked
# against intermittent errors (test if src/tgt dirs exist, etc.)
logmsg_info "Prepare test conditions: location for license becomes a file, not directory..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && [ -n "$DATADIR" ] && \
      sut_run "rm -f '${DATADIR}/license' && mv -f '${DATADIR}' '${DATADIR}'.x && echo qwe > '${DATADIR}'" \
    ) || { RES=$?; logmsg_error "Could not prepare the remote/VTE test well"; }
else
    # Tests for local-source builds: license data are in $BUILDSUBDIR/tests/fixtures/license and are symlinks to the ../../../COPYING file
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && [ -n "$DATADIR" ] && { \
        rm -f "${DATADIR}"/license && \
        mv -f "${DATADIR}" "${DATADIR}".x && \
        echo qwe > "${DATADIR}"
      } ) || { RES=$?; logmsg_error "Could not prepare the local/CI test well"; }
fi # SUT_IS_REMOTE
logmsg_info "Try to accept license (should fail)"
CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_auth_post_json '/admin/license' "foobar" >&5 || RES=$?
curlfail_pop
logmsg_info "Clean up after test (restore directory for license and other data)..."
if [ "$SUT_IS_REMOTE" = yes ]; then
    # TODO: Maybe this should consider Eaton EULA as well/instead
    ( ALTROOT=/ . "$CHECKOUTDIR/tests/CI/run_tntnet_packaged.env" && [ -n "$DATADIR" ] && \
      sut_run "rm -f '${DATADIR}' && mv -f '${DATADIR}'.x '${DATADIR}'"
    ) || { RES=$?; logmsg_error "Could not un-prepare the remote/VTE test well"; }
else
    ( . "$BUILDSUBDIR/tests/CI/run_tntnet_make.env" && [ -n "$DATADIR" ] && { \
        rm -f "${DATADIR}" && \
        mv -f "${DATADIR}".x "${DATADIR}"
      } ) || { RES=$?; logmsg_error "Could not un-prepare the local/CI test well"; }
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
