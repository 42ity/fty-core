[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"

SED_FILTER_TIME='s|.*\"time\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'
ZEROEPOCH='{ "time":"1970-01-01T00:00:00Z" }'

# Check getting time
test_it "time_get"
TIME="`api_get '/admin/time' | sed -n "$SED_FILTER_TIME"`"
TIME_S_GET="`date --utc -d"$TIME" +%s 2> /dev/null`"
TIME_S_NOW="`date --utc +%s`"
[ "`expr "$TIME_S_NOW" - "$TIME_S_GET"`" -lt 10 ]
print_result $?

# Check setting time as unprivileged user
test_it "unauth_time_set"
[ "`api_post '/admin/time' "$ZEROEPOCH" | \
    grep 'HTTP/1.1 401 Unauthorized'`" ]
print_result $?

# Check setting time as privileged user
# NOTE: Doesn't work within lxc
SYSINFO="`api_get_content '/admin/sysinfo'`"
RES=$?
JPATH='"operating-system","container"'
SYSINFO_CONTAINER="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
if [ $RES = 0 -a -n "$SYSINFO_CONTAINER" -a \
     x"$SYSINFO_CONTAINER" != x'""' ] && \
    echo "$SYSINFO_CONTAINER" | egrep 'lxc' >/dev/null ; \
then
    echo "SKIPPED test auth_time_set because server runs in a container" >&2
    echo "=== SYSINFO_CONTAINER ($RES): '$SYSINFO_CONTAINER'" >&2
else
    test_it "auth_time_set"
    TIME_NOW="`date --utc +%FT%TZ`"
    TIME="`api_auth_post '/admin/time' "$ZEROEPOCH" | \
	sed -n "$SED_FILTER_TIME"`"
    api_auth_post '/admin/time' '{ "time":"'"$TIME_NOW"'" }' > /dev/null
    TIME_S="`date -d"$TIME" +%s 2> /dev/null`"
    [ "$TIME_S" ] && [ "$TIME_S" -lt 10 ]
    print_result $?
fi

# Check setting nonsense
test_it "wrong_time"
[ "`api_auth_post '/admin/time' 'stardate 48960.9' | \
    grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

