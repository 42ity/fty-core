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
#! \file    testlib.sh
#  \brief   library of functions useful for general testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \details This is library of functions useful for general testing,
#           which can be sourced to interactive shell
#           You can 'export CITEST_QUICKFAIL=yes' to abort on first failure
#           Tests start with a `test_it "testname"` to initialize, and end
#           with a `print_results $?` to account successes and failures.
#           When doing TDD, you can use `print_results -$?` for tests that
#           are expected/allowed to fail but this should not cause overall
#           test-suite error (testing stuff known as not implemented yet).

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
# this is a shared variable between test_it and print_result
# that should not allow to call
#   print_it before apripriate test_it
#   test_it before previous result was printed
_testlib_result_printed=yes

# Numeric counters
[ -z "${TESTLIB_COUNT_PASS-}" ] && TESTLIB_COUNT_PASS="0"
[ -z "${TESTLIB_COUNT_SKIP-}" ] && TESTLIB_COUNT_SKIP="0"
[ -z "${TESTLIB_COUNT_FAIL-}" ] && TESTLIB_COUNT_FAIL="0"
[ -z "${TESTLIB_COUNT_TOTAL-}" ] && TESTLIB_COUNT_TOTAL="0"
# String lists of space-separated single-token test names that failed
[ -z "${TESTLIB_LIST_FAILED-}" ] && TESTLIB_LIST_FAILED=""
[ -z "${TESTLIB_LIST_FAILED_IGNORED-}" ] && TESTLIB_LIST_FAILED_IGNORED=""
[ -z "${TESTLIB_LIST_PASSED-}" ] && TESTLIB_LIST_PASSED=""

print_result() {
    [ "${_testlib_result_printed}" = yes ] && return 0
    _testlib_result_printed=yes
    _ret="$1"
    ### Is this a valid number (negative == failed_ignored)?
    ### If not - it may be some text comment about the error.
    [ "$_ret" -ge 0 -o "$_ret" -le 0 ] 2>/dev/null || \
        _ret=255

    # Produce a single-token name for the failed test, including its
    # positive return-code value
    if [ "$_ret" -lt 0 ] 2>/dev/null ; then
        _ret="`expr -1 \* $_ret`"
    fi

    if [ "$TNAME" = "`basename $NAME .sh`" ]; then
        TESTLIB_LASTTESTTAG="`echo "$NAME(${_ret})" | sed 's, ,__,g'`"
        logmsg_info "Completed test $TNAME :"
    else
        TESTLIB_LASTTESTTAG="`echo "$NAME::$TNAME(${_ret})" | sed 's, ,__,g'`"
        logmsg_info "Completed test $NAME::$TNAME :"
    fi

    if [ "$_ret" -eq 0 ]; then  # should include "-0" too
        echo " * PASSED"
        TESTLIB_COUNT_PASS="`expr $TESTLIB_COUNT_PASS + 1`"
        TESTLIB_LIST_PASSED="$TESTLIB_LIST_PASSED $TESTLIB_LASTTESTTAG"
    else
        if [ x-"$_ret" = x"$1" ] ; then
            # The "$1" string was a negative number
            TESTLIB_COUNT_SKIP="`expr $TESTLIB_COUNT_SKIP + 1`"
            echo " * FAILED_IGNORED ($_ret)"
            TESTLIB_LIST_FAILED_IGNORED="$TESTLIB_LIST_FAILED_IGNORED $TESTLIB_LASTTESTTAG"
            echo
            return $_ret
        fi

        # Positive _ret, including 255 set for a failure with comment
        # Unlike ignored negative retcodes above, this can abort the script
        [ x"$_ret" = x"$1" ] && \
            echo " * FAILED ($_ret)" || \
            echo " * FAILED ($_ret, $1)"

        TESTLIB_LIST_FAILED="$TESTLIB_LIST_FAILED $TESTLIB_LASTTESTTAG"
        TESTLIB_COUNT_FAIL="`expr $TESTLIB_COUNT_FAIL + 1`"

	# This optional envvar can be set by the caller
	if [ "$CITEST_QUICKFAIL" = yes ]; then
	    echo ""
	    echo "$TESTLIB_COUNT_PASS previous tests have succeeded"
	    echo "CI-TESTLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"CITEST_QUICKFAIL=$CITEST_QUICKFAIL" \
		"after first failure with test $TESTLIB_LASTTESTTAG"
	    exit $_ret
	fi >&2

	# This optional envvar can be set by CURL() and trap_*() below
	if [ "$TESTLIB_FORCEABORT" = yes ]; then
	    echo ""
	    echo "$TESTLIB_COUNT_PASS previous tests have succeeded"
	    echo "CI-TESTLIB-FATAL-ABORT[$$]: Testing aborted due to" \
		"TESTLIB_FORCEABORT=$TESTLIB_FORCEABORT" \
		"after forced abortion in test $TESTLIB_LASTTESTTAG"
	    exit $_ret
	fi >&2
    fi
    echo
    return $_ret
}

