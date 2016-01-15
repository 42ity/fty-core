#
# Copyright (c) 2015 Eaton
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
#! \file   auth.sh
#  \brief  CI tests for password change REST API
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

echo
echo "###################################################################################################"
echo "********* auth.sh ********************************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

# Check getting token
_test_auth() {
    api_get "/oauth2/token?username=$1&password=$2&grant_type=password"
}

_gettoken_auth_sh() {
    _test_auth "$BIOS_USER" "$BIOS_PASSWD" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p' | tail -n 1
}

if [ -n "$TESTPASS" ] && [ -x "$TESTPASS" ] ; then : ; else
    [ -n "$CHECKOUTDIR" ] && [ -d "$CHECKOUTDIR" ] && \
        TESTPASS="$CHECKOUTDIR/tools/testpass.sh" || \
        TESTPASS="/usr/libexec/bios/testpass.sh"
fi

PASS_ORIG_GOOD=""

test_it "good_login"
TOKEN="`_gettoken_auth_sh`"
[ "$TOKEN" ] && PASS_ORIG_GOOD="$BIOS_PASSWD"
print_result $?


# Check not getting token
test_it "wrong_login"
curlfail_push_expect_400
_test_auth "not$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "wrong_password"
curlfail_push_expect_400
_test_auth "$BIOS_USER" "not$BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "token_revoke"
TOKEN="`_gettoken_auth_sh`"
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "foobar" \
        -v --progress-bar "$BASE_URL/admin/license" 3> /dev/null 2> /dev/null >/dev/null
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "token=$TOKEN"  \
        -v --progress-bar "$BASE_URL/oauth2/revoke" 3> /dev/null 2> /dev/null | jsonsh_cli -N >&5
curlfail_push_expect_401
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "foobar" \
        -v --progress-bar "$BASE_URL/admin/license" 3> /dev/null 2> /dev/null | jsonsh_cli -N >&5
curlfail_pop
print_result 0
TOKEN="`_gettoken_auth_sh`"


# NOTE: There are no RESTable tests for SASL_SERVICE, because it is a system
# pre-setting verified by ci-test-restapi.sh and test_web.sh and friends:
# the "tntnet" webserver runs as user "bios", who is member of "sasl" group
# so is considered for SASL integration, and that SASL service name "bios"
# is referenced in PAM/SASL/SUDOERS setup of the OS to give certain privileges
# to processes running as this user account.

# First try a few weak passwords

curlfail_push_expect_400

test_it "change_password_weak_HasUsername"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "'"xZg4$BIOS_PASSWD@$BIOS_USER"'" }'
print_result $?

test_it "change_password_weak_HasOldPass"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "'"xW6_$BIOS_PASSWD@"'" }'
print_result $?

# Actually, this likely breaks in cryptlib before length checks by the script
test_it "change_password_weak_TooShort"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "1qW" }'
print_result $?

test_it "change_password_weak_OnlyLC"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "1x5g7h2w0f" }'
print_result $?

test_it "change_password_weak_TooSimple"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "1234abcdef" }'
print_result $?

curlfail_pop


# Now try setting a password that should succeed

ORIG_PASSWD="$BIOS_PASSWD"
#NEW_BIOS_PASSWD="nEw2`echo "$BIOS_PASSWD" | cut -c 1-3`%`echo "$BIOS_PASSWD" | cut -c 4-`"'@'
NEW_BIOS_PASSWD="xX!9`head --bytes 16 /dev/urandom | base64 | sed 's,[\+\=\/\ \t\n\r\%],_,g'`"

test_it "change_password"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "'"$NEW_BIOS_PASSWD"'" }'
print_result $?


curlfail_push_expect_400

test_it "wrong_password_completely"
_test_auth "$BIOS_USER" "not$BIOS_PASSWD" >/dev/null
print_result $?

test_it "wrong_password_previousNoLonger"
_test_auth "$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?

curlfail_pop


test_it "good_password_new"
_test_auth "$BIOS_USER" "$NEW_BIOS_PASSWD" >/dev/null
print_result $?

curlfail_push_expect_400
test_it "wrong_change_password_back_badold"
BIOS_PASSWD="$NEW_BIOS_PASSWD" api_auth_post_json /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "bad'"$NEW_BIOS_PASSWD"'", "new_passwd" : "'"$BIOS_PASSWD"'" }' >&5
print_result $?
curlfail_pop


echo "=========== Start Restore origin password at low-level========================"
test_it "restore_PASS_ORIG_GOOD_lowlevel"
sut_run "( echo "${PASS_ORIG_GOOD}"; echo "${PASS_ORIG_GOOD}"; ) | passwd ${BIOS_USER}"
print_result $?

test_it "sasl_cache_cleanup_lowlevel"
sut_run "testsaslauthd -u '$BIOS_USER' -p '${PASS_ORIG_GOOD}' -s '$SASL_SERVICE'"
print_result $?
echo "=========== End Restore origin password at low-level=========================="

curlfail_push_expect_400
test_it "wrong_password_temporaryNoLonger"
_test_auth "$BIOS_USER" "$NEW_BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "good_password_oldIsBack"
_test_auth "$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?

echo
echo "###################################################################################################"
echo "********* auth.sh *********************************** END *****************************************"
echo "###################################################################################################"
echo
