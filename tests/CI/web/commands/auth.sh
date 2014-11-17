#!/bin/sh

# Check getting token
test_it "login"
TOKEN="`api_get "/oauth2/token?username=$BIOS_USER&password=$BIOS_PASSWD&grant_type=password" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
[ "$TOKEN" ]
print_result $?

# Check not getting token
test_it "wrong_login"
[ "`api_get "/oauth2/token?username=$BIOS_USER&password=not$BIOS_PASSWD&grant_type=password" | \
    grep "HTTP/1.1 401 Unauthorized"`" ]
print_result $?

