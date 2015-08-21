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

# Check getting token
_test_auth() {
    api_get "/oauth2/token?username=$1&password=$2&grant_type=password"
}

_gettoken_auth_sh() {
    _test_auth "$BIOS_USER" "$BIOS_PASSWD" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'
}

test_it "good_login"
TOKEN="`_gettoken_auth_sh`"
[ "$TOKEN" ]
print_result $?


# Check not getting token
test_it "wrong_login"
curlfail_push_expect_401
#api_get "/oauth2/token?username=not$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" >/dev/null
_test_auth "not$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?

test_it "wrong_password"
#api_get "/oauth2/token?username=$BIOS_USER&password=not$BIOS_PASSWD&grant_type=password" >/dev/null
_test_auth "$BIOS_USER" "not$BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

# NOTE: There are no RESTable tests for SASL_SERVICE, because it is a system
# pre-setting verified by ci-test-restapi.sh and test_web.sh and friends:
# the "tntnet" webserver runs as user "bios", who is member of "sasl" group
# so is considered for SASL integration, and that SASL service name "bios"
# is referenced in PAM/SASL/SUDOERS setup of the OS to give certain privileges
# to processes running as this user account.

ORIG_PASSWD="$BIOS_PASSWD"
NEW_BIOS_PASSWD="new$BIOS_PASSWD"'@'

test_it "change_password"
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "'"$NEW_BIOS_PASSWD"'" }'
print_result $?


curlfail_push_expect_401

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
BIOS_PASSWD="$NEW_BIOS_PASSWD" api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "bad'"$NEW_BIOS_PASSWD"'", "new_passwd" : "'"$BIOS_PASSWD"'" }'
print_result $?
curlfail_pop

test_it "change_password_back_goodold"
BIOS_PASSWD="$NEW_BIOS_PASSWD" api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$NEW_BIOS_PASSWD"'", "new_passwd" : "'"$ORIG_PASSWD"'" }'
print_result $?

curlfail_push_expect_401
test_it "wrong_password_temporaryNoLonger"
_test_auth "$BIOS_USER" "$NEW_BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "good_password_oldIsBack"
_test_auth "$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?


