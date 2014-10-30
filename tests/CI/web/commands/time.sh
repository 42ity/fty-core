# Check getting time
test_it "time_get"
TIME="`api_get "/admin/time" | \
       sed -n 's|.*"time"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
TIME_S_GET="`date -d"$TIME" +%s 2> /dev/null`"
TIME_S_NOW="`date +%s`"
[ "`expr "$TIME_S_NOW" - "$TIME_S_GET"`" -lt 10 ]
print_result $?

# Check setting time as unprivileged user
test_it "unauth_time_set"
[ "`api_post "/admin/time" "1970-01-01T00:00:00Z" | \
    grep "HTTP/1.1 401 Unauthorized"`" ]
print_result $?

# Check setting time as privileged user
test_it "auth_time_set"
TIME_NOW="`date --utc +%FT%TZ`"
TIME="`api_auth_post "/admin/time" "1970-01-01T00:00:00Z" | \
       sed -n 's|.*"time"[[:blank:]]*:[[:blank:]]*"\([^"]*\)".*|\1|p'`"
api_auth_post "/admin/time" "$TIME_NOW" > /dev/null
TIME_S="`date -d"$TIME" +%s 2> /dev/null`"
[ "$TIME_S" -lt 10 ]
print_result $?

# Check setting nonsense
test_it "wrong_time"
[ "`api_auth_post "/admin/time" "stardate 48960.9" | \
    grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

