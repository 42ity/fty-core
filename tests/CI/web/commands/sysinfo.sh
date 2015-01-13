###############################################################
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"
# Check getting server system info as unprivileged user
# As of now, this should work, just in a limited manner
test_it "sysinfo_get_auth=0_raw"
SYSINFOURAW="`api_get '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFOURAW ($RES):" >&2
echo "$SYSINFOURAW" >&2
if [ $RES = 0 ]; then
    echo "$SYSINFOURAW" | grep 'HTTP/1\.. 401' && \
    echo "Got HTTP-401 Unauthorized, this is no longer expected" && RES=124
fi >&2
if [ $RES = 0 ]; then
    echo "$SYSINFOURAW" | grep 'HTTP/1\.. 4' && \
    echo "Got HTTP-4xx error" && RES=123
fi >&2
print_result $RES

# Check getting server system info as unprivileged user
test_it "sysinfo_get_auth=0"
SYSINFOU="`api_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFOU ($RES):" >&2
echo "$SYSINFOU" >&2
[ $RES = 0 -a -n "$SYSINFOU" ]
print_result $?

test_it "sysinfo_auth=0_is_object"
# The expected JSON markup is an object, so the first non-whitespace
# character should be an opening brace
case "`echo "$SYSINFOU" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) ;;
    *)	RES=126; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
# Properly here, some json result was returned
[ $RES = 0 -a -n "$SYSINFOU" ]
print_result $?

test_it "sysinfo_auth=0_is_json"
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -N`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

# Finally, test if JSON is valid
test_it "sysinfo_auth=0_is_json_is_valid"
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -l --no-newline`"
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

# Check getting server system info as unprivileged user
test_it "sysinfo_auth=0_accessNOTgranted"
### The finer details on server OS are only available after auth
JPATH='"operating-system","uname","version"'
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -x="$JPATH" | grep unauthorized`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=0_build_version_source_accessNOTgranted"
JPATH='"\$BIOS","packages",[0-9]*,"(source-repo|build-info)"$'
SYSINFO_VERSION="`echo "$SYSINFOU" | ${JSONSH} -x="$JPATH"`"
RES=$?
# There should be no parsing error; no matches is not an error
[ -n "$SYSINFO_VERSION" ] && \
    echo "ERROR: Got parsed markup where none was expected" >&2 && \
    echo "=== SYSINFO_VERSION ($RES): '$SYSINFO_VERSION'" >&2
# The source/build details should be empty
[ $RES = 0 -a -z "$SYSINFO_VERSION" ]
print_result $?


### Obsolete checks (when unauth was to be empty):
#echo "$SYSINFO_PARSED" | egrep '^\[\]' && \
#    echo "ERROR: Got empty branch name" >&2 && RES=125
#echo "$SYSINFO_PARSED" | egrep '^\[\"' && \
#    echo "ERROR: Got parsed markup where none was expected" >&2 && RES=124
# Properly here, no json result was returned so string should become empty
# JSON.sh generally returns an error, but may return an empty token name
# like '[] 123' - which is wrong for our usecase
# So the expected GOOD outcome is a parsing error or empty parser output.
#[ $RES != 0 -o -z "$SYSINFO_PARSED" ]
#print_result $?

###############################################################
# Check getting server system info (authorized only?)
test_it "sysinfo_get_auth=2"
SYSINFOA="`api_auth_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFOA ($RES):" >&2
echo "$SYSINFOA" >&2
[ $RES = 0 -a -n "$SYSINFOA" ]
print_result $?

test_it "sysinfo_auth=2_is_object"
# Like above, expect an opening brace as the first non-whitespace character
case "`echo "$SYSINFOA" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) ;;
    *)	RES=126; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
[ $RES = 0 -a -n "$SYSINFOA" ]
print_result $?

test_it "sysinfo_auth=2_is_json"
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -N`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

# Finally, test if JSON is valid
test_it "sysinfo_auth=2_is_json_is_valid"
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -l --no-newline`"
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=2_accessgranted"
JPATH='"operating-system","uname","version"'
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -x "$JPATH" | grep -v unauthorized`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=2_runs_in_container"
JPATH='"operating-system","container"'
SYSINFO_CONTAINER="`echo "$SYSINFOA" | ${JSONSH} -x="$JPATH"`"
RES=$?
echo "=== SYSINFO_CONTAINER ($RES): '$SYSINFO_CONTAINER'" >&2
[ $RES = 0 -a -n "$SYSINFO_CONTAINER" -a \
    x"$SYSINFO_CONTAINER" != x'""' ]
print_result $?

test_it "sysinfo_auth=2_runs_in_virtmachine"
JPATH='"operating-system","hypervisor"'
SYSINFO_VIRTMACHINE="`echo "$SYSINFOA" | ${JSONSH} -x="$JPATH"`"
RES=$?
echo "=== SYSINFO_VIRTMACHINE ($RES): '$SYSINFO_VIRTMACHINE'" >&2
[ $RES = 0 -a -n "$SYSINFO_VIRTMACHINE" -a \
    x"$SYSINFO_VIRTMACHINE" != x'""' ]
print_result $?

test_it "sysinfo_auth=2_build_version_package"
JPATH='"\$BIOS","packages",[0-9]*,"package-(name|version)"$'
SYSINFO_VERSION="`echo "$SYSINFOA" | ${JSONSH} -x="$JPATH"`"
RES=$?
{ echo "=== SYSINFO_VERSION ($RES):"; echo "$SYSINFO_VERSION"; } >&2
[ $RES = 0 -a -n "$SYSINFO_VERSION" -a \
    x"$SYSINFO_VERSION" != x'""' ]
print_result $?

test_it "sysinfo_auth=2_build_version_source"
JPATH='"\$BIOS","packages",[0-9]*,"(source-repo|build-info)"$'
SYSINFO_VERSION="`echo "$SYSINFOA" | ${JSONSH} -x="$JPATH"`"
RES=$?
{ echo "=== SYSINFO_VERSION ($RES):"; echo "$SYSINFO_VERSION"; } >&2
[ $RES = 0 -a -n "$SYSINFO_VERSION" -a \
    x"$SYSINFO_VERSION" != x'""' ]
print_result $?

