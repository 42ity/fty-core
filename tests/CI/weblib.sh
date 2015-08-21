#!/bin/sh
#
# Copyright (C) 2014 Eaton
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
#  \brief   library of functions usefull for REST API testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \details This is library of functions usefull for REST API testing,
#           which can be sourced to interactive shell
#           You can 'export TESTWEB_QUICKFAIL=yes' to abort on first failure

# ***********************************************
echo "INFO-WEBLIB: Initial  BASE_URL = '$BASE_URL'"

[ -z "${SUT_HOST-}" ] && SUT_HOST="127.0.0.1"
[ -z "${SUT_WEB_PORT-}" ] && SUT_WEB_PORT="8000"
[ -z "${BIOS_USER-}" ] && BIOS_USER="bios"
[ -z "${BIOS_PASSWD-}" ] && BIOS_PASSWD="@PASSWORD@"
[ -z "${SASL_SERVICE-}" ] && SASL_SERVICE="bios"
[ -z "${BASE_URL-}" ] && BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
#[ -z "${BASE_URL-}" ] && BASE_URL="http://127.0.0.1:8000/api/v1"
#[ -z "${BASE_URL-}" ] && BASE_URL="http://root@debian.roz.lab.etn.com:8007/api/v1"

echo "INFO-WEBLIB: Will use BASE_URL = '$BASE_URL'"

# Should the test suite abort if "curl" errors out?
[ -z "${WEBLIB_CURLFAIL-}" ] && WEBLIB_CURLFAIL=yes

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
        . "$CHECKOUTDIR/tests/CI"/testlib.sh || exit
else
        . "$SCRIPTDIR"/testlib.sh || exit
fi

### Should the test suite break upon first failed test?
### Legacy weblib value (may be set by caller/includer)
### overrides the common testlib variable
[ x"${WEBLIB_QUICKFAIL-}" = xno -o x"${WEBLIB_QUICKFAIL-}" = xyes ] && \
        echo "CI-WEBLIB-INFO: Overriding CITEST_QUICKFAIL with WEBLIB_QUICKFAIL='$WEBLIB_QUICKFAIL'" && \
        CITEST_QUICKFAIL="$WEBLIB_QUICKFAIL"

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

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
    [ -n "$_PID_TESTER" ] && \
    kill -SIGUSR2 $_PID_TESTER $$ >/dev/null 2>&1

    exit $RES_CURL
}
trap "trap_break_weblib" SIGUSR1 || \
trap "trap_break_weblib" USR1

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

curlfail_push_expect_404() {
    ### Preconfigured push that resets current values to specifically
    ### expect an HTTP-404 not found (and fail for other results)
    curlfail_push "expect" 'HTTP/[^ ]+ 404'
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
        OUT_CURL=""
        ERR_CURL=""
#        OUT_CURL="`curl "$@"`"
        curl "$@"
        RES_CURL=$?
#        echo "$OUT_CURL"
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
        ERR_MATCH="`( echo "$ERR_CURL"; echo "" ) | tr '\r' '\n' | egrep '^( *< |wget: server returned error: )'"$WEBLIB_HTTPERRORS_REGEX"`" 2>&3 || true
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
        kill -SIGUSR1 $_PID_TESTER_WEBLIB $$ >/dev/null 2>&1
        # exit $RES_CURL
    fi

    return $RES_CURL
}

