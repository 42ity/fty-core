###############################################################
# Check getting server system info as unprivileged user
test_it "unauth_sysinfo_get"
[ "`api_get '/admin/sysinfo' | \
    grep 'HTTP/1.1 401 Unauthorized'`" ]
print_result $?

# Check getting server system info as unprivileged user
test_it "unauth_sysinfo_get_nonjson_detect"
SYSINFO="`api_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO:" >&2
echo "$SYSINFO" >&2
case "`echo "$SYSINFO" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) RES=126 ;;
    *)	SYSINFO=""; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
# Properly here, no json result was returned so string should become empty
[ $RES = 0 -a -z "$SYSINFO" ]
print_result $?

# Check getting server system info as unprivileged user
test_it "unauth_sysinfo_get_nonjson_parse"
SYSINFO="`api_get_content '/admin/sysinfo'`"
SYSINFO_PARSED="`echo "$SYSINFO" | $CHECKOUTDIR/tools/JSON.sh -b`"
RES=$?
echo "$SYSINFO_PARSED" | egrep '^\[\]' && \
    echo "ERROR: Got empty branch name" >&2 && RES=125
echo "$SYSINFO_PARSED" | egrep '^\[\"' && \
    echo "ERROR: Got parsed markup where none was expected" >&2 && RES=124
# Properly here, no json result was returned so string should become empty
# JSON.sh generally returns an error, but may return an empty token name
# like '[] 123' - which is wrong for our usecase
# So the expected GOOD outcome is a parsing error or empty parser output.
[ $RES != 0 -o -z "$SYSINFO_PARSED" ]
print_result $?

###############################################################
# Check getting server system info (authorized only?)
test_it "sysinfo_get"
SYSINFO="`api_auth_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO:" >&2
echo "$SYSINFO" >&2
case "`echo "$SYSINFO" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) ;;
    *)	SYSINFO=""; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
[ $RES = 0 -a -n "$SYSINFO" ]
print_result $?

test_it "sysinfo_parsable"
SYSINFO_PARSED="`echo "$SYSINFO" | $CHECKOUTDIR/tools/JSON.sh -b`"
RES=$?
echo "=== SYSINFO_PARSED:" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

getval() {
    echo "$SYSINFO_PARSED" | egrep "^\[$1\]" | \
    while read _C _T; do echo "$_T"; done | \
    sed 's,^"*,,' | sed 's,"*$,,'
}

test_it "sysinfo_runs_in_container"
RES=0
SYSINFO_CONTAINER="`getval '"operating-system","container"'`" || RES=$?
echo "=== SYSINFO_CONTAINER: '$SYSINFO_CONTAINER'" >&2
[ $RES = 0 -a -n "$SYSINFO_CONTAINER" -a \
    x"$SYSINFO_CONTAINER" != x'""' ]
print_result $?

test_it "sysinfo_runs_in_virtmachine"
RES=0
SYSINFO_VIRTMACHINE="`getval '"operating-system","hypervisor"'`" || RES=$?
echo "=== SYSINFO_VIRTMACHINE: '$SYSINFO_VIRTMACHINE'" >&2
[ $RES = 0 -a -n "$SYSINFO_VIRTMACHINE" -a \
    x"$SYSINFO_VIRTMACHINE" != x'""' ]
print_result $?
