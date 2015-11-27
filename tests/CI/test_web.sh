#!/bin/bash
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
#! \file   test_web.sh
#  \brief  This script automates tests of REST API for the $BIOS project
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

if [ $# -eq 0 ]; then
    echo "ERROR: test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi

TESTLIB_COUNT_PASS=0
TESTLIB_COUNT_SKIP=0
TESTLIB_COUNT_FAIL=0
TESTLIB_COUNT_TOTAL=0
TESTLIB_LIST_FAILED=""
TESTLIB_LIST_FAILED_IGNORED=""
TESTLIB_LIST_PASSED=""

# There is a logic below that selects only *.sh filenames as eligible for testing
# If this value is not "yes" then any filenames which match the requested POSITIVE
# pattern will be permitted as test contents.
[ -z "$SKIP_NONSH_TESTS" ] && SKIP_NONSH_TESTS=yes
SKIPPED_NONSH_TESTS=0

# SKIP_SANITY=(yes|no|onlyerrors)
#   yes = skip sanity tests in ultimate request/test scripts
#   no  = do all tests
#   onlyerrors = do only tests expected to fail (not for curlbbwget.sh)
[ -z "$SKIP_SANITY" ] && SKIP_SANITY=no

while [ $# -gt 0 ]; do
    case "$1" in
        --port-web|--sut-port-web|-wp|--port)
            SUT_WEB_PORT="$2"
            shift
            ;;
        --host|--machine|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift
            ;;
        -p|--passwd|--bios-passwd)
            BIOS_PASSWD="$2"
            shift
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift
            ;;
        # TODO: remove -q as it is misleading as -q is usually quite
        -q|--quick|-f|--force) SKIP_SANITY=yes ;;
        *)  # fall through - these are lists of tests to do
            break
            ;;
    esac
    shift
done

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"

PATH="$PATH:/sbin:/usr/sbin"

if [ "$SKIP_SANITY" = yes ]; then
    # This is hit e.g. when a wget-based "curl emulator" is used for requests
    logmsg_info "$0: REST API sanity checks skipped due to SKIP_SANITY=$SKIP_SANITY"
else
    # fixture ini
    if ! pidof saslauthd > /dev/null; then
        CODE=1 die "saslauthd is not running, please start it first!"
    fi

    if ! pidof malamute > /dev/null; then
        logmsg_error "malamute is not running (locally), you may need to start it first!"
    fi

    if ! pidof mysqld > /dev/null ; then
        logmsg_error "mysqld is not running (locally), you may need to start it first!"
    fi

    # Check the user account in system
    # We expect SASL uses Linux PAM, therefore getent will tell us all we need
    if ! getent passwd "$BIOS_USER" > /dev/null; then
        CODE=2 die "User $BIOS_USER is not known to system administrative database" \
            "To add it locally, run: " \
            "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER" \
            "and don't forget the password '$BIOS_PASSWD'"
    fi

    SASLTEST="`which testsaslauthd`"
    [ -x "$SASLTEST" ] || SASLTEST="/usr/sbin/testsaslauthd"
    [ -x "$SASLTEST" ] || SASLTEST="/sbin/testsaslauthd"

    $SASLTEST -u "$BIOS_USER" -p "$BIOS_PASSWD" -s "$SASL_SERVICE" > /dev/null || \
        CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
            "Check the existence of /etc/pam.d/bios (and maybe /etc/sasl2/bios.conf for some OS distributions)"

    logmsg_info "Testing webserver ability to serve the REST API"
    if [ -n "`api_get "/oauth2/token" 2>&1 | grep 'HTTP/.* 500'`" ]; then
        logmsg_error "api_get() returned an error:"
        api_get "" >&2
        CODE=4 die "Webserver code is deeply broken, please fix it first!"
    fi

    if [ -z "`api_get "/oauth2/token" 2>&1 | grep 'HTTP/.* 200 OK'`" ]; then
        # We do expect an HTTP-404 on the API base URL
        logmsg_error "api_get() returned an error:"
        api_get "" >&2
        CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
    fi

    if [ "$SKIP_SANITY" != onlyerrors ]; then
        curlfail_push_expect_noerrors
        if [ -z "`api_get '/oauth2/token' 2>&1 | grep 'HTTP/.* 200 OK'`" ]; then
            # We expect that the login service responds
            logmsg_error "api_get() returned an error:"
            api_get "/oauth2/token" >&2
            CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
        fi
        curlfail_pop
    fi

    logmsg_info "Webserver seems basically able to serve the REST API"
fi

cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || CODE=4 die "Can not create log directory for tests $LOG_DIR"
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "$CMP" ] && CMP="$CMPJSON_SH"
[ -s "$CMP" ] || CODE=5 die "Can not use comparator '$CMP'"

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done
[ -s "$JSONSH" ] || CODE=7 die "Can not find JSON.sh"

cd web/commands || CODE=6 die "Can not change to `pwd`/web/commands"

POSITIVE=""
NEGATIVE=""
while [ "$1" ]; do
    if [ -z "`echo "x$1" | grep "^x-"`" ]; then
        POSITIVE="$POSITIVE $1"
    else
        NEGATIVE="$NEGATIVE `echo "x$1" | sed 's|^x-||'`"
    fi
    shift
