#!/bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Michal Hrusecky <MichalHrusecky@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#            Karol Hrdina <KarolHrdina@eaton.com>
#
# Description: This is library of functions usefull for REST API testing,
#              which can be sourced to interactive shell
#              You can 'export TESTWEB_QUICKFAIL=yes' to abort on first failure

# ***********************************************
echo "INFO-WEBLIB: Initial  BASE_URL = '$BASE_URL'"

[ -z "$SUT_NAME" ] && SUT_NAME="127.0.0.1"
[ -z "$SUT_WEB_PORT" ] && SUT_WEB_PORT="8000"
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"
[ -z "$BASE_URL" ] && BASE_URL="http://$SUT_NAME:$SUT_WEB_PORT/api/v1"
#[ -z "$BASE_URL" ] && BASE_URL="http://127.0.0.1:8000/api/v1"
#[ -z "$BASE_URL" ] && BASE_URL="http://root@debian.roz.lab.etn.com:8007/api/v1"

echo "INFO-WEBLIB: Will use BASE_URL = '$BASE_URL'"

### Should the test suite break upon first failed test?
[ -z "$WEBLIB_QUICKFAIL" ] && WEBLIB_QUICKFAIL=no

# Should the test suite abort if "curl" errors out?
[ -z "$WEBLIB_CURLFAIL" ] && WEBLIB_CURLFAIL=yes

# Should the test suite abort if "curl" sees HTTP error codes?
# This can be overridden on a per-call basis for those api_get's
# where we do expect errors as the proper response.
#       fatal   Fail if errors are detected in stderr contents
#       debug   Do not fail but do print STDERR and STDOUT for the request upon hits
#       ignore  Don't care, and don't test
#       expect  Fail if stderr is not empty but result is OK
#       *       Anything else gives a warning if error was matched and goes on
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS" ] && WEBLIB_CURLFAIL_HTTPERRORS=warn
[ -z "$WEBLIB_HTTPERRORS_REGEX" ] && WEBLIB_HTTPERRORS_REGEX='HTTP/[^ ]+ [45]'

# Print out the CURL stdout and stderr (via FD#3)?
[ -z "$WEBLIB_TRACE_CURL" ] && WEBLIB_TRACE_CURL=no

