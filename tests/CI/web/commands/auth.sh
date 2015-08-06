# Check getting token
_gettoken_auth_sh() {
    api_get "/oauth2/token?username=$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'
}

test_it "good_login"
TOKEN="`_gettoken_auth_sh`"
[ "$TOKEN" ]
print_result $?


# Check not getting token
test_it "wrong_login"
curlfail_push_expect_401
api_get "/oauth2/token?username=not$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" >/dev/null
print_result $?

test_it "wrong_password"
api_get "/oauth2/token?username=$BIOS_USER&password=not$BIOS_PASSWD&grant_type=password" >/dev/null
print_result $?
curlfail_pop

# NOTE: There are no RESTable tests for SASL_SERVICE, because it is a system
# pre-setting verified by ci-test-restapi.sh and test_web.sh and friends:
# the "tntnet" webserver runs as user "bios", who is member of "sasl" group
# so is considered for SASL integration, and that SASL service name "bios"
# is referenced in PAM/SASL/SUDOERS setup of the OS to give certain privileges
# to processes running as this user account.
