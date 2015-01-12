###############################################################
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"
# Check getting server system info as unprivileged user
# As of now, this should work, just in a limited manner
test_it "unauth_sysinfo_get"
SYSINFO="`api_get '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO ($RES):" >&2
echo "$SYSINFO" >&2
if [ $RES = 0 ]; then
    echo "$SYSINFO" | grep 'HTTP/1.1 401 Unauthorized' && \
    echo "Got HTTP-401, this is no longer expected" && RES=124
fi >&2
if [ $RES = 0 ]; then
    echo "$SYSINFO" | grep 'HTTP/1.1 4' && \
    echo "Got HTTP-4xx error" && RES=123
fi >&2
print_result $RES

# Check getting server system info as unprivileged user
test_it "unauth_sysinfo_get_nonjson_detect"
SYSINFO="`api_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO ($RES):" >&2
echo "$SYSINFO" >&2
case "`echo "$SYSINFO" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) ;;
    *)	RES=126; SYSINFO=""; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
# Properly here, some json result was returned
[ $RES = 0 -a -n "$SYSINFO" ]
print_result $?

# Check getting server system info as unprivileged user
test_it "unauth_sysinfo_get_nonjson_parse"
SYSINFO="`api_get_content '/admin/sysinfo'`"
RES=$?
JPATH='"operating-system","uname","version"'
SYSINFO_PARSED="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH" | grep unauthorized`"
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
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
test_it "sysinfo_get_auth"
SYSINFO="`api_auth_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO ($RES):" >&2
echo "$SYSINFO" >&2
case "`echo "$SYSINFO" | tr '[:space:]' ' ' | sed 's,^ *,,'`" in
    "{"*) ;;
    *)	SYSINFO=""; echo "ERROR: Received output is not JSON markup!" >&2 ;;
esac
[ $RES = 0 -a -n "$SYSINFO" ]
print_result $?

test_it "sysinfo_auth_accessgranted"
SYSINFO_PARSED="`echo "$SYSINFO" | ${JSONSH} -x 'version' | grep -v unauthorized`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_parsable"
SYSINFO_PARSED="`echo "$SYSINFO" | ${JSONSH} -l`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_runs_in_container"
RES=0
JPATH='"operating-system","container"'
SYSINFO_CONTAINER="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
echo "=== SYSINFO_CONTAINER ($RES): '$SYSINFO_CONTAINER'" >&2
[ $RES = 0 -a -n "$SYSINFO_CONTAINER" -a \
    x"$SYSINFO_CONTAINER" != x'""' ]
print_result $?

test_it "sysinfo_runs_in_virtmachine"
RES=0
JPATH='"operating-system","hypervisor"'
SYSINFO_VIRTMACHINE="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
echo "=== SYSINFO_VIRTMACHINE ($RES): '$SYSINFO_VIRTMACHINE'" >&2
[ $RES = 0 -a -n "$SYSINFO_VIRTMACHINE" -a \
    x"$SYSINFO_VIRTMACHINE" != x'""' ]
print_result $?

test_it "sysinfo_build_version_package"
RES=0
JPATH='"\$BIOS","packages",[0-9]*,"package-name"$'
SYSINFO_VERSION="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
echo "=== SYSINFO_VERSION ($RES): '$SYSINFO_VERSION'" >&2
[ $RES = 0 -a -n "$SYSINFO_VERSION" -a \
    x"$SYSINFO_VERSION" != x'""' ]
print_result $?

test_it "sysinfo_build_version_source"
RES=0
JPATH='"\$BIOS","(source-repo|build-info)"$'
SYSINFO_VERSION="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
echo "=== SYSINFO_VERSION ($RES): '$SYSINFO_VERSION'" >&2
[ $RES = 0 -a -n "$SYSINFO_VERSION" -a \
    x"$SYSINFO_VERSION" != x'""' ]
print_result $?


# Finally, test if JSON is valid
test_it "sysinfo_json_is_valid"
SYSINFO_PARSED="`echo "$SYSINFO" | ${JSONSH} -l --no-newline`"
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

