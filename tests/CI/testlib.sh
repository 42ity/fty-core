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
#! \file    testlib.sh
#  \brief   library of functions useful for general testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \details This is library of functions useful for general testing,
#           which can be sourced to interactive shell
#           You can 'export CITEST_QUICKFAIL=yes' to abort on first failure
#           Tests start with a `test_it "testname"` to initialize, and end
#           with a `print_result $?` to account successes and failures.
#           When doing TDD, you can use `print_result -$?` for tests that
#           are expected/allowed to fail but this should not cause overall
#           test-suite error (testing stuff known as not implemented yet).

# ***********************************************

### Should the test suite break upon first failed test?
[ x"${CITEST_QUICKFAIL-}" = x- ] && CITEST_QUICKFAIL=""
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
TESTLIB_FORCEABORT=no
# this is a shared variable between test_it and print_result
# that should not allow to call
#   print_result before apropriate test_it
#   test_it before previous result was printed with print_result
# possible values:
#   notest - test suite not yet started (zero test_it's called)
#   notyet - a test_it() clause began a routine, but no result so far
#   yes    - a print_result() completed a previous test routine and
#            no new one was started yet; it is not a fatal error to
#            call print_result() when in this state, but is a warning
_testlib_result_printed=notest
LOGMSG_PREFIX_TESTLIB="CI-TESTLIB-"
# TNAME of the specific test as named by the test_it() argument
TNAME=""
# NAME of the tested scriptlet (if used, or the parent script by default)
NAME="$0"

# Numeric counters
[ -z "${TESTLIB_COUNT_PASS-}" ] && TESTLIB_COUNT_PASS=0
[ -z "${TESTLIB_COUNT_SKIP-}" ] && TESTLIB_COUNT_SKIP=0
[ -z "${TESTLIB_COUNT_FAIL-}" ] && TESTLIB_COUNT_FAIL=0
[ -z "${TESTLIB_COUNT_TOTAL-}" ] && TESTLIB_COUNT_TOTAL=0
# String lists of space-separated single-token test names that failed
[ -z "${TESTLIB_LIST_FAILED-}" ] && TESTLIB_LIST_FAILED=""
[ -z "${TESTLIB_LIST_FAILED_IGNORED-}" ] && TESTLIB_LIST_FAILED_IGNORED=""
[ -z "${TESTLIB_LIST_PASSED-}" ] && TESTLIB_LIST_PASSED=""

# Should we track and add timestamps to each test (profile what took long)?
[ x"${TESTLIB_PROFILE_TESTDURATION-}" = x- ] && TESTLIB_PROFILE_TESTDURATION=""
[ -z "${TESTLIB_PROFILE_TESTDURATION-}" ] && \
    if [ -n "${CI_DEBUG-}" ] && [ -n "${CI_DEBUGLEVEL_DEBUG-}" ] && [ "$CI_DEBUG" -ge "$CI_DEBUGLEVEL_DEBUG" ]; then
        TESTLIB_PROFILE_TESTDURATION="yes"
    else
        TESTLIB_PROFILE_TESTDURATION="no"
    fi
# ... and summarize longest tests in the end?
[ -z "${TESTLIB_PROFILE_TESTDURATION_TOP-}" ] && TESTLIB_PROFILE_TESTDURATION_TOP=10
export TESTLIB_PROFILE_TESTDURATION TESTLIB_PROFILE_TESTDURATION_TOP
# Do not export these local variables...
[ -z "${TESTLIB_TIMESTAMP_TESTSTART-}" ] && TESTLIB_TIMESTAMP_TESTSTART=0
[ -z "${TESTLIB_TIMESTAMP_TESTFINISH-}" ] && TESTLIB_TIMESTAMP_TESTFINISH=0
[ -z "${TESTLIB_TIME_INTESTS-}" ] && TESTLIB_TIME_INTESTS=-1
[ -z "${TESTLIB_TIME_BEFORETESTS-}" ] && TESTLIB_TIME_BEFORETESTS=-1
[ -z "${TESTLIB_TIME_AFTERTESTS-}" ] && TESTLIB_TIME_AFTERTESTS=-1
[ -z "${TESTLIB_TIME_BETWEENTESTS-}" ] && TESTLIB_TIME_BETWEENTESTS=-1

