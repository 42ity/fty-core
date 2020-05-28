#
# Copyright (C) 2014 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    weblib.sh
#  \brief   library of functions useful for REST API testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \details This is library of functions usefull for REST API testing,
#           which can be sourced to interactive shell
#           You can 'export CITEST_QUICKFAIL=yes' to abort on first failure

# Should the test suite abort if "curl" errors out?
[ -z "${WEBLIB_CURLFAIL-}" ] || [ x"${WEBLIB_CURLFAIL-}" = x- ] && WEBLIB_CURLFAIL=yes

# Should the test suite abort if "curl" sees HTTP error codes?
# This can be overridden on a per-call basis for those api_get's
# where we do expect errors as the proper response.
#       fatal   Fail if errors are detected in stderr contents
#       expect  Fail if stderr is not empty but result is OK
#       ignore  Don't care, and don't test
#       warn    Anything else (*) gives a warning if error was matched and goes on
[ -z "${WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT-}" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="warn"
[ -z "${WEBLIB_CURLFAIL_HTTPERRORS-}" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS="$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT"

# If set to "protected", then _api_get_token will automatically expect
# success headers and fail the test otherwise; set it to anything else
# only to test failures in the token-work routines
[ -z "${WEBLIB_CURLFAIL_GETTOKEN-}" ] && \
    WEBLIB_CURLFAIL_GETTOKEN="protected"

# Flag (yes|no|onerror) to print CURL trace on any HTTP error mismatch
# This prints STDERR and STDOUT for the request upon regex hits
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEBUG="onerror"

# Regexp of HTTP header contents that is considered an error;
# one may test for specific codes with custom regexps for example
# NOTE: Must be single-line for push/pop to work well.
[ -z "${WEBLIB_HTTPERRORS_REGEX_DEFAULT-}" ] && \
    WEBLIB_HTTPERRORS_REGEX_DEFAULT='HTTP/[^ ]+ [45]'
[ -z "${WEBLIB_HTTPERRORS_REGEX-}" ] && \
    WEBLIB_HTTPERRORS_REGEX="$WEBLIB_HTTPERRORS_REGEX_DEFAULT"

# Print out the CURL stdout and stderr (via FD#3)?
[ -z "${WEBLIB_TRACE_CURL-}" ] && WEBLIB_TRACE_CURL=no

[ -n "${SCRIPTDIR-}" ] && [ -d "$SCRIPTDIR" ] || \
        SCRIPTDIR="$(cd "`dirname "$0"`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname "$0"`" || \
        SCRIPTDIR="`dirname "$0"`"

. "${SCRIPTDIR}"/scriptlib.sh || exit $?

if [ -z "${CHECKOUTDIR-}" ]; then
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
            CHECKOUTDIR="$(realpath $SCRIPTDIR/../..)" || \
            CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
            CHECKOUTDIR="$( cd "$SCRIPTDIR"/../.. && pwd )" || \
            CHECKOUTDIR="" ;;
        */tools|tools)
            CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tools$||' )" || \
            CHECKOUTDIR="$( cd "$SCRIPTDIR"/.. && pwd )" || \
            CHECKOUTDIR="" ;;
    esac
fi

# Detect needed real curl or its wget-based emulator which suffices in
# a limited way (e.g. no parsing of output headers in "< Line" format)
# Not good for CI tests, but is sufficient for command-line automation.

# SKIP_SANITY=(yes|no|onlyerrors)
#   yes = skip sanity tests in certain ultimate request/test scripts
#   no  = do all tests
#   onlyerrors = do only tests expected to fail (not for curlbbwget.sh)
[ x"${SKIP_SANITY-}" = x- ] && SKIP_SANITY=""
[ -z "${SKIP_SANITY-}" ] && SKIP_SANITY=no

( which curl >/dev/null 2>&1 ) || {
    [ -x "$SCRIPTDIR/curlbbwget.sh" ] && \
    SKIP_SANITY=onlyerrors && \
    curl() {
        "$SCRIPTDIR/curlbbwget.sh" "$@"
    } || {
        echo "FATAL-WEBLIB: neither curl program nor curlbbwget.sh emulator" \
            "were found and one is required for requests!" >&2
        exit 127
    }
}

# Support delivery of weblib.sh into distro so that paths are different
# and testlib may be not available (avoid kill $_PID_TESTER in traps below)
[ x"${NEED_TESTLIB-}" != xno ] && \
if [ -n "$CHECKOUTDIR" ] && [ -d "$CHECKOUTDIR/tests/CI" ]; then
        . "$CHECKOUTDIR/tests/CI"/testlib.sh || exit $?
        . "$CHECKOUTDIR/tests/CI"/testlib-db.sh || exit $?
else
        . "$SCRIPTDIR"/testlib.sh || exit $?
        . "$SCRIPTDIR"/testlib-db.sh || exit $?
fi

### Should the test suite break upon first failed test?
### Legacy weblib value (may be set by caller/includer)
### overrides the common testlib variable
[ x"${WEBLIB_QUICKFAIL-}" = xno -o x"${WEBLIB_QUICKFAIL-}" = xyes ] && \
        echo "CI-WEBLIB-INFO: Overriding CITEST_QUICKFAIL with WEBLIB_QUICKFAIL='$WEBLIB_QUICKFAIL'" && \
        CITEST_QUICKFAIL="$WEBLIB_QUICKFAIL"

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh" "$SCRIPTDIR/../../tools/JSON.sh" "/usr/share/fty/scripts/JSON.sh" "/usr/share/bios/scripts/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

# Check if we already have jsonsh_cli() defined... afterwards we should have it :)
[ -z "${JSONSH_CLI_DEFINED-}" ] && JSONSH_CLI_DEFINED="no"
[ x"$JSONSH_CLI_DEFINED" = xyes ] || \
if [ -n "$JSONSH" ] && [ -x "$JSONSH" ] ; then
    if [ x"$(jsonsh_cli -QQ '"' 2>/dev/null)" = 'x\"' ]
    then : ; else
        if [ -n "${BASH-}" ] && . "$JSONSH" ; then
            logmsg_debug "Will use sourced JSON.sh from '$JSONSH'" >&2
        else
            logmsg_debug "Will fork to use JSON.sh from '$JSONSH'" >&2
            jsonsh_cli() { "$JSONSH" "$@"; }
        fi
    fi
    JSONSH_CLI_DEFINED=yes
    export JSONSH
else
    JSONSH=""
    JSONSH_CLI_DEFINED=no
    export JSONSH
fi

_TOKEN_=""

### This is what we will sig-kill if needed
_PID_TESTER_WEBLIB=$$
RES_CURL=0
trap_break_weblib() {
    ### This SIGUSR1 handler is reserved for CURL failures
    set +e
    [ "$RES_CURL" = 0 ] && \
        echo "CI-WEBLIB-ERROR-WEB: curl program failed, aborting test suite" >&2 && RES_CURL=126 || \
        echo "CI-WEBLIB-ERROR-WEB: curl program failed ($RES_CURL), aborting test suite" >&2
    TESTLIB_FORCEABORT=yes
    if [ -n "$_PID_TESTER" ] ; then
        echo "CI-WEBLIB-ERROR-WEB: Killing the test suite: kill -SIGUSR2 $_PID_TESTER $$" >&2
        kill -n 12 $_PID_TESTER $$
#        kill -SIGUSR2 $_PID_TESTER $$
    else
        echo "CI-WEBLIB-ERROR-WEB: exiting ($RES_CURL)" >&2
        exit $RES_CURL
    fi

    return $RES_CURL
}
TRAP_SIGNALS=USR1 settraps "trap_break_weblib"

STACKED_HTTPERRORS_ACTIONS=""
STACKED_HTTPERRORS_REGEX=""
STACKED_HTTPERRORS_COUNT=0

curlfail_push() {
    ### saves current "$WEBLIB_CURLFAIL_HTTPERRORS" actions to the stack,
    ### and sets it to the value of "$1" (empty means default)
    ### also saves "$WEBLIB_HTTPERRORS_REGEX" to its stack and changes to
    ### "$2" (if provided), or retains it as is otherwise

    [ x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xyes -o x"$WEBLIB_TRACE_CURL" = xyes ] && \
    echo "CI-WEBLIB-TRACE-CURL: curlfail_push(): setting" \
        "WEBLIB_CURLFAIL_HTTPERRORS='$1' instead of '$WEBLIB_CURLFAIL_HTTPERRORS'" \
        "and WEBLIB_HTTPERRORS_REGEX='$2' instead of '$WEBLIB_HTTPERRORS_REGEX'" \
        "on top of $STACKED_HTTPERRORS_COUNT items already in stack"

    [ -z "${STACKED_HTTPERRORS_ACTIONS-}" ] && \
        STACKED_HTTPERRORS_ACTIONS="$WEBLIB_CURLFAIL_HTTPERRORS" || \
        STACKED_HTTPERRORS_ACTIONS="$WEBLIB_CURLFAIL_HTTPERRORS
$STACKED_HTTPERRORS_ACTIONS"

    [ -z "${STACKED_HTTPERRORS_REGEX-}" ] && \
        STACKED_HTTPERRORS_REGEX="$WEBLIB_HTTPERRORS_REGEX" || \
        STACKED_HTTPERRORS_REGEX="$WEBLIB_HTTPERRORS_REGEX
$STACKED_HTTPERRORS_REGEX"

    STACKED_HTTPERRORS_COUNT="`expr $STACKED_HTTPERRORS_COUNT + 1`" || \
    STACKED_HTTPERRORS_COUNT="`echo "$STACKED_HTTPERRORS_ACTIONS" | wc -l`"

    [ -n "$1" ] && \
        WEBLIB_CURLFAIL_HTTPERRORS="$1" || \
        WEBLIB_CURLFAIL_HTTPERRORS="$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT"
    [ -n "$2" ] && WEBLIB_HTTPERRORS_REGEX="$2"
    return 0
}

curlfail_pop() {
    ### retrieves most recent values of "$WEBLIB_CURLFAIL_HTTPERRORS" and
    ### pops "$WEBLIB_HTTPERRORS_REGEX" from the stack (empty val = default);
    ### if the stack is empty, resets the popped values to their defaults

    if [ -z "${STACKED_HTTPERRORS_ACTIONS-}" ]; then
        WEBLIB_CURLFAIL_HTTPERRORS=""
    else
        WEBLIB_CURLFAIL_HTTPERRORS="`echo "$STACKED_HTTPERRORS_ACTIONS" | head -1`" || \
        WEBLIB_CURLFAIL_HTTPERRORS=""
        STACKED_HTTPERRORS_ACTIONS="`echo "$STACKED_HTTPERRORS_ACTIONS" | tail -n +2`"
    fi
    [ -z "${WEBLIB_CURLFAIL_HTTPERRORS-}" ] && \
        WEBLIB_CURLFAIL_HTTPERRORS="$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT"

    if [ -z "${STACKED_HTTPERRORS_REGEX-}" ]; then
        WEBLIB_HTTPERRORS_REGEX=""
    else
        WEBLIB_HTTPERRORS_REGEX="`echo "$STACKED_HTTPERRORS_REGEX" | head -1`" || \
        WEBLIB_HTTPERRORS_REGEX=""
        STACKED_HTTPERRORS_REGEX="`echo "$STACKED_HTTPERRORS_REGEX" | tail -n +2`"
    fi
    [ -z "${WEBLIB_HTTPERRORS_REGEX-}" ] && \
        WEBLIB_HTTPERRORS_REGEX="$WEBLIB_HTTPERRORS_REGEX_DEFAULT"

    if [ "${STACKED_HTTPERRORS_COUNT-}" -le 1 ]; then
        STACKED_HTTPERRORS_COUNT=0
        STACKED_HTTPERRORS_REGEX=""
        STACKED_HTTPERRORS_ACTIONS=""
    else
        STACKED_HTTPERRORS_COUNT="`expr $STACKED_HTTPERRORS_COUNT - 1`" || \
        STACKED_HTTPERRORS_COUNT="`echo "$STACKED_HTTPERRORS_ACTIONS" | wc -l`"
    fi

    [ x"${WEBLIB_CURLFAIL_HTTPERRORS_DEBUG-}" = xyes -o x"${WEBLIB_TRACE_CURL-}" = xyes ] && \
    echo "CI-WEBLIB-TRACE-CURL: curlfail_pop(): returned to " \
        "WEBLIB_CURLFAIL_HTTPERRORS='$WEBLIB_CURLFAIL_HTTPERRORS'" \
        "and WEBLIB_HTTPERRORS_REGEX='$WEBLIB_HTTPERRORS_REGEX';" \
        "$STACKED_HTTPERRORS_COUNT items remain in stack"

    return 0
}

curlfail_push_default() {
    ### Preconfigured push that resets current values to defaults
    curlfail_push "" ""
}

curlfail_push_expect_noerrors() {
    ### Preconfigured push that resets current values to
    ### expect any HTTP ok code (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ [123][0-9][0-9]'
}

curlfail_push_expect_200() {
    ### Preconfigured push that resets current values to specifically
    ### expect HTTP 200 OK (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 200'
}

curlfail_push_expect_4xx5xx() {
    ### Preconfigured push that resets current values to
    ### expect any HTTP error (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ [45][0-9][0-9]'
}

curlfail_push_expect_400() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-400 bad request (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 400'
}

curlfail_push_expect_401() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-401 access denied (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 401'
}

curlfail_push_expect_403() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-403 Forbidden (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 403'
}

curlfail_push_expect_404() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-404 not found (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 404'
}
curlfail_push_expect_405() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-405 Request method not expected (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 405'
}

curlfail_push_expect_409() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-409 conflict (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 409'
}

curlfail_push_expect_413() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-413 Request Entity Too Large (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 413'
}

curlfail_push_expect_500() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-500 server error (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 500'
}

CURL() {
    _TMP_CURL="/tmp/.bios_weblib_curl.$$"
    /bin/rm -f "$_TMP_CURL" >/dev/null 2>&1
    if  touch "$_TMP_CURL" >/dev/null 2>&1 && \
        chmod 600 "$_TMP_CURL" >/dev/null 2>&1 \
    ; then
        ### Ensure "-v" to print out headers; it does not hurt if the caller
        ### also ask for that mode (via "$@")
        OUT_CURL="`curl -v --stderr "$_TMP_CURL" "$@"`" 2>&3
        RES_CURL=$?
        ERR_CURL="`cat "$_TMP_CURL"`" 2>&3
        /bin/rm -f "$_TMP_CURL" >/dev/null 2>&1
        echo "$ERR_CURL" >&2
        echo "$OUT_CURL"
    else
#        OUT_CURL=""
        ERR_CURL=""
        OUT_CURL="`curl "$@"`"
#        curl "$@"
        RES_CURL=$?
        echo "$OUT_CURL"
    fi

    _PRINT_CURL_TRACE=no
    if [ $RES_CURL != 0 ]; then
        if [ -n "$ERR_CURL" ] && ( echo "$ERR_CURL"; echo "" ) | grep wget >/dev/null ; then
            # Busybox wget returns non-zero for non-success HTTP result codes
            # and returns the header part with code in its stderr message
            RES_CURL=0
        else
            ### Based on caller redirections, this output may never be seen
            echo "CI-WEBLIB-ERROR-CURL: 'curl $@' program failed ($RES_CURL)," \
                "perhaps the web server is not available or has crashed?" >&3
            _PRINT_CURL_TRACE=yes
        fi
    fi

    ERR_MATCH=""
    if [ -n "$ERR_CURL" -a x"$WEBLIB_CURLFAIL_HTTPERRORS" != xignore ]; then
        ERR_MATCH="`( echo "$ERR_CURL"; echo "" ) | tr '\r' '\n' | egrep '^( *< |.*#+ 100.0%< |wget: server returned error: )'"$WEBLIB_HTTPERRORS_REGEX"`" 2>&3 || true
        if [ -n "$ERR_MATCH" ]; then
            if [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xexpect ]; then
                [ x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xyes ] && \
                echo "CI-WEBLIB-INFO-CURL: Last response headers matched" \
                    "'$WEBLIB_HTTPERRORS_REGEX' for an HTTP result code," \
                    "as expected: '$ERR_MATCH'" >&3
            else
                echo "CI-WEBLIB-WARN-CURL: Last response headers matched" \
                    "'$WEBLIB_HTTPERRORS_REGEX' for an HTTP result code," \
                    "which is an error: '$ERR_MATCH'" >&3
                # legacy value for WEBLIB_CURLFAIL_HTTPERRORS==debug
                [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xdebug -o \
                  x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xyes ] && \
                    _PRINT_CURL_TRACE=yes
                if [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xfatal ]; then
                    echo "CI-WEBLIB-ERROR-CURL: WEBLIB_CURLFAIL_HTTPERRORS=fatal" \
                        "is set, so marking the request as failed now" >&3
                    [ x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xonerror ] && \
                        _PRINT_CURL_TRACE=yes
                    RES_CURL=123
                else
                    echo "CI-WEBLIB-ERROR-CURL: WEBLIB_CURLFAIL_HTTPERRORS='$WEBLIB_CURLFAIL_HTTPERRORS'" \
                        "is NOT 'fatal', so assuming the request as successful for now" >&3
                fi
            fi
        else
            ### No match
            if [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xexpect ]; then
                echo "CI-WEBLIB-ERROR-CURL: Last response headers did not match" \
                    "an expected HTTP result pattern '$WEBLIB_HTTPERRORS_REGEX';" \
                    "while WEBLIB_CURLFAIL_HTTPERRORS=expect is set," \
                    "so marking the request as failed now" >&3
                [ x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xyes -o \
                  x"$WEBLIB_CURLFAIL_HTTPERRORS_DEBUG" = xonerror ] && \
                        _PRINT_CURL_TRACE=yes
                _PRINT_CURL_TRACE=yes
                RES_CURL=124
            fi
        fi
    fi

    if [ x"$WEBLIB_TRACE_CURL" = xyes -o x"$_PRINT_CURL_TRACE" = xyes ]; then
        echo "CI-WEBLIB-TRACE-CURL: curl $@: RES=$RES_CURL"
        echo "=== WEBLIB_CURLFAIL_HTTPERRORS:   '$WEBLIB_CURLFAIL_HTTPERRORS'"
        echo "=== WEBLIB_HTTPERRORS_REGEX:      '$WEBLIB_HTTPERRORS_REGEX'"
        echo "=== ERR_MATCH:    '$ERR_MATCH'"
        echo "=== ERR vvv"
        echo "$ERR_CURL"
        echo "/// ERR ^^^"
        echo "=== OUT vvv"
        echo "$OUT_CURL"
        echo "/// OUT ^^^"
    fi >&3

    if [ $RES_CURL != 0 -a x"$WEBLIB_CURLFAIL" = xyes ]; then
        echo "CI-WEBLIB-ERROR-CURL: Killing the test: kill -SIGUSR1 $_PID_TESTER_WEBLIB $$" >&3
        kill -n 10 $_PID_TESTER_WEBLIB $$ >&3 2>&3 #>/dev/null 2>&1
        # exit $RES_CURL
    fi

    return $RES_CURL
}

_api_get_token() {
    _RES_=0
    if [ -z "$_TOKEN_" ]; then
        AUTH_URL="/oauth2/token"
#        AUTH_DATA="username=${BIOS_USER}&password=${BIOS_PASSWD}&grant_type=password"
        AUTH_DATA="{\"username\":\"`jsonsh_cli -QQ "${BIOS_USER}"`\",\"password\":\"`jsonsh_cli -QQ "${BIOS_PASSWD}"`\",\"grant_type\":\"password\"}"
        [ x"$WEBLIB_CURLFAIL_GETTOKEN" = xprotected ] && \
            curlfail_push_expect_noerrors
        _TOKEN_RAW_="`set +x; api_post "$AUTH_URL" "$AUTH_DATA"`" || _RES_=$?
        [ x"$WEBLIB_CURLFAIL_GETTOKEN" = xprotected ] && \
            curlfail_pop
        _TOKEN_="`echo "$_TOKEN_RAW_" | sed -n 's|.*\"access_token\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'`" || _RES_=$?
#	echo "CI-WEBLIB-DEBUG: _api_get_token(): got ($_RES_) new token '$_TOKEN_'" >&2
    fi
    echo "$_TOKEN_"
    return $_RES_
}

# Common helper routine which passes $OUT_CURL to the $JSONSH parser to
# normalize output and either succeed or report errors and dump the input.
# Called from api_*_json functions after they populate OUT_CURL by API calls.
_normalize_OUT_CURL_json() {
    [ x"$JSONSH_CLI_DEFINED" != xyes ] && logmsg_error "jsonsh_cli() not defined" && return 127
    JSONSH_RES=0
    echo "$OUT_CURL" | jsonsh_cli -N || \
        JSONSH_RES=$?
    [ "$JSONSH_RES" = 0 ] && return 0
    logmsg_debug "The '$JSONSH' JSON normalization choked on this input:" \
        "### ===vvvvvv" "$OUT_CURL" "### ===^^^^^^"
    return $JSONSH_RES
}

# Does an api GET request without authorization
# Params:
#	$1	Relative URL for API call
# Result:
#    content + HTTP headers
api_get() {
    CURL --insecure -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# Does an api GET request without authorization
# Params:
#	$1	Relative URL for API call
# Result:
#    content without HTTP headers
api_get_json() {
    api_get "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# Does an api POST request without authorization
# Params:
#	$1	Relative URL for API call
#	$2	data
# Result:
#    content + HTTP headers
api_post() {
    CURL --insecure -v -d "$2" --progress-bar "$BASE_URL$1" -X "POST"  3>&2 2>&1
}

# Does an api DELETE request without authorization
# Params:
#	$1	Relative URL for API call
# Result:
#    content + HTTP headers
api_delete() {
    CURL --insecure -v --progress-bar "$BASE_URL$1" -X "DELETE"  3>&2 2>&1
}

# Does an api POST request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
#	$2	data
#   $@  aditional params for curl
# Result:
#    content + HTTP headers
api_auth_post() {
    local url data
    url="$1"
    data="$2"
    shift 2
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$data" \
        -v --progress-bar "$BASE_URL$url" "$@" 3>&2 2>&1
}

# Does an api DELETE request without authorization
# Params:
#	$1	Relative URL for API call
# Result:
#    content without HTTP headers
api_delete_json() {
    api_delete "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# Does an api POST request without authorization
# Params:
#	$1	Relative URL for API call
#	$2	data
# Result:
#    content without HTTP headers
api_post_json() {
    api_post "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# Does an api GET request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
# Result:
#    content without HTTP headers
api_auth_get_json() {
    api_auth_get "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# Does an api POST request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
#	$2	data
#   $@  aditional params for curl
# Result:
#    content without HTTP headers
api_auth_post_json() {
    api_auth_post "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# Does an api DELETE request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
# Result:
#    content without HTTP headers
api_auth_delete_json() {
    api_auth_delete "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}

# POST the file to the server with Content-Type multipart/form-data according
# to RFC 2388 - this simulates the HTML form and Submit button
#
# Params:
#	$1	Relative URL for API call
#	$2	filename=@/file/path[;mime/type]
#   $@  aditional params for curl
# Example:
#   send file 'assets':
#   api_auth_post_file_form assets=@tests/persist/test-loadcsv.cc.csv
#   send file 'foo' with proper mime type
#   api_auth_post_file_form foo=@path/to/foo.json;type=application/json
#   see man curl, parameter -F/--form
# Result:
#    HTTP headers + content
api_auth_post_file_form() {
    local url data
    url="$1"
    data="$2"
    shift 2
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure -H "Expect:" --header "Authorization: Bearer $TOKEN" --form "$data" \
        -v --progress-bar "$BASE_URL$url" "$@" 3>&2 2>&1
}

# this version of api_auth_post_file* uses the --data instead of --form switch
api_auth_post_file_data() {
    local url data
    url="$1"
    data="$2"
    shift 2
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure -H "Expect:" --header "Authorization: Bearer $TOKEN" --data "$data" \
        -v --progress-bar "$BASE_URL$url" "$@" 3>&2 2>&1
}


api_auth_post_file_form_json() {
    api_auth_post_file_form "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}


api_auth_post_file_data_json() {
    api_auth_post_file_data "$@" > /dev/null || return $?
    _normalize_OUT_CURL_json
}


# Does an api DELETE request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
# Result:
#    HTTP headers + content
api_auth_delete() {
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" -X "DELETE" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# Does an api PUT request with authorization
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#	$1	Relative URL for API call
#	$2	data
# Result:
#    HTTP headers + content
api_auth_put() {
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$2" -X "PUT" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# Does an api GET request with authorization
# Similar to api_auth_get_wToken
# Authorization is done through HTTP header --header "Authorization: Bearer $TOKEN"
# Params:
#    $1  Relative URL for API call
# Result:
#    HTTP headers + content
api_auth_get() {
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# Does an api GET request with authorization
# Similar to api_auth_get
# Authorization is done through URL parameter access_token=$TOKEN"
# Params:
#    $1  Relative URL for API call
# Result:
#    HTTP headers + content
api_auth_get_wToken() {
    TOKEN="`_api_get_token`" || return $?
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    CURL --insecure -v --progress-bar \
        "$BASE_URL$1$URLSEP""access_token=$TOKEN" 3>&2 2>&1
}

# GETs a resource with authentication passed in headers
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
# NOTE: Not for direct use in CI tests since REST API returns JSON which we test
api_auth_get_content() {
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" "$BASE_URL$1" 3>&2 2>/dev/null
}

# GETs a resource with authentication passed in GET arguments
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
api_auth_get_content_wToken() {
    TOKEN="`_api_get_token`" || return $?
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    CURL --insecure "$BASE_URL$1$URLSEP""access_token=$TOKEN" 3>&2 2>/dev/null
}

# POSTs to a resource with authentication passed in headers
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
# NOTE: Not for direct use in CI tests since REST API returns JSON which we test
api_auth_post_content() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$2" \
        "$BASE_URL$1" 3>&2 2>/dev/null
}

# POSTs to a resource with authentication passed in a POSTed form data
# Returns stdout merged stderr reports, headers, verbosity, etc. and raw response data
api_auth_post_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure -d "access_token=$TOKEN&$2" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# POSTs to a resource with authentication passed in a POSTed form data
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
api_auth_post_content_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`" || return $?
    CURL --insecure -d "access_token=$TOKEN&$2" \
        "$BASE_URL$1" 3>&2 2>/dev/null
}

# GETs an anonymous resource without authentication
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
# NOTE: Not for direct use in CI tests since REST API returns JSON which we test
api_get_content() {
    CURL --insecure "$BASE_URL$1" 3>&2 2>/dev/null
}

# POSTs to an anonymous resource without authentication
# Strips stderr reports, headers, verbosity, etc. and returns raw response data
# NOTE: Not for direct use in CI tests since REST API returns JSON which we test
api_post_content() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    CURL --insecure -d "$2" "$BASE_URL$1" 3>&2 2>/dev/null
}