api_get() {
    CURL --insecure -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_get_content() {
    CURL --insecure "$BASE_URL$1" 3>&2 2>/dev/null
}

api_get_json() {
    ### Properly normalize JSON markup into a single string via JSON.sh
    if [ -z "$JSONSH" -o ! -x "$JSONSH" ] ; then
        api_get_json_sed "$@"
        return $?
    fi

    CURL --insecure -v --progress-bar "$BASE_URL$1" 3>&2 2> /dev/null \
    | $JSONSH -N
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
    if [ -z "$JSONSH" -o ! -x "$JSONSH" ] ; then
        simple_get_json_code_sed "$@"
        return $?
    fi

    local __out
    __out=$( curl -s --insecure -v --progress-bar "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | $JSONSH -N`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode=$3
    local __resultout=$2
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

api_get_json_sed() {
    ### Old approach to strip any whitespace including linebreaks from JSON
    CURL --insecure -v --progress-bar "$BASE_URL$1" 3>&2 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
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
    __out=$( curl -s --insecure -v --progress-bar "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*' | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode=$3
    local __resultout=$2
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

api_get_jsonv() {
    ### Sort of a JSON validity check by passing it through Python parser
    api_get_json "$@" | \
        python -c "import sys, json; s=sys.stdin.read(); json.loads(s); print(s)"
}

api_post() {
    CURL --insecure -v -d "$2" --progress-bar "$BASE_URL$1" 3>&2 2>&1
}
simple_post_code() {

    local __out=
    __out=$( curl -s --insecure -v --progress-bar -d "$2" -X POST "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | $JSONSH -N`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode=$4
    local __resultout=$3
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

### Flag for _api_get_token_certainPlus()
LOGIN_RESET="no"
_api_get_token() {
    _RES_=0
    if [ -z "$_TOKEN_" -o x"$LOGIN_RESET" = xyes ]; then
	AUTH_URL="/oauth2/token?username=${BIOS_USER}&password=${BIOS_PASSWD}&grant_type=password"
	[ x"$LOGIN_RESET" = xyes ] && AUTH_URL="${AUTH_URL}&grant_reset=true&grant_reset_inst=true"
        [ x"$WEBLIB_CURLFAIL_GETTOKEN" = xprotected ] && \
            curlfail_push_expect_noerrors
	_TOKEN_RAW_="`set +x; api_get "$AUTH_URL"`" || _RES_=$?
        [ x"$WEBLIB_CURLFAIL_GETTOKEN" = xprotected ] && \
            curlfail_pop
	_TOKEN_="`echo "$_TOKEN_RAW_" | sed -n 's|.*\"access_token\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'`" || _RES_=$?
	echo "CI-WEBLIB-DEBUG: _api_get_token(): got ($_RES_) new token '$_TOKEN_'" >&2
    fi
    echo "$_TOKEN_"
    return $_RES_
}

_api_get_token_certainPlus() {
    _PLUS="$1"
    if [ "$_PLUS" != with -a "$_PLUS" != without ]; then
	echo "CI-WEBLIB-ERROR: _api_get_token_certainPlus():" \
            "unknown certainty was requested: '$_PLUS'" >&2
	return 1
    fi

    _TO="$_TOKEN_"
    C=0
    while : ; do
	_T="`LOGIN_RESET=yes _api_get_token`"
	_RES_=$?
	if [ $_RES_ != 0 ]; then
	    echo "CI-WEBLIB-ERROR: _api_get_token_certainPlus():" \
                "got error from _api_get_token(): $_RES_'" >&2
	    return $_RES_
	fi
	if [ -z "$_T" ]; then
	    echo "CI-WEBLIB-ERROR: _api_get_token_certainPlus():" \
                "got empty token value from _api_get_token()'" >&2
	    return 2
	fi
	case "$_T" in
	    *\+*)	[ "$_PLUS" = with ] && break ;;
	    *)		[ "$_PLUS" = without ] && break ;;
	esac

	echo "CI-WEBLIB-WARN: _api_get_token_certainPlus():" \
            "Got unsuitable token '$_T' (wanted $_PLUS a plus)" >&2

	if [ x"$_TO" = x"$_T" ]; then
	    C="`expr $C + 1`"
	else C=0; fi

	if [ "$C" -gt 5 ]; then
	    echo "CI-WEBLIB-ERROR: _api_get_token_certainPlus():" \
                "Got the same token too many times in a row, aborting loop" >&2
	    echo "$_T"
	    return 3
	fi
	sleep 1
	_TO="$_T"
    done
    _TOKEN_="$_T"
    echo "$_T"
}

_api_get_token_withplus() {
    _api_get_token_certainPlus with
}

_api_get_token_withoutplus() {
    _api_get_token_certainPlus without
}

api_auth_post() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    #   $@  aditional params for curl
    local url data
    url=$1
    data=$2
    shift 2
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$data" \
        -v --progress-bar "$BASE_URL$url" "$@" 3>&2 2>&1
}

simple_auth_post_code () {
    # WARNING: At the moment does not fill in $3... WIP
    TOKEN="`_api_get_token`"

    local __out=
    __out=$( curl -s --insecure --header "Authorization: Bearer $TOKEN" -d "$2" -X "POST" -v --progress-bar "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | $JSONSH -N`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode=$4
    local __resultout=$3
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
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
#   api_auth_post_file assets=@tests/persist/test-loadcsv.cc.csv
#   send file 'foo' with proper mime type
#   api_auth_post_file foo=@path/to/foo.json;type=application/json
#   see man curl, parameter -F/--form
api_auth_post_file() {
    local url data
    url=$1
    data=$2
    shift 2
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" --form "$data" \
        -v --progress-bar "$BASE_URL$url" "$@" 3>&2 2>&1
}

api_auth_post_content() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$2" \
        "$BASE_URL$1" 3>&2 2>/dev/null
}

api_auth_post_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    CURL --insecure -d "access_token=$TOKEN&$2" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_auth_post_content_wToken() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    CURL --insecure -d "access_token=$TOKEN&$2" "$BASE_URL$1" 3>&2 2>/dev/null
}

api_auth_delete() {
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" -X "DELETE" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_auth_put() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	PUT data
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" -d "$2" -X "PUT" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

# Returns:
#   1 on error
#   0 on success
# Arguments:
#   $1 - rest api call
#   $2 - input
#   $3 - output
#   $4 - HTTP code
# TODO:
#   check args
simple_auth_put () {
    TOKEN="`_api_get_token`"

    local __out=
    __out=$( curl -s --insecure --header "Authorization: Bearer $TOKEN" -d "$2" -X "PUT"  -v --progress-bar "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | $JSONSH -N`"
    if [ $? -ne 0 ]; then
        return 1
    fi

    local __resultcode=$4
    local __resultout=$3
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

api_auth_get() {
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

simple_auth_get_code() {
    TOKEN="`_api_get_token`"

    local __out=
    __out=$( curl -s --insecure --header "Authorization: Bearer $TOKEN" -v --progress-bar "$BASE_URL$1" 2>&1 )
    if [ $? -ne 0 ]; then
        return 1
    fi
    local __code="`echo "$__out" | grep -E '<\s+HTTP' | sed -r -e 's/<\s+HTTP\/[1-9]+[.][0-9]+\s+([1-9]{1}[0-9]{2}).*/\1/'`"
    __out="`echo "$__out" | grep -vE '^([<>*]|\{ *\[data not shown\]|\{\s\[[0-9]+).*'`"
    __out="`echo "$__out" | $JSONSH -N`"

    local __resultcode=$3
    local __resultout=$2
    eval $__resultcode='"$__code"'
    eval $__resultout='"$__out"'
    return 0
}

api_auth_get_wToken() {
    TOKEN="`_api_get_token`"
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    CURL --insecure -v --progress-bar \
        "$BASE_URL$1$URLSEP""access_token=$TOKEN" 3>&2 2>&1
}

api_auth_get_content() {
    TOKEN="`_api_get_token`"
    CURL --insecure --header "Authorization: Bearer $TOKEN" "$BASE_URL$1" 3>&2 2>/dev/null
}

api_auth_get_content_wToken() {
    TOKEN="`_api_get_token`"
    URLSEP='?'
    case "$1" in
        *"?"*) URLSEP='&' ;;
    esac
    CURL --insecure "$BASE_URL$1$URLSEP""access_token=$TOKEN" 3>&2 2>/dev/null
}

api_auth_get_json() {
    TOKEN="`_api_get_token`"
    CURL --insecure -v --progress-bar --header "Authorization: Bearer $TOKEN" \
        "$BASE_URL$1" 3>&2 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_auth_get_jsonv() {
    TOKEN="`_api_get_token`"
    api_auth_get_json "$@" | \
        python -c "import sys, json; s=sys.stdin.read(); json.loads(s); print(s)"
}

api_post_json_cmp() {
#    set -x
    text=$(CURL --insecure -v --progress-bar -d "$2" "$BASE_URL$1" 3>&2 2>&1)
    res=$(echo "${text}" | egrep "$3")
    if [ -z "$res" ]; then
        return 1
    else
        return 0
    fi
}
