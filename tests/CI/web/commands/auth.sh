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
curlfail_push_expect_401
#api_get "/oauth2/token?username=not$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" >/dev/null
_test_auth "not$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?

test_it "wrong_password"
#api_get "/oauth2/token?username=$BIOS_USER&password=not$BIOS_PASSWD&grant_type=password" >/dev/null
_test_auth "$BIOS_USER" "not$BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "token_revoke"
TOKEN="`_gettoken_auth_sh`"
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "foobar" \
        -v --progress-bar "$BASE_URL/admin/license" 3> /dev/null 2> /dev/null >/dev/null
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "token=$TOKEN"  \
        -v --progress-bar "$BASE_URL/oauth2/revoke" 3> /dev/null 2> /dev/null | $JSONSH -N >&5
curlfail_push_expect_401
CURL --insecure --header "Authorization: Bearer $TOKEN" -d "foobar" \
        -v --progress-bar "$BASE_URL/admin/license" 3> /dev/null 2> /dev/null | $JSONSH -N >&5
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
api_auth_post /admin/passwd '{"user" : "'"$BIOS_USER"'", "old_passwd" : "'"$BIOS_PASSWD"'", "new_passwd" : "'"$BIOS_PASSWD@$BIOS_USER"'" }'
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
NEW_BIOS_PASSWD="nEw2`echo "$BIOS_PASSWD" | cut -c 1-3`%`echo "$BIOS_PASSWD" | cut -c 4-`"'@'

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
PASS_RECOVERED=$?
if [ "$PASS_RECOVERED" = 0 ]; then
    print_result 0
else
    if [ -n "${PASS_ORIG_GOOD}" ] ; then
        RES_TESTPASS=-1
        if [ -x "$TESTPASS" ] ; then
            ( echo "$BIOS_USER"; echo "${PASS_ORIG_GOOD}"; echo "$NEW_BIOS_PASSWD" ) | "$TESTPASS"
            RES_TESTPASS=$?
        fi
        case "$RES_TESTPASS" in
            1[1-5])
                echo "WARNING: The REST API failed to restore the original password because it was weak; not considering this as an error!"
                print_result 0 ;;
            0|*) print_result $PASS_RECOVERED ;; # REST API failed not due to weakness
        esac
        unset RES_TESTPASS

        echo "WARNING: Previously failed to restore the (expected) original password, so doing it low-level"
        test_it "restore_PASS_ORIG_GOOD_lowlevel"
        sut_run "( echo "${PASS_ORIG_GOOD}"; echo "${PASS_ORIG_GOOD}"; ) | passwd ${BIOS_USER}"
        print_result $?

        test_it "sasl_cache_cleanup_lowlevel"
        sut_run "testsaslauthd -u '$BIOS_USER' -p '${PASS_ORIG_GOOD}' -s '$SASL_SERVICE'"
        print_result $?
    else
        print_result $PASS_RECOVERED
    fi
fi

curlfail_push_expect_401
test_it "wrong_password_temporaryNoLonger"
_test_auth "$BIOS_USER" "$NEW_BIOS_PASSWD" >/dev/null
print_result $?
curlfail_pop

test_it "good_password_oldIsBack"
_test_auth "$BIOS_USER" "$BIOS_PASSWD" >/dev/null
print_result $?