# We do not want to be spammed by profiling reports if overhead is negligible
# Note these are printed with logmsg_debug() anyway
[ x"${TESTLIB_SPENT_BETWEENTESTS_REPORT-}" = x- ] && TESTLIB_SPENT_BETWEENTESTS_REPORT=""
[ -z "${TESTLIB_SPENT_BETWEENTESTS_REPORT-}" ] && \
    if [ -n "${CI_DEBUG-}" ] && [ -n "${CI_DEBUGLEVEL_DEBUG-}" ] && [ "$CI_DEBUG" -gt "$CI_DEBUGLEVEL_DEBUG" ]; then
        TESTLIB_SPENT_BETWEENTESTS_REPORT=0
    else
        TESTLIB_SPENT_BETWEENTESTS_REPORT=3
    fi

# Predefine for "between" accounting
if [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && [ "$TESTLIB_TIMESTAMP_TESTFINISH" -eq 0 ]; then
    TESTLIB_TIMESTAMP_TESTFINISH=-"`date -u +%s 2>/dev/null`" \
    || TESTLIB_TIMESTAMP_TESTFINISH=0
fi

# Optional filename (preferably full path) that can be set by caller - if it is
# set, then the exit_summarizeTestlibResults() would append its output there.
[ -z "${TESTLIB_LOG_SUMMARY-}" ] && TESTLIB_LOG_SUMMARY=""
[ -z "${TESTLIB_LOG_SUMMARY_COMMENT-}" ] && TESTLIB_LOG_SUMMARY_COMMENT=""

print_result() {
    # $1 = exit-code of the test being closed:
    #   0|-0    success
    #   >0      failed (and can abort the test suite if setup so)
    #   <0      failed but this was expected and so not fatal
    # $2+ = optional SHORT SINGLE-LINE comment about the failure
    #           if it does strike
    # For legacy purposes, "$1" can be the comment, then exit-code
    # value is hardcoded as a failure (255).
    # The absolute(!) value of the exit-code should be passed to the
    # caller as this routine's exit-code.
    if [ x"${_testlib_result_printed-}" = xyes ]; then
        logmsg_warn "print_result() called before a new test_it() was started!"
        return 0
    fi
    if [ x"${_testlib_result_printed-}" = xnotest ]; then
        logmsg_warn "print_result() called before any test_it() was started!"
        return 0
    fi

    [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && \
        TESTLIB_TIMESTAMP_TESTFINISH="`date -u +%s 2>/dev/null`" \
        || TESTLIB_TIMESTAMP_TESTFINISH=0

    TESTLIB_TESTDURATION="-1"
    if [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && \
       [ "$TESTLIB_TIMESTAMP_TESTFINISH" -gt 0 -a "$TESTLIB_TIMESTAMP_TESTSTART" -gt 0 ] 2>/dev/null \
    ; then
        if [ x"$TESTLIB_TIMESTAMP_TESTFINISH" = x"$TESTLIB_TIMESTAMP_TESTSTART" ]; then
            TESTLIB_TESTDURATION=0
        else
            TESTLIB_TESTDURATION="`expr $TESTLIB_TIMESTAMP_TESTFINISH - $TESTLIB_TIMESTAMP_TESTSTART`" && \
            [ "$TESTLIB_TESTDURATION" -ge 0 ] \
            || TESTLIB_TESTDURATION="-1"
        fi
    fi
    TESTLIB_TIMESTAMP_TESTSTART=0
    # At this point, if TESTLIB_TESTDURATION>=0 then profiling is enabled
    # and a reasonable value is available
    [ "$TESTLIB_TESTDURATION" -ge 0 ] 2>/dev/null && \
        TESTLIB_TESTDURATION_TEXT=" (took $TESTLIB_TESTDURATION seconds)" || \
        TESTLIB_TESTDURATION_TEXT=""

    if [ "$TESTLIB_TESTDURATION" -gt 0 ] 2>/dev/null ; then
        [ "$TESTLIB_TIME_INTESTS" -le 0 ] && TESTLIB_TIME_INTESTS="$TESTLIB_TESTDURATION" || \
            TESTLIB_TIME_INTESTS="`expr $TESTLIB_TIME_INTESTS + $TESTLIB_TESTDURATION`"
    fi

    _testlib_result_printed=yes
    _code="$1"
    shift
    _info="$*"
    ### Is this _code a valid number (negative == failed_ignored)?
    ### If not - it may also be some text comment about the error.
    [ -n "$_code" ] && [ "$_code" -ge 0 -o "$_code" -le 0 ] 2>/dev/null || \
        { _info="${_code} ${_info}"; _code=255 ; }

    _info="`echo "${_info}" | sed -e 's,^ *,,g' -e 's, *$,,g'`"

    # Produce a single-token name for the failed test, including its
    # positive return-code value
    if [ "$_code" -lt 0 ] 2>/dev/null ; then
        _ret="`expr -1 \* $_code`"
    else
        _ret="$_code"
    fi

    # If we have some failure-info, clamp it into the echos and tags uniformly
    [ -n "$_info" ] && [ "$_code" -ne 0 ] && \
        _report="$_ret, $_info" || \
        _report="$_ret"

    # Tags are single-token strings saved into the corresponding list
    # of passed/ignored/failed tests
    [ "$CI_DEBUG" -gt "$CI_DEBUGLEVEL_NOOP" ] && echo ""
    if [ "${TNAME-}" = "`basename $NAME .sh`" ]; then
        TESTLIB_LASTTESTTAG="`echo "$NAME(${_report})" | sed 's, ,__,g'`"
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_info "Completed test $TNAME${TESTLIB_TESTDURATION_TEXT} :"
    else
        TESTLIB_LASTTESTTAG="`echo "$NAME::$TNAME(${_report})" | sed 's, ,__,g'`"
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_info "Completed test $NAME::$TNAME${TESTLIB_TESTDURATION_TEXT} :"
    fi
    [ "$TESTLIB_TESTDURATION" -ge 0 ] 2>/dev/null && \
        TESTLIB_LASTTESTTAG="$TESTLIB_LASTTESTTAG[${TESTLIB_TESTDURATION}sec]"
        # Note: This tag is inspected in summarizeResults() so format matters!

    if [ "$_code" -eq 0 ]; then  # should include "-0" too
        logmsg_echo $CI_DEBUGLEVEL_INFO " * PASSED"
        TESTLIB_COUNT_PASS="`expr $TESTLIB_COUNT_PASS + 1`"
        [ "$TESTLIB_COUNT_PASS" -eq "$TESTLIB_COUNT_TOTAL" ] && \
            TEMP_NUMBER=0 || \
            TEMP_NUMBER="`expr $TESTLIB_COUNT_PASS - $TESTLIB_COUNT_TOTAL`"
        if [ "$TEMP_NUMBER" -gt 0 ]; then
            logmsg_error "WOW: TESTLIB_COUNT_PASS - TESTLIB_COUNT_TOTAL = $TESTLIB_COUNT_PASS - $TESTLIB_COUNT_TOTAL = $TEMP_NUMBER > 0 ! This should not happen!"
        fi
        TESTLIB_LIST_PASSED="$TESTLIB_LIST_PASSED $TESTLIB_LASTTESTTAG"
        logmsg_echo $CI_DEBUGLEVEL_INFO ""
        return 0
    else
        if [ "$_code" -lt 0 ] ; then
            # The "$1" string was a negative number
            TESTLIB_COUNT_SKIP="`expr $TESTLIB_COUNT_SKIP + 1`"
            logmsg_echo $CI_DEBUGLEVEL_INFO " * FAILED_IGNORED ($_report)"
            TESTLIB_LIST_FAILED_IGNORED="$TESTLIB_LIST_FAILED_IGNORED $TESTLIB_LASTTESTTAG"
            logmsg_echo $CI_DEBUGLEVEL_INFO ""
            return $_ret
        fi

        # Positive _code, including 255 set for anon failure with comment
        # Unlike ignored-negative retcodes above, this can abort the script
        logmsg_echo $CI_DEBUGLEVEL_ERROR " * FAILED ($_report)"

        TESTLIB_LIST_FAILED="$TESTLIB_LIST_FAILED $TESTLIB_LASTTESTTAG"
        TESTLIB_COUNT_FAIL="`expr $TESTLIB_COUNT_FAIL + 1`"

        # This optional envvar can be set by the caller
        if [ "$CITEST_QUICKFAIL" = yes ]; then
            [ "$CI_DEBUG" -gt "$CI_DEBUGLEVEL_NOOP" ] && {
            echo ""
            echo ""
            echo "################### ABORT on CITEST_QUICKFAIL #######################"
            echo ""            echo "$TESTLIB_COUNT_PASS previous tests have succeeded"
            echo "${LOGMSG_PREFIX_TESTLIB}FATAL-ABORT[$$]: Testing aborted due to" \
                "CITEST_QUICKFAIL=$CITEST_QUICKFAIL" \
                "after first failure with test $TESTLIB_LASTTESTTAG"
            echo "#####################################################################"
            echo ""
            }
            exit $_ret
        fi >&2

        # This optional envvar can be set by CURL() and trap_*() below
        if [ "$TESTLIB_FORCEABORT" = yes ]; then
            [ "$CI_DEBUG" -gt "$CI_DEBUGLEVEL_NOOP" ] && {
            echo ""
            echo ""
            echo "################### ABORT on TESTLIB_FORCEABORT #####################"
            echo "$TESTLIB_COUNT_PASS previous tests have succeeded"
            echo "${LOGMSG_PREFIX_TESTLIB}FATAL-ABORT[$$]: Testing aborted due to" \
                "TESTLIB_FORCEABORT=$TESTLIB_FORCEABORT" \
                "after forced abortion in test $TESTLIB_LASTTESTTAG"
            echo "#####################################################################"
            echo ""
            }
            exit $_ret
        fi >&2
    fi
    echo
    return $_ret
}

account_time_between() {
    # This accounts the time spent between last TESTLIB_TIMESTAMP_TESTFINISH
    # (seeded to negative value of "NOW" at the time this library is included)
    # and the "NOW" at the moment of this accounting function execution.
    _ACCT_FLAG="${1-}"     # May be "after" when used from exit-handler
    if [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && [ "$TESTLIB_TIMESTAMP_TESTFINISH" -ne 0 ]; then
        _TESTLIB_TIMESTAMP_PRESTART="`date -u +%s 2>/dev/null`" \
        || _TESTLIB_TIMESTAMP_PRESTART=0

        if [ "$TESTLIB_TIMESTAMP_TESTFINISH" -lt 0 ]; then
            [ x"${_ACCT_FLAG}" != xafter ] && _ACCT_FLAG="before"
            TESTLIB_TIMESTAMP_TESTFINISH="`expr $TESTLIB_TIMESTAMP_TESTFINISH \* -1`"
            # logmsg_debug "This is the first test in the set after inclusion of testlib.sh; usable below is the set-up overhead"
        fi

        _TESTLIB_SPENT_BETWEEN="-1"
        if [ "${_TESTLIB_TIMESTAMP_PRESTART}" -gt 0 ] 2>/dev/null \
        ; then
            if [ x"$TESTLIB_TIMESTAMP_TESTFINISH" = x"${_TESTLIB_TIMESTAMP_PRESTART}" ]; then
                _TESTLIB_SPENT_BETWEEN=0
            else
                _TESTLIB_SPENT_BETWEEN="`expr ${_TESTLIB_TIMESTAMP_PRESTART} - $TESTLIB_TIMESTAMP_TESTFINISH`" && \
                [ "${_TESTLIB_SPENT_BETWEEN}" -ge 0 ] \
                || _TESTLIB_SPENT_BETWEEN="-1"
            fi
        fi

        case "${_ACCT_FLAG}" in
            before) #[ "${_TESTLIB_SPENT_BETWEEN}" -ge "$TESTLIB_SPENT_BETWEENTESTS_REPORT" ] && \
                        logmsg_debug "${_TESTLIB_SPENT_BETWEEN} seconds were spent between inclusion of testlib.sh and first test"
                    TESTLIB_TIME_BEFORETESTS="${_TESTLIB_SPENT_BETWEEN}" ;;
            after)  #[ "${_TESTLIB_SPENT_BETWEEN}" -ge "$TESTLIB_SPENT_BETWEENTESTS_REPORT" ] && \
                        logmsg_debug "${_TESTLIB_SPENT_BETWEEN} seconds were spent between last test and this exit-handler"
                    TESTLIB_TIME_AFTERTESTS="${_TESTLIB_SPENT_BETWEEN}" ;;
            *)      [ "${_TESTLIB_SPENT_BETWEEN}" -ge "$TESTLIB_SPENT_BETWEENTESTS_REPORT" ] && \
                        logmsg_debug "${_TESTLIB_SPENT_BETWEEN} seconds were spent between now and last test"
                    if [ "${_TESTLIB_SPENT_BETWEEN}" -gt 0 ] 2>/dev/null ; then
                        [ "$TESTLIB_TIME_BETWEENTESTS" -le 0 ] && TESTLIB_TIME_BETWEENTESTS="${_TESTLIB_SPENT_BETWEEN}" || \
                            TESTLIB_TIME_BETWEENTESTS="`expr $TESTLIB_TIME_BETWEENTESTS + ${_TESTLIB_SPENT_BETWEEN}`"
                    fi
                    ;;
        esac

    fi
}

test_it() {
    if [ x"${_testlib_result_printed}" = xnotyet ]; then
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_warn "Starting a new test_it() while an old one was not followed by a print_result()!"
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_warn "Closing old test with result code 128 ..."
        print_result 128 "Automatically closed an unfinished test"
    fi

    account_time_between
    _testlib_result_printed=notyet
    [ -n "${NAME-}" ] || NAME="$0"
    [ -n "${TNAME-}" ] || TNAME="${NAME}"
    if [ "$1" ]; then
        TNAME="$1"
    fi
    [ -n "$TNAME" ] || TNAME="$0"
    TNAME="`basename "$TNAME" .sh | sed 's, ,__,g'`"
    echo
    if [ "$TNAME" = "`basename $NAME .sh`" ]; then
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_info "Running test $TNAME ..."
    else
        LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_info "Running test $NAME::$TNAME ..."
    fi
    [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && \
        TESTLIB_TIMESTAMP_TESTSTART="`date -u +%s 2>/dev/null`" \
        || TESTLIB_TIMESTAMP_TESTSTART=0
    TESTLIB_TIMESTAMP_TESTFINISH=0
    TESTLIB_COUNT_TOTAL="`expr $TESTLIB_COUNT_TOTAL + 1`"
}

### This is what we will sig-kill if needed
_PID_TESTER=$$
RES_TEST=0
trap_break_testlib() {
    ### This SIGUSR2 handler is reserved for testscript-initiated failures
#    set +e
    [ "$RES_TEST" != 0 ] && \
        echo "${LOGMSG_PREFIX_TESTLIB}ERROR-WEB: test program failed ($RES_TEST), aborting test suite" >&2
    echo "${LOGMSG_PREFIX_TESTLIB}FATAL-BREAK: Got forced interruption signal" >&2
    TESTLIB_FORCEABORT=yes

### Just cause the loop to break at a proper moment in print_result()
#    exit $1
    return 1
}
TRAP_SIGNALS=USR2 settraps "trap_break_testlib"

# If the TESTLIB_LOG_SUMMARY is set, appends the beginning of the current run.
# Note: the path will be absolutized and the variable will be exported so child
# processes can inherit and use it, and it remains valid after "cd" in scripts.
# Note: this DOES NOT set up the trap_summarizeTestlibResults() for caller!!!
init_summarizeTestlibResults() {
    # $1 - optional default value for TESTLIB_LOG_SUMMARY (if not set already)
    # $2 - optional comment about the test run being started
    if [ -z "$TESTLIB_LOG_SUMMARY" ]; then
        [ -n "${1-}" ] && TESTLIB_LOG_SUMMARY="$1"
    fi

    if [ -z "$TESTLIB_LOG_SUMMARY" ]; then
        logmsg_warn "init_summarizeTestlibResults(): called without a TESTLIB_LOG_SUMMARY value, skipped"
        return 1
    fi

    # Absolutize the path
    case "$TESTLIB_LOG_SUMMARY" in
        /*) ;;
        *) TESTLIB_LOG_SUMMARY="`pwd`/$TESTLIB_LOG_SUMMARY" ;;
    esac
    export TESTLIB_LOG_SUMMARY
    if [ -d "`dirname "$TESTLIB_LOG_SUMMARY"`" ] && [ -w "`dirname "$TESTLIB_LOG_SUMMARY"`" ] \
    ; then true ; else
        logmsg_warn "init_summarizeTestlibResults(): TESTLIB_LOG_SUMMARY='$TESTLIB_LOG_SUMMARY' is not under an existing writable directory, creating..."
        mkdir -p "`dirname "$TESTLIB_LOG_SUMMARY"`"
        chmod 775 "`dirname "$TESTLIB_LOG_SUMMARY"`"
    fi

    if [ $# -ge 2 ]; then
        TESTLIB_LOG_SUMMARY_COMMENT="${2-}"
        # Empty or not - take it as THIS test-run's comment
    else
        TESTLIB_LOG_SUMMARY_COMMENT="${TESTLIB_LOG_SUMMARY_COMMENT-}"
        # Keep the previously defined value
    fi
    export TESTLIB_LOG_SUMMARY_COMMENT

    logmsg_info "Summary of this test run will be appended to '$TESTLIB_LOG_SUMMARY'"
    { echo ""; echo "=============================================================="
      logmsg_info "`date -u`: Starting '${_SCRIPT_PATH} ${_SCRIPT_ARGS}'${TESTLIB_LOG_SUMMARY_COMMENT:+: $TESTLIB_LOG_SUMMARY_COMMENT}"; \
    } >> "$TESTLIB_LOG_SUMMARY"
    return $?
}

echo_profilingLadder() {
    # Checks whether to call this routine are on the caller
    ( IFS=" 	"; export IFS
      [ -n "$TESTLIB_LIST_PASSED" ] && for i in $TESTLIB_LIST_PASSED ; do echo "PASSED	$i" ; done
      [ -n "$TESTLIB_LIST_FAILED" ] && for i in $TESTLIB_LIST_FAILED ; do echo "FAILED	$i" ; done
      [ -n "$TESTLIB_LIST_FAILED_IGNORED" ] && for i in $TESTLIB_LIST_FAILED_IGNORED ; do echo "FAILED_IGNORED	$i" ; done
      echo "" ) | egrep '\[[0-9]+sec\][ \t]*$' | \
        sed 's,^\(.*\)\[\([0-9]*\)sec\][ \t]*$,\2\t\1,' | sort -nr
}

# This implements the summary of the test run; can be just echoed or also
# appended to the TESTLIB_LOG_SUMMARY by exit_summarizeTestlibResults()
# Uses and changes TRAP_RES defined by caller exit_summarizeTestlibResults()
TESTLIB_TIMESTAMP_SUITESTART="`date -u +%s 2>/dev/null`" \
|| TESTLIB_TIMESTAMP_SUITESTART=0
echo_summarizeTestlibResults() {
    # Do not doctor up the LOGMSG_PREFIX as these are rather results of the
    # test-script than the framework
    eTRAP_RES="${1-}"

    echo
    echo "####################################################################"
    echo "************ Ending testlib-driven suite execution *****************"
    echo "####################################################################"
    echo

    # Ensure the common whitespace IFS
    IFS=" 	"
    export IFS

    [ "$TESTLIB_TIMESTAMP_SUITESTART" -gt 0 ] 2>/dev/null && \
    TESTLIB_TIMESTAMP_SUITEFINISH="`date -u +%s 2>/dev/null`" \
    || TESTLIB_TIMESTAMP_SUITEFINISH=0

    TESTLIB_DURATION_TESTSUITE=-1
    if [ "$TESTLIB_TIMESTAMP_SUITESTART" -gt 0 ] 2>/dev/null && \
       [ "$TESTLIB_TIMESTAMP_SUITEFINISH" -gt 0 ] 2>/dev/null \
    ; then
        if [ x"$TESTLIB_TIMESTAMP_SUITEFINISH" = x"$TESTLIB_TIMESTAMP_SUITESTART" ]; then
            TESTLIB_DURATION_TESTSUITE=0
        else
            TESTLIB_DURATION_TESTSUITE="`expr $TESTLIB_TIMESTAMP_SUITEFINISH - $TESTLIB_TIMESTAMP_SUITESTART`" && \
            [ "$TESTLIB_DURATION_TESTSUITE" -ge 0 ] \
            || TESTLIB_DURATION_TESTSUITE=-1
        fi
    fi

    NUM_NOTFAILED="`expr $TESTLIB_COUNT_PASS + $TESTLIB_COUNT_SKIP`"
    logmsg_info "Testing completed ($eTRAP_RES), $NUM_NOTFAILED/$TESTLIB_COUNT_TOTAL tests passed($TESTLIB_COUNT_PASS) or not-failed($TESTLIB_COUNT_SKIP)"

    if [ -n "$TESTLIB_LIST_FAILED_IGNORED" ]; then
        logmsg_info "The following $TESTLIB_COUNT_SKIP tests have failed but were ignored (TDD in progress):"
        for i in $TESTLIB_LIST_FAILED_IGNORED; do
            echo " * $i"
        done
    fi

    NUM_FAILED="`expr $TESTLIB_COUNT_TOTAL - $NUM_NOTFAILED`"
    if [ -z "$TESTLIB_LIST_FAILED" ] && [ x"$TESTLIB_COUNT_FAIL" = x0 ] && [ x"$NUM_FAILED" = x0 ]; then
        [ -z "$eTRAP_RES" ] && eTRAP_RES=0
    else
        logmsg_info "The following $TESTLIB_COUNT_FAIL tests have failed:"
        N=0 # Do a bit of double-accounting to be sure ;)
        for i in $TESTLIB_LIST_FAILED; do
            echo " * FAILED : $i"
            N="`expr $N + 1`"
        done
        [ x"$TESTLIB_COUNT_FAIL" = x"$NUM_FAILED" ] && \
        [ x"$N" = x"$NUM_FAILED" ] && \
        [ x"$TESTLIB_COUNT_FAIL" = x"$N" ] || \
            LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_error "TEST-LIB accounting fault: failed-test counts mismatched: TESTLIB_COUNT_FAIL=$TESTLIB_COUNT_FAIL vs NUM_FAILED=$NUM_FAILED vs N=$N"
        logmsg_error "$N/$TESTLIB_COUNT_TOTAL tests FAILED, $TESTLIB_COUNT_SKIP tests FAILED_IGNORED, $TESTLIB_COUNT_PASS tests PASSED"
        if [ -n "$TESTLIB_LOG_SUMMARY" ]; then
            # Duplicate on stdout for the log file
            [ x"$TESTLIB_COUNT_FAIL" = x"$N" ] || \
                LOGMSG_PREFIX="${LOGMSG_PREFIX_TESTLIB}" logmsg_error "TEST-LIB accounting fault: failed-test counts mismatched: TESTLIB_COUNT_FAIL=$TESTLIB_COUNT_FAIL vs NUM_FAILED=$NUM_FAILED vs N=$N"
            logmsg_error "$N/$TESTLIB_COUNT_TOTAL tests FAILED, $TESTLIB_COUNT_SKIP tests FAILED_IGNORED, $TESTLIB_COUNT_PASS tests PASSED"
        fi 2>&1
        unset N

        # If we are here, we've at least had some failed tests
        [ -z "$eTRAP_RES" -o "$eTRAP_RES" = 0 ] && eTRAP_RES=1
    fi

    if [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] && [ "$TESTLIB_COUNT_TOTAL" -gt 0 ] ; then
        [ "${TESTLIB_PROFILE_TESTDURATION_TOP-}" -gt 0 ] 2>/dev/null || TESTLIB_PROFILE_TESTDURATION_TOP=10
        [ "$TESTLIB_COUNT_TOTAL" -lt "$TESTLIB_PROFILE_TESTDURATION_TOP" ] && \
                TESTLIB_PROFILE_TESTDURATION_TOP="$TESTLIB_COUNT_TOTAL"

        LADDER="`echo_profilingLadder`" && \
        LADDER_LEN="`echo "$LADDER" | wc -l`"
        if [ $? = 0 ] && [ "$LADDER_LEN" -gt 0 ]; then
            [ "$LADDER_LEN" -lt "$TESTLIB_PROFILE_TESTDURATION_TOP" ] && \
                TESTLIB_PROFILE_TESTDURATION_TOP="$LADDER_LEN"
            logmsg_info "Below are up to $TESTLIB_PROFILE_TESTDURATION_TOP longest test units (duration rounded to seconds):"
            if [ "$LADDER_LEN" -le "$TESTLIB_PROFILE_TESTDURATION_TOP" ] ; then
                echo "$LADDER"
            else
                echo "$LADDER" | head -${TESTLIB_PROFILE_TESTDURATION_TOP}
            fi
        fi
    fi

    account_time_between "after"
    if [ "$TESTLIB_DURATION_TESTSUITE" -ge 0 ] 2>/dev/null ; then
        logmsg_info "This test suite took $TESTLIB_DURATION_TESTSUITE full seconds to complete (counting from import of testlib.sh)"

        if [ "$TESTLIB_PROFILE_TESTDURATION" = yes ] ; then
            TESTLIB_DURATION_ACCOUNTED=0
            [ "$TESTLIB_TIME_BEFORETESTS" -ge 0 ] 2>/dev/null && \
                echo " ** $TESTLIB_TIME_BEFORETESTS full seconds are known to be spent before the first test set" && \
                TESTLIB_DURATION_ACCOUNTED="`expr $TESTLIB_DURATION_ACCOUNTED + $TESTLIB_TIME_BEFORETESTS`"

            [ "$TESTLIB_TIME_INTESTS" -ge 0 ] 2>/dev/null && \
                echo " ** $TESTLIB_TIME_INTESTS full seconds are known to be spent inside the test sets" && \
                TESTLIB_DURATION_ACCOUNTED="`expr $TESTLIB_DURATION_ACCOUNTED + $TESTLIB_TIME_INTESTS`"

            [ "$TESTLIB_TIME_BETWEENTESTS" -ge 0 ] 2>/dev/null && \
                echo " ** $TESTLIB_TIME_BETWEENTESTS full seconds are known to be spent between test sets" && \
                TESTLIB_DURATION_ACCOUNTED="`expr $TESTLIB_DURATION_ACCOUNTED + $TESTLIB_TIME_BETWEENTESTS`"

            [ "$TESTLIB_TIME_AFTERTESTS" -ge 0 ] 2>/dev/null && \
                echo " ** $TESTLIB_TIME_AFTERTESTS full seconds are known to be spent after the last test set up till this moment" && \
                TESTLIB_DURATION_ACCOUNTED="`expr $TESTLIB_DURATION_ACCOUNTED + $TESTLIB_TIME_AFTERTESTS`"

            echo " * ACCT: $TESTLIB_DURATION_ACCOUNTED full seconds are accounted for in detail"
        fi

        echo " * NOTE: $TESTLIB_COUNT_TOTAL tests might add up to almost as many incomplete seconds in-between accounted as zeroes"
    fi

    echo
    echo "####################################################################"
    echo "************ END OF testlib-driven suite execution ($eTRAP_RES) *************"
    echo "####################################################################"
    echo
    sleep 2

    return $eTRAP_RES
}

# A consumer script can set this as (part of) their exit/abort-trap to always
# print a summary of processed tests in the end, whatever the reason to exit().
exit_summarizeTestlibResults() {
    sTRAP_RES=$?
    # ERRCODE is maintained by testlib settraps() routines as "$?" at real
    # beginning of the (wrapped) trap processing
    [ -n "${ERRCODE-}" ] && [ "$ERRCODE" -ge 0 ] 2>/dev/null && sTRAP_RES="$ERRCODE"
    [ -n "${1-}" ] && [ "$1" -gt 0 ] && sTRAP_RES="$1"

    # No longer error out on bad lines, even if we did
    set +e
    set +u
    # This would be a no-op if the test case previously started with a
    # test_it() has been already closed with its proper print_result()
    if [ x"${_testlib_result_printed}" = xnotyet ]; then
        print_result $sTRAP_RES
    fi

    if [ -z "$TESTLIB_LOG_SUMMARY" ]; then
        echo_summarizeTestlibResults $sTRAP_RES
        exit $?
    fi

    # NOTE: There can be a bit of STDERR here
    TRAP_OUT="`echo_summarizeTestlibResults $sTRAP_RES`"
    sTRAP_RES=$?
    echo "$TRAP_OUT"

    { echo ""; echo "$TRAP_OUT"; echo "";
      logmsg_info "`date -u`: Finished '${_SCRIPT_PATH} ${_SCRIPT_ARGS}'${TESTLIB_LOG_SUMMARY_COMMENT:+: $TESTLIB_LOG_SUMMARY_COMMENT}"
      echo "=============================================================="; echo ""; echo ""; \
    } >> "$TESTLIB_LOG_SUMMARY"
    logmsg_info "Summary of this test run was appended to '$TESTLIB_LOG_SUMMARY'"
    exit $sTRAP_RES
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

:

