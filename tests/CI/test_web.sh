#!/bin/bash

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
#            Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: This script automates tests of REST API for the $BIOS project

if [ $# -eq 0 ]; then
    echo "ERROR: test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi

PASS=0
TOTAL=0
FAILED=""

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

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
#cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

while [ $# -gt 0 ]; do
    case "$1" in
    -u|--user)
        BIOS_USER="$2"
        shift 2
        ;;
    -p|--passwd)
        BIOS_PASSWD="$2"
        shift 2
        ;;
    *)
        break
        ;;
    esac
done

[ -n "$BIOS_USER"   ] || BIOS_USER="bios"
[ -n "$BIOS_PASSWD" ] || BIOS_PASSWD="@PASSWORD@"

PATH="$PATH:/sbin:/usr/sbin"

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

if ! $SASLTEST -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios > /dev/null; then
    CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
        "Check the existence of /etc/pam.d/bios (and maybe /etc/sasl2/bios.conf for some OS distributions)"
fi

if [ "$SKIP_SANITY" = yes ]; then
    # This is hit e.g. when a wget-based "curl emulator" is used for requests
    logmsg_info "$0: REST API sanity checks skipped due to SKIP_SANITY=$SKIP_SANITY"
else
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

cd web/commands || CODE=6 die "Can not change to `pwd`/web/commands"

summarizeResults() {
    set +e
    logmsg_info "Testing completed, $PASS/$TOTAL tests passed for groups:"
    logmsg_info "  POSITIVE = $POSITIVE"
    logmsg_info "  NEGATIVE = $NEGATIVE"
    logmsg_info "  SKIP_NONSH_TESTS = $SKIP_NONSH_TESTS (so skipped $SKIPPED_NONSH_TESTS tests)"
    [ -z "$FAILED" ] && exit 0

    logmsg_info "The following tests have failed:"
    for i in $FAILED; do
        echo " * $i"
    done
    logmsg_error "`expr $TOTAL - $PASS`/$TOTAL tests FAILED"
    exit 1
}

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

settraps "summarizeResults"

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

    _weblib_result_printed=notest
    . ./"$NAME" 5> "$REALLIFE_RESULT"
    RES=$?

    [ "$_weblib_result_printed" = notest ] && \
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
        ls -la "$EXPECTED_RESULT" "$REALLIFE_RESULT"
        if [ -x "../results/$NAME".cmp ]; then
            ### Use an optional custom comparator
            test_it "compare_expectation_custom"
            ../results/"$NAME".cmp "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            RES=$?
        else
            ### Use the default comparation script which makes sure that
            ### each line of RESULT matches the same-numbered line of EXPECT
            test_it "compare_expectation_`basename "$CMP"`"
            "$CMP" "$EXPECTED_RESULT" "$REALLIFE_RESULT"
            RES=$?
        fi
        if [ $RES -ne 0 ]; then
            diff -Naru "$EXPECTED_RESULT" "$REALLIFE_RESULT"
        fi
        print_result $RES
    else
        # This might do nothing, if the test file already ended with a print_result
        if [ "$_weblib_result_printed" != yes ]; then
        logmsg_info "No expected-results file was found for test script '$NAME'," \
            "so nothing to compare real-life output against. Note that the result" \
            "below ($RES) may refer to execution of the test script itself and" \
            "recording the log-files of its output:"
        print_result $RES
        echo ""
        fi
    fi
    done
done

