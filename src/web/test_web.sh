#!/bin/sh

PASS=0
TOTAL=0

USER="morbo"
PASSWD="iwilldestroyyou"

BASE_URL="http://127.0.0.1:8000/api/1.0"

print_result() {
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
    fi
    TOTAL="`expr $TOTAL + 1`"
    echo
}

api_get() {
    curl -v --progress-bar "$BASE_URL$1" 2>&1
}

api_post() {
    curl -v -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_post() {
    TOKEN="`api_get "/token?username=$USER&password=$PASSWD&grant_type=password" | \
            sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

# Check getting token
echo "Testing login:"
TOKEN="`api_get "/token?username=$USER&password=$PASSWD&grant_type=password" | \
        sed -n 's|.*"access_token"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
[ "$TOKEN" ]
print_result $?

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