[ -z "$CHECKOUTDIR" ] && CHECKOUTDIR="`dirname $0`/../.."
[ -z "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"

_TOKEN_=""
WEBLIB_FORCEABORT=no
_weblib_result_printed=no

print_result() {
    [ "$_weblib_result_printed" = yes ] && return 0
    _weblib_result_printed=yes
    _ret=0
    TOTAL="`expr $TOTAL + 1`"
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
        _ret=1
	if [ "$TNAME" = "$NAME" ]; then
            LASTFAILED="`echo "$NAME" | sed 's, ,__,g'`"
	else
            LASTFAILED="`echo "$NAME::$TNAME" | sed 's, ,__,g'`"
	fi
        FAILED="$FAILED $LASTFAILED"

	# This optional envvar can be set by the caller
	if [ "$WEBLIB_QUICKFAIL" = yes ]; then
	    echo ""
	    echo "$PASS previous tests have succeeded"
	    echo "CI-WEBLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"WEBLIB_QUICKFAIL=$WEBLIB_QUICKFAIL" \
		"after first failure with test $LASTFAILED"
	    exit $_ret
	fi >&2

	# This optional envvar can be set by CURL() and trap_*() below
	if [ "$WEBLIB_FORCEABORT" = yes ]; then
	    echo ""
	    echo "$PASS previous tests have succeeded"
	    echo "CI-WEBLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"WEBLIB_FORCEABORT=$WEBLIB_FORCEABORT" \
		"after forced abortion in test $LASTFAILED"
	    exit $_ret
	fi >&2
    fi
    echo
    return $_ret
}

test_it() {
    _weblib_result_printed=no
    [ -n "$TNAME" ] || TNAME="$NAME"
    if [ "$1" ]; then
        TNAME="$1"
    fi
    [ -n "$TNAME" ] || TNAME="$0"
    TNAME="`basename "$TNAME" .sh | sed 's, ,__,g'`"
    if [ "$TNAME" = "`basename "$NAME" .sh`" ]; then
        echo "Running test $TNAME :"
    else
        echo "Running test $NAME::$TNAME :"
    fi
}

### This is what we will sig-kill if needed
_PID_TESTER=$$
RES_CURL=0
trap_break() {
    ### This SIGUSR1 handler is reserved for CURL failures
    [ "$RES_CURL" = 0 ] && \
        echo "CI-WEBLIB-ERROR-WEB: curl program failed, aborting test suite" >&2 || \
        echo "CI-WEBLIB-ERROR-WEB: curl program failed ($RES_CURL), aborting test suite" >&2
    echo "CI-WEBLIB-FATAL-BREAK: Got forced interruption signal" >&2
    WEBLIB_FORCEABORT=yes

### Just cause the loop to break at a proper moment in print_result()
#    exit $1
    return 1
}
trap "trap_break" SIGUSR1

CURL() {
    _TMP_CURL="/tmp/.bios_weblib_curl.$$"
    /bin/rm -f "$_TMP_CURL" 2>/dev/null
    if touch "$_TMP_CURL" && chmod 600 "$_TMP_CURL" ; then
        OUT_CURL="`curl --stderr "$_TMP_CURL" "$@"`"
        RES_CURL=$?
        ERR_CURL="`cat "$_TMP_CURL"`"
        /bin/rm -f "$_TMP_CURL" 2>/dev/null
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
        ### Based on caller redirections, this output may never be seen
        echo "CI-WEBLIB-ERROR-CURL: 'curl $@' program failed ($RES_CURL)," \
            "perhaps the web server is not available or has crashed?" >&3
        _PRINT_CURL_TRACE=yes
    fi

    if [ -n "$ERR_CURL" -a x"$WEBLIB_CURLFAIL_HTTPERRORS" != xignore ]; then
        ERR_MATCH="`( echo "$ERR_CURL"; echo "" ) | tr '\r' '\n' | egrep '^ *< '"$WEBLIB_HTTPERRORS_REGEX"`"
        if [ -n "$ERR_MATCH" ]; then
            echo "CI-WEBLIB-WARN-CURL: Last response headers matched an HTTP" \
                "error code: '$ERR_MATCH'" >&3
            [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xdebug ] && _PRINT_CURL_TRACE=yes
            if [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xfatal ]; then
                echo "CI-WEBLIB-ERROR-CURL: WEBLIB_CURLFAIL_HTTPERRORS=fatal" \
                    "is set, so failing now" >&3
                RES_CURL=123
            fi
        else
            ### No match
            if [ x"$WEBLIB_CURLFAIL_HTTPERRORS" = xexpect ]; then
                echo "CI-WEBLIB-ERROR-CURL: Last response headers did not match" \
                    "an HTTP error pattern '$WEBLIB_HTTPERRORS_REGEX', while" \
                    "WEBLIB_CURLFAIL_HTTPERRORS=expect is set, so failing now" >&3
                RES_CURL=124
            fi
        fi
    fi

    if [ x"$WEBLIB_TRACE_CURL" = xyes -o x"$_PRINT_CURL_TRACE" = xyes ]; then
        echo "CI-WEBLIB-TRACE-CURL: curl $@: RES=$RES"
        echo "=== ERR vvv"
        echo "$ERR_CURL"
        echo "/// ERR ^^^"
        echo "=== OUT vvv"
        echo "$OUT_CURL"
        echo "/// OUT ^^^"
    fi >&3

    if [ $RES_CURL != 0 -a x"$WEBLIB_CURLFAIL" = xyes ]; then
        kill -SIGUSR1 $_PID_TESTER $$ >/dev/null 2>&1
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

api_get_json_sed() {
    ### Old approach to strip any whitespace including linebreaks from JSON
    CURL --insecure -v --progress-bar "$BASE_URL$1" 3>&2 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_get_jsonv() {
    ### Sort of a JSON validity check by passing it through Python parser
    api_get_json "$@" | \
        python -c "import sys, json; s=sys.stdin.read(); json.loads(s); print(s)"
}

api_post() {
    CURL --insecure -v -d "$2" --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

### Flag for _api_get_token_certainPlus()
LOGIN_RESET="no"
_api_get_token() {
    _RES_=0
    if [ -z "$_TOKEN_" -o x"$LOGIN_RESET" = xyes ]; then
	AUTH_URL="/oauth2/token?username=${BIOS_USER}&password=${BIOS_PASSWD}&grant_type=password"
	[ x"$LOGIN_RESET" = xyes ] && AUTH_URL="${AUTH_URL}&grant_reset=true&grant_reset_inst=true"
	_TOKEN_RAW_="`api_get "$AUTH_URL"`" || _RES_=$?
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
    TOKEN="`_api_get_token`"
    CURL --insecure -H "Authorization: Bearer $TOKEN" -d "$2" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_auth_post_content() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	POST data
    TOKEN="`_api_get_token`"
    CURL --insecure -H "Authorization: Bearer $TOKEN" -d "$2" \
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
    CURL --insecure -H "Authorization: Bearer $TOKEN" -X "DELETE" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_auth_put() {
    # Params:
    #	$1	Relative URL for API call
    #	$2	PUT data
    TOKEN="`_api_get_token`"
    CURL --insecure -H "Authorization: Bearer $TOKEN" -d "$2" -X "PUT" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
}

api_auth_get() {
    TOKEN="`_api_get_token`"
    CURL --insecure -H "Authorization: Bearer $TOKEN" \
        -v --progress-bar "$BASE_URL$1" 3>&2 2>&1
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
    CURL --insecure -H "Authorization: Bearer $TOKEN" "$BASE_URL$1" 3>&2 2>/dev/null
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
    CURL --insecure -v --progress-bar -H "Authorization: Bearer $TOKEN" \
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
