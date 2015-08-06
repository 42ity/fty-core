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
# Description: This is library of functions useful for general testing,
#              which can be sourced to interactive shell
#              You can 'export CITEST_QUICKFAIL=yes' to abort on first failure

# ***********************************************
### Should the test suite break upon first failed test?
[ x"${CITEST_QUICKFAIL-}" != xyes ] && CITEST_QUICKFAIL=no

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

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

_TOKEN_=""
TESTLIB_FORCEABORT=no
_testlib_result_printed=notest

print_result() {
    [ "$_testlib_result_printed" = yes ] && return 0
    _testlib_result_printed=yes
    _ret="$1"
    ### Is this a valid number? if not - it may be some comment about the error
    [ "$_ret" -ge 0 ] 2>/dev/null || \
        _ret=255
    TOTAL="`expr $TOTAL + 1`"
    if [ "$_ret" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        [ x"$_ret" = x"$1" ] && \
            echo " * FAILED ($_ret)" || \
            echo " * FAILED ($_ret, $1)"
	if [ "$TNAME" = "$NAME" ]; then
            LASTFAILED="`echo "$NAME(${_ret})" | sed 's, ,__,g'`"
	else
            LASTFAILED="`echo "$NAME::$TNAME(${_ret})" | sed 's, ,__,g'`"
	fi
        FAILED="$FAILED $LASTFAILED"

	# This optional envvar can be set by the caller
	if [ "$CITEST_QUICKFAIL" = yes ]; then
	    echo ""
	    echo "$PASS previous tests have succeeded"
	    echo "CI-TESTLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"CITEST_QUICKFAIL=$CITEST_QUICKFAIL" \
		"after first failure with test $LASTFAILED"
	    exit $_ret
	fi >&2

	# This optional envvar can be set by CURL() and trap_*() below
	if [ "$TESTLIB_FORCEABORT" = yes ]; then
	    echo ""
	    echo "$PASS previous tests have succeeded"
	    echo "CI-TESTLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"TESTLIB_FORCEABORT=$TESTLIB_FORCEABORT" \
		"after forced abortion in test $LASTFAILED"
	    exit $_ret
	fi >&2
    fi
    echo
    return $_ret
}

test_it() {
    _testlib_result_printed=notyet
    [ -n "$TNAME" ] || TNAME="$NAME"
    if [ "$1" ]; then
        TNAME="$1"
    fi
    [ -n "$TNAME" ] || TNAME="$0"
    TNAME="`basename "$TNAME" .sh | sed 's, ,__,g'`"
    if [ "$TNAME" = "`basename $NAME .sh`" ]; then
        echo "Running test $TNAME :"
    else
        echo "Running test $NAME::$TNAME :"
    fi
}

### This is what we will sig-kill if needed
_PID_TESTER=$$
RES_TEST=0
trap_break_testlib() {
    ### This SIGUSR2 handler is reserved for testscript-initiated failures
#    set +e
    [ "$RES_TEST" != 0 ] && \
        echo "CI-TESTLIB-ERROR-WEB: test program failed ($RES_TEST), aborting test suite" >&2
    echo "CI-TESTLIB-FATAL-BREAK: Got forced interruption signal" >&2
    TESTLIB_FORCEABORT=yes

### Just cause the loop to break at a proper moment in print_result()
#    exit $1
    return 1
}
trap "trap_break_testlib" SIGUSR2

# common testing function which compares outputs to a pattern
# intended for use with weblib.sh driven tests
# Usage:
# do_test $test_name $api_call $url $regexp
# do_test $test_name $api_call $url $url_args $regexp
do_test_match() {
    local test_name api_call url api_args regexp out err

    case $# in
        0|1|2|3)
            echo "CI-TESTLIB-ERROR: do_test_match(): insuficient number of arguments" >&2
            return 1
            ;;
        4)
            test_name=${1}
            api_call=${2}
            url=${3}
            api_args=""
            regexp=${4}
            ;;
        5)
            test_name=${1}
            api_call=${2}
            url=${3}
            api_args=${4}
            regexp=${5}
            ;;
        *)
            echo "CI-TESTLIB-ERROR: do_test_match(): too many arguments: $#" >&2
            return 1
            ;;
    esac

    out="${BUILDSUBDIR}/${test_name}.stdout.$$.log"
    err="${BUILDSUBDIR}/${test_name}.stderr.$$.log"

    test_it "${test_name}"
    ${api_call} ${url} "${api_args}" > "${out}"  2> "${err}"
    if ! egrep -q "${regexp}" "${out}"; then
        echo "    >>>>> DEBUG: ${out} <<<<"
        cat "${out}"
        echo "    >>>>> DEBUG: ${err} <<<<"
        cat "${err}"
        echo "    >>>>> \\DEBUG: ${test_name} <<<<"
        return 1
    fi
    rm -f "${err}" "${out}"
    return 0
}
