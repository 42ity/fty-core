#!/bin/sh

PASS=0
TOTAL=0

USER="morbo"
PASSWD="iwilldestroyyou"

BASE_URL="http://127.0.0.1:8000/api/v1"

print_result() {
    _ret=0
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
        _ret=1
    fi
    TOTAL="`expr $TOTAL + 1`"
    echo
    return $_ret
}

api_get() {
    curl -v --progress-bar "$BASE_URL$1" 2>&1
}

api_post() {
    curl -v -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_post() {
    TOKEN="`api_get "/oauth2/token?username=$USER&password=$PASSWD&grant_type=password" | \
            sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

# fixture ini
if [ ! /usr/bin/systemctl is-active -q saslauthd.service ]; then
    echo "saslauthd does not run, please entrer a root password to run it"
    /usr/bin/sudo /usr/bin/systemctl start saslauthd.service || exit $?
fi

# Check getting token
echo "Testing login:"
TOKEN="`api_get "/token?username=$USER&password=$PASSWD&grant_type=password" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
[ "$TOKEN" ]
print_result $?
if [ $? -ne 0 ]; then
    echo "FATAL: access token test must not fail, otherwise it does not makes a sense to continue"
    exit 1
fi

# Check not getting token
echo "Testing wrong login:"
[ "`api_get "/token?username=$USER&password=not$PASSWD&grant_type=password" | \
    grep "HTTP/1.1 401 Unauthorized"`" ]
print_result $?

# Check getting time
echo "Testing time get:"
TIME="`api_get "/admin/time" | \
       sed -n 's|.*"time"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
TIME_S_GET="`date -d"$TIME" +%s 2> /dev/null`"
TIME_S_NOW="`date +%s`"
[ "`expr "$TIME_S_NOW" - "$TIME_S_GET"`" -lt 10 ]
print_result $?

# Check setting time as unprivileged user
echo "Testing unauthorized time set:"
[ "`api_post "/admin/time" "1970-01-01T00:00:00Z" | \
    grep "HTTP/1.1 401 Unauthorized"`" ]
print_result $?

# Check setting time as privileged user
echo "Testing authorized time set:"
TIME_NOW="`date --utc +%FT%TZ`"
TIME="`api_auth_post "/admin/time" "1970-01-01T00:00:00Z" | \
       sed -n 's|.*"time"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
api_auth_post "/admin/time" "$TIME_NOW" > /dev/null
TIME_S="`date -d"$TIME" +%s 2> /dev/null`"
[ "$TIME_S" -lt 10 ]
print_result $?

# Check setting nonsense
echo "Testing wrong time format set:"
[ "`api_auth_post "/admin/time" "stardate 48960.9" | \
    grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

echo "Testing completed, $PASS/$TOTAL tests passed"