# Returns:
#   1 on error
#   0 on success
# Arguments:
#   $1 - rest api call
#   $2 - output
#   $3 - HTTP code
# TODO:
#   check args
simple_get_json_code() {
    if [ x"$JSONSH_CLI_DEFINED" != xyes ] ; then
        simple_get_json_code_sed "$@"
        return $?
    fi

    local __out
    __out="$( curl -s --insecure -v --progress-bar "$BASE_URL$1" 2>&1 )"
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | jsonsh_cli -N`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode="$3"
    local __resultout="$2"
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

# Returns:
#   1 on error
#   0 on success
# Arguments:
#   $1 - rest api call
#   $2 - output
#   $3 - HTTP code
# TODO:
#   check args
simple_get_json_code_sed() {
    ### Old approach to strip any whitespace including linebreaks from JSON
    local __out
    __out="$( curl -s --insecure -v --progress-bar "$BASE_URL$1" 2>&1 )"
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*' | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode="$3"
    local __resultout="$2"
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

# license accept (note: no password change)
accept_license(){
    curlfail_push_expect_noerrors
    if CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no WEBLIB_CURLFAIL=no api_get_json '/admin/license/status' | grep "accepted_at"; then
        echo 'WEBLIB::accept_license(): license already accepted'
    else
        if [ x"${SKIP_LICENSE_FORCEACCEPT-}" = xyes ] ; then
            echo "WEBLIB::accept_license(): SKIP_LICENSE_FORCEACCEPT=$SKIP_LICENSE_FORCEACCEPT so not running '00_license-CI-forceaccept.sh.test' first"
            logmsg_warn "42ity license not accepted on the server, subsequent tests may fail"
        else
            ### Note: If CITEST_QUICKFAIL=yes then the license-acceptance can kill the test suite
            echo "WEBLIB::accept_license(): Trying to accept the license via REST API on 42ity server '$BASE_URL'..."
            api_auth_post_json '/admin/license' "foobar" || \
            ( SKIP_SANITY=yes; WEBLIB_CURLFAIL=no; CITEST_QUICKFAIL=no; WEBLIB_QUICKFAIL=no
              export SKIP_SANITY WEBLIB_CURLFAIL CITEST_QUICKFAIL WEBLIB_QUICKFAIL
              . "$CHECKOUTDIR"/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
                if [ x"$CITEST_QUICKFAIL" = xyes ] || [ x"$WEBLIB_QUICKFAIL" = xyes ] ; then
                    die "42ity license not accepted on the server, subsequent tests will fail"
                else
                    logmsg_warn "42ity license not accepted on the server, subsequent tests may fail"
                fi
        fi
    fi
    curlfail_pop
}

:

