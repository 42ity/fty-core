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
test_it "sysinfo_auth=0_accessNOTgranted_OSversion"
### The finer details on server OS are only available after auth
JPATH='"operating-system","uname","version"'
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -x="$JPATH" | grep unauthorized`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=0_accessNOTgranted_process_status"
### The finer details on server OS are only available after auth
JPATH='"\$BIOS","main-process-status"$'
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -l -x="$JPATH" | grep unauthorized`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=0_accessNOTgranted_process_details"
### The finer details on server OS are only available after auth
JPATH='"\$BIOS","main-process-details"'
SYSINFO_PARSED="`echo "$SYSINFOU" | ${JSONSH} -l -x="$JPATH"`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
# The process details should be empty
[ $RES = 0 -a -z "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=0_accessNOTgranted_build_source"
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

test_it "sysinfo_auth=2_process_status"
### The finer details on server OS are only available after auth
JPATH='"\$BIOS","main-process-status"$'
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -l -x="$JPATH" | egrep 'unknown|available'`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=2_process_details"
### The finer details on server OS are only available after auth
### At least the "tntnet" process which is on the list of interesting
### ones should be visible
JPATH='"\$BIOS","main-process-details"'
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -l -x="$JPATH"`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
# The process details should not be empty
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

test_it "sysinfo_auth=2_process_details_tntnet"
JPATH='"\$BIOS","main-process-details",[0-9]*$'
SYSINFO_PARSED="`echo "$SYSINFOA" | ${JSONSH} -x="$JPATH" | grep tntnet`"
RES=$?
echo "=== SYSINFO_PARSED ($RES):" >&2
echo "$SYSINFO_PARSED" >&2
# The process details should not be empty
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
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

###############################################################
# Compare different request methods

test_it "sysinfo_get_wToken_auth=2"
SYSINFOA_WT_G="`api_auth_get_content_wToken '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFOA_WT_G ($RES):" >&2
echo "$SYSINFOA_WT_G" >&2
[ $RES = 0 -a -n "$SYSINFOA_WT_G" ]
print_result $?

test_it "sysinfo_get_wToken_auth=2_is_json_is_valid"
SYSINFO_PARSED="`echo "$SYSINFOA_WT_G" | ${JSONSH} -l --no-newline`"
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?


test_it "sysinfo_post_wToken_auth=2"
SYSINFOA_WT_P="`api_auth_post_content_wToken '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFOA_WT_P ($RES):" >&2
echo "$SYSINFOA_WT_P" >&2
[ $RES = 0 -a -n "$SYSINFOA_WT_P" ]
print_result $?

test_it "sysinfo_post_wToken_auth=2_is_json_is_valid"
SYSINFO_PARSED="`echo "$SYSINFOA_WT_P" | ${JSONSH} -l --no-newline`"
RES=$?
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?


if [ -n "$CMPJSON_SH" -a -x "$CMPJSON_SH" ]; then
    # Note: variable output like CPU and MEM info should be ignored in comparison
    test_it "sysinfo_get_wToken_auth=2_sameAsHeaderAuth_cmpjson"
    OUT="`"$CMPJSON_SH" -s "$SYSINFOA_WT_G" "$SYSINFOA"`"
    RES=$?
    if [ -n "$OUT" ]; then
        OUT="`echo "$OUT" | egrep -v '^(\-\-\-| |@|\+\+\+|\<|\>)' | egrep -v 'process-percent-|process-mem-|process-time-'`"
        [ -z "$OUT" ] && RES=0 || echo "$OUT" >&2
    fi
    print_result $RES

    test_it "sysinfo_post_wToken_auth=2_sameAsHeaderAuth_cmpjson"
    OUT="`"$CMPJSON_SH" -s "$SYSINFOA_WT_P" "$SYSINFOA"`"
    RES=$?
    if [ -n "$OUT" ]; then
        OUT="`echo "$OUT" | egrep -v '^(\-\-\-| |@|\+\+\+|\<|\>)' | egrep -v 'process-percent-|process-mem-|process-time-'`"
        [ -z "$OUT" ] && RES=0 || echo "$OUT" >&2
    fi
    print_result $RES
#else
#    test_it "sysinfo_get_wToken_auth=2_sameAsHeaderAuth_naive"
#    [ x"$SYSINFOA_WT_G" = x"$SYSINFOA" ]
#    print_result $?
#
#    test_it "sysinfo_post_wToken_auth=2_sameAsHeaderAuth_naive"
#    [ x"$SYSINFOA_WT_P" = x"$SYSINFOA" ]
#    print_result $?
fi