done
[ -n "$POSITIVE" ] || POSITIVE="*"

exit_summarizeTestedScriptlets() {
    logmsg_info "This $0 $* run selected the following scriptlets from web/commands :"
    logmsg_info "  Execution pattern (POSITIVE) = $POSITIVE"
    logmsg_info "  Ignored pattern (NEGATIVE)   = $NEGATIVE"
    logmsg_info "  SKIP_NONSH_TESTS = $SKIP_NONSH_TESTS (so skipped ${SKIPPED_NONSH_TESTS+0} tests)"
}

settraps '_TRAP_RES=$?; exit_summarizeTestedScriptlets ; exit_summarizeTestlibResults; exit $_TRAP_RES'

for i in $POSITIVE; do
    for NAME in *$i*; do
    SKIP=""
    for n in $NEGATIVE; do
        if expr match $NAME .\*"$n".\* > /dev/null; then
            SKIP="true"
        fi
    done
    [ -z "$SKIP" -a x"$SKIP_NONSH_TESTS" = xyes ] && \
    case "$NAME" in
        *.sh)   ;;      # OK to proceed
        *)  [ "$POSITIVE" = '*' ] && SKIP=true || \
            case "$i" in
                *\**|*\?*) # Wildcards are not good
                    SKIP="true" ;;
                "$NAME") logmsg_warn "Non-'.sh' test file executed due to explicit request: '$NAME' (matched for '$i')" ;;
                *)  SKIP="true" ;;
            esac
            if [ "$SKIP" = true ]; then
                logmsg_warn "Non-'.sh' test file ignored: '$NAME' (matched for '$i')"
                echo ""
                SKIPPED_NONSH_TESTS=$(($SKIPPED_NONSH_TESTS+1))
            fi
            sleep 3
            ;;
    esac
    [ -z "$SKIP" ] || continue

    EXPECTED_RESULT="../results/$NAME".res
    REALLIFE_RESULT="$LOG_DIR/$NAME".log
    case "$NAME" in
        *.sh) ;;
        *.sh.*) # If we process the renamed tests, consider original result-names
            if [ ! -f "$EXPECTED_RESULT" ]; then
                SHORTNAME="`echo "$NAME" | sed 's/^\(.*\.sh\).*$/\1/'`" || \
                    SHORTNAME="$NAME"
                [ -f "../results/$SHORTNAME.res" ] && \
                    EXPECTED_RESULT="../results/$SHORTNAME.res"
            fi
            ;;
    esac

    ### Default value for logging the test items
    TNAME="$NAME"

    # "Poison"-protection about unused standard infrastructure aka test_it()
    _testlib_result_printed=notest
    . ./"$NAME" 5>"$REALLIFE_RESULT"
    RES=$?

    [ "${_testlib_result_printed}" = notest ] && \
        logmsg_error "NOTE: Previous test(s) apparently did not use test_it()" \
            "to begin logging, amending that omission now by assigning filename" \
            "as the test name:" && \
        test_it "$NAME"

    # Stash the last result-code for trivial tests and no expectations below
    # For better reliability, all test files should call print_result to verify
    # the basic test commands, because here we essentially get result of inclusion
    # itself (which is usually "true") and of redirection into the logfile.
    # Only if the last line of the test file was a failuer (e.g. it only contained
    # a couple of lines "test_it" and "api_get ..." and that failed) will we see a
    # real test failure here.

    if [ -r "$EXPECTED_RESULT" ]; then
        if [ -x "../results/$NAME".cmp ]; then
            ### Use an optional custom comparator
            test_it "compare_expectation_custom"
            ls -la "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            ../results/"$NAME".cmp "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            RES=$?
            [ $RES -ne 0 ] && \
                diff -Naru "$EXPECTED_RESULT" "$REALLIFE_RESULT"
        else
            ### Use the default comparation script which makes sure that
            ### each line of RESULT matches the same-numbered line of EXPECT
            test_it "compare_expectation_`basename "$CMP"`"
            ls -la "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            "$CMP" "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            RES_CMP=$?
            RES_JSONV=0
            while IFS='' read -r line || [ -n "$line" ]; do
                OUT_JSONV="`echo "$line" | "$JSONSH" 2>&1`"
                RES_JSONV=$?
                if [ $RES_JSONV -ne 0 ]; then
                    echo "$OUT_JSONV"
                    break
                fi
            done < "$REALLIFE_RESULT"

            if [ $RES_CMP -eq 0 ] && [ $RES_JSONV -eq 0 ]; then
                RES=$RES_CMP
            elif [ $RES_CMP -ne 0 ]; then
                RES=$RES_CMP
                diff -Naru "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            elif [ $RES_JSONV -ne 0 ]; then
                RES=$RES_JSONV
                logmsg_error "INVALID JSON!" 
            fi
        fi
        print_result $RES
    else
        # This might do nothing, if the test file already ended with a print_result
        if [ "${_testlib_result_printed}" != yes ]; then
        logmsg_info "No expected-results file was found for test script '$NAME'," \
            "so nothing to compare real-life output against. Note that the result" \
            "below ($RES) may refer to execution of the test script itself and" \
            "recording the log-files of its output, rather than failure of a test:"
        print_result $RES
        echo ""
        fi
    fi
    done
done