test_it() {
    if [ x"${_testlib_result_printed}" = xnotyet ]; then
        logmsg_warn "Starting a new test_it() while an old one was not followed by a print_result()!"
        logmsg_warn "Closing old test with result code 128 ..."
        print_result 128
    fi
    _testlib_result_printed=notyet
    [ -n "$TNAME" ] || TNAME="$NAME"
    if [ "$1" ]; then
        TNAME="$1"
    fi
    [ -n "$TNAME" ] || TNAME="$0"
    TNAME="`basename "$TNAME" .sh | sed 's, ,__,g'`"
    if [ "$TNAME" = "`basename $NAME .sh`" ]; then
        logmsg_info "Running test $TNAME :"
    else
        logmsg_info "Running test $NAME::$TNAME :"
    fi
    TESTLIB_COUNT_TOTAL="`expr $TESTLIB_COUNT_TOTAL + 1`"
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

# A consumer script can set this as (part of) their exit/abort-trap to always
# print a summary of processed tests in the end, whatever the reason to exit().
exit_summarizeResults() {
    TRAP_RES=$?
    # This would be a no-op if the test case previously started with a
    # test_it() has been already closed with its proper print_result()
    print_result $TRAP_RES
    set +e
    NUM_NOTFAILED="`expr $TESTLIB_COUNT_PASS + $TESTLIB_COUNT_SKIP`"
    logmsg_info "Testing completed ($TRAP_RES), $NUM_NOTFAILED/$TESTLIB_COUNT_TOTAL tests passed($TESTLIB_COUNT_PASS) or not-failed($TESTLIB_COUNT_SKIP) for test-groups:"
    logmsg_info "  POSITIVE (exec glob) = $POSITIVE"
    logmsg_info "  NEGATIVE (skip glob) = $NEGATIVE"
    logmsg_info "  SKIP_NONSH_TESTS = $SKIP_NONSH_TESTS (so skipped $SKIPPED_NONSH_TESTS tests)"
    if [ -n "$TESTLIB_LIST_FAILED_IGNORED" ]; then
        logmsg_info "The following $TESTLIB_COUNT_SKIP tests have failed but were ignored (TDD in progress):"
        for i in $TESTLIB_LIST_FAILED_IGNORED; do
            echo " * $i"
        done
    fi
    NUM_FAILED="`expr $TESTLIB_COUNT_TOTAL - $NUM_NOTFAILED`"
    if [ -z "$TESTLIB_LIST_FAILED" ] && [ x"$TESTLIB_COUNT_FAIL" = x0 ] && [ x"$NUM_FAILED" = x0 ]; then
        [ -z "$TRAP_RES" ] && TRAP_RES=0
        exit $TRAP_RES
    fi
    logmsg_info "The following $TESTLIB_COUNT_FAILED tests have failed:"
    N=0 # Do a bit of double-accounting to be sure ;)
    for i in $TESTLIB_LIST_FAILED; do
        echo " * $i"
        N="`expr $N + 1`"
    done
    [ x"$TESTLIB_COUNT_FAIL" = x"$NUM_FAILED" ] && \
    [ x"$N" = x"$NUM_FAILED" ] && \
    [ x"$TESTLIB_COUNT_FAIL" = x"$N" ] || \
        logmsg_error "TEST-LIB accounting fault: failed-test counts mismatched: TESTLIB_COUNT_FAIL=$TESTLIB_COUNT_FAIL vs NUM_FAILED=$NUM_FAILED vs N=$N"
    logmsg_error "$N/$TESTLIB_COUNT_TOTAL tests FAILED, $TESTLIB_COUNT_SKIP tests FAILED_IGNORED, $TESTLIB_COUNT_PASS tests PASSED"
    unset N

    # If we are here, we've at least had some failed tests
    [ -z "$TRAP_RES" -o "$TRAP_RES" = 0 ] && TRAP_RES=1
    exit $TRAP_RES
}

# common testing function which compares outputs to a pattern
# intended for use with weblib.sh driven tests
# Usage:
# do_test $test_name $api_call $url $regexp
# do_test $test_name $api_call $url $url_args $regexp
do_test_match() {
    local test_name api_call url api_args regexp out err

    case $# in
        0|1|2|3)
            logmsg_error "do_test_match(): insuficient number of arguments"
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
            logmsg_error "do_test_match(): too many arguments: $#"
            return 1
            ;;
    esac

    out="${BUILDSUBDIR}/${test_name}.stdout.$$.log"
    err="${BUILDSUBDIR}/${test_name}.stderr.$$.log"

    test_it "${test_name}"
    ${api_call} ${url} "${api_args}" > "${out}"  2> "${err}"
    RES=$?
    if ! egrep -q "${regexp}" "${out}"; then
        echo "    >>>>> DEBUG: ${out} <<<<"
        cat "${out}"
        echo "    >>>>> DEBUG: ${err} <<<<"
        cat "${err}"
        echo "    >>>>> \\DEBUG: ${test_name} <<<<"
        [ "$RES" = 0 ] && RES=1
        print_result $RES
        return $RES
    fi
    rm -f "${err}" "${out}"
    print_result 0
    return 0
}
