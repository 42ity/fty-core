#!/bin/bash
#
# Copyright (C) 2014-2016 Eaton
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
#! \file   test_web.sh aliased as vte-test_web.sh
#  \brief  This script automates tests of REST API for the $BIOS project
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \details This script automates tests of REST API for the $BIOS project
# In CI (local) mode it works with the image installation of the BIOS on the
# local machine (CI container, developer workstation) and is started from a
# checked-out copy of the bios-core development workspace.
# In VTE (remote) mode it works with the image installation of the BIOS on the
# SUT (System Under Test) server and tests are started from another server - 
# MS (test Management Station) from a checked-out copy of the bios-core
# development workspace.
#
# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** dealing with external parameter, some are mandatory.
    # *** The script must be called in the following format:
        # * test_web.sh -u "<user on SUT>" -p "<pasword>" -s "<SASL service>" \
        # * "<substring for the testcase filtering>"
   # *** no parameters? ERROR!
if [ $# -eq 0 ]; then
    echo "ERROR: (vte-)test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi

case "`basename "$0"`" in
	vte*) SUT_IS_REMOTE=yes ;; # Unconditionally remote
	*) [ -z "${SUT_IS_REMOTE-}" ] && SUT_IS_REMOTE="" ;;
		  # Determine default SUT_IS_REMOTE when we include script libs
		  # or assign SUT-related CLI settings
esac

    # *** set TESTLIB_COUNT_* to 0 and TESTLIB_LIST_* to ""
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
[ x"${SKIP_SANITY-}" = x- ] && SKIP_SANITY=""
[ -z "$SKIP_SANITY" ] && SKIP_SANITY=no

# *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp)
            SUT_SSH_PORT="$2"
            SUT_IS_REMOTE=yes
            shift
            ;;
        --port-web|--sut-port-web|-wp)
            SUT_WEB_PORT="$2"
            shift
            ;;
        --host|--machine|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift
            ;;
        --use-https|--sut-web-https)	SUT_WEB_SCHEMA="https"; export SUT_WEB_SCHEMA;;
        --use-http|--sut-web-http)  	SUT_WEB_SCHEMA="http"; export SUT_WEB_SCHEMA;;
        --sut-user|-su)
            SUT_USER="$2"
            SUT_IS_REMOTE=yes
            shift
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift
            ;;
        -p|--passwd|--bios-passwd|--password|--bios-password)
            BIOS_PASSWD="$2"
            shift
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift
            ;;
        # TODO: remove -q as it is misleading as -q is usually quiet
        -q|--quick|-f|--force) SKIP_SANITY=yes ;;
        *)  # fall through - these are lists of tests to do
            break
            ;;
    esac
    shift
done

[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""
# *** default connection parameters values:
case "${SUT_IS_REMOTE}" in
no)
        [ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="http"
        ;;
yes)
        # default values:
        [ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="https"
        # default values:
        [ -z "${SUT_USER-}" ] && SUT_USER="root"
        [ -z "${SUT_HOST-}" ] && SUT_HOST="debian.roz53.lab.etn.com"
        # port used for ssh requests:
        [ -z "${SUT_SSH_PORT-}" ] && SUT_SSH_PORT="2206"
        # port used for REST API requests:
        if [ -z "${SUT_WEB_PORT-}" ]; then
                if [ -n "${BIOS_PORT-}" ]; then
                        SUT_WEB_PORT="$BIOS_PORT"
                else
                        SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
                        [ "${SUT_SSH_PORT-}" -ge 2200 ] && \
                                SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
                fi
        fi
        # unconditionally calculated values for current setup
        BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
#auto|""|*) ;; ### Defaulted in the script libraries below
esac

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

echo '************************************************************************'
logmsg_info "REST API test-case scriptlets will use the following settings:"
echo "BASE_URL   	= $BASE_URL"
echo "SUT_IS_REMOTE	= $SUT_IS_REMOTE"
echo "BIOS_USER 	= $BIOS_USER"
echo "BIOS_PASSWD	= $BIOS_PASSWD"
echo "SASL_SERVICE	= $SASL_SERVICE"
echo "SUT_HOST  	= $SUT_HOST"
echo "SUT_SSH_PORT	= $SUT_SSH_PORT"
echo "SUT_WEB_PORT	= $SUT_WEB_PORT"
echo "SUT_WEB_SCHEMA	= $SUT_WEB_SCHEMA"

PATH="$PATH:/sbin:/usr/sbin"

# Note: this default log filename will be ignored if already set by caller
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
# NOTE: This is the initial trap for sanity checks etc;
# we override it for test results (if we get so far) below
settraps 'exit_summarizeTestlibResults'

if [ "$SKIP_SANITY" = yes ]; then
    # This is hit e.g. when a wget-based "curl emulator" is used for requests
    logmsg_info "$0: REST API sanity checks skipped due to SKIP_SANITY=$SKIP_SANITY"
else
    # *** is sasl running on SUT?
    if ! sut_run "pidof saslauthd > /dev/null" ; then
        CODE=1 die "saslauthd is not running (on SUT), please start it first!"
    fi

    if ! sut_run "pidof malamute > /dev/null"; then
        logmsg_error "malamute is not running (on SUT), you may need to start it first!"
    fi

    if ! sut_run "pidof mysqld > /dev/null" ; then
        logmsg_error "mysqld is not running (on SUT), you may need to start it first!"
    fi

    # is bios user present?
    # Check the user account in system
    # We expect SASL uses Linux PAM, therefore getent will tell us all we need
    LINE="$(sut_run "getent passwd '$BIOS_USER'")"
    if [ $? != 0 -o -z "$LINE" ]; then
        logmsg_error "User $BIOS_USER is not known to system administrative database"
        isRemoteSUT && \
            echo "at $SUT_HOST:$SUT_SSH_PORT." || \
            echo "at the local system."
        logmsg_info "To add it locally on the SUT, run: "
        echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
        echo "and don't forget the password '$BIOS_PASSWD'"
        CODE=2 die "BIOS_USER absent on system under test"
    fi >&2

    # is bios access to sasl correct?
    SASLTEST=$(sut_run 'SASLTEST="`which testsaslauthd`" && [ -n "$SASLTEST" ] && [ -x "$SASLTEST" ] || SASLTEST=""; if [ -z "$SASLTEST" ]; then for S in /usr/sbin/testsaslauthd /sbin/testsaslauthd; do [ -x "$S" ] && SASLTEST="$S"; break; done; fi; echo "$SASLTEST"; [ -n "$SASLTEST" ]') && \
    LINE="$(sut_run "$SASLTEST -u '$BIOS_USER' -p '$BIOS_PASSWD' -s '$SASL_SERVICE'")"
    if [ $? != 0 -o -z "$LINE" ]; then
        CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
            "Please check the existence of /etc/pam.d/bios (and maybe" \
            "/etc/sasl2/bios.conf for some OS distributions)"
    fi

    logmsg_info "Testing webserver ability to serve the REST API"

    curlfail_push_expect_404
    if [ -n "`api_get "" 2>&1 | grep 'HTTP/.* 500'`" ] >/dev/null 2>&1 ; then
        logmsg_error "api_get() returned an Internal Server Error:"
        api_get "" >&2
        CODE=4 die "Webserver code is deeply broken (maybe missing libraries), please fix it first!"
    fi

    if [ -z "`api_get "" 2>&1 | grep 'HTTP/.* 404 Not Found'`" ] >/dev/null 2>&1 ; then
        # We do expect an HTTP-404 on the API base URL
        logmsg_error "api_get() returned an error:"
        api_get "" >&2
        CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
    fi
    curlfail_pop

    if [ "$SKIP_SANITY" != onlyerrors ]; then
        curlfail_push_expect_noerrors
        if [ -z "`api_get '/admin/ifaces' 2>&1 | grep 'HTTP/.* 200 OK'`" ] >/dev/null 2>&1 ; then
            # We expect that this simple service responds
            logmsg_error "api_get() returned an error:"
            api_get "/admin/ifaces" >&2
            CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
        fi
        curlfail_pop

        TMP_TOKEN="`_api_get_token`"
        if [ $? != 0 ] || [ -n "`echo "$TMP_TOKEN" | grep 'errors'`" ]; then
            logmsg_error "cannot get a token:"
            echo "$TMP_TOKEN" >&2
            logmsg_warn "Webserver does not allow to get the token, is the license accepted?"
        fi
        unset TMP_TOKEN
    fi

    logmsg_info "Webserver seems basically able to serve the REST API"
fi

# log dir contents the real responses
cd "`dirname "$0"`" || die "Can not run from directory '`dirname "$0"`'"
[ -n "${LOG_DIR-}" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || CODE=4 die "Can not create log directory for tests '$LOG_DIR'"

# cmpjson.sh compares json like files
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "${CMP-}" ] && CMP="$CMPJSON_SH"
[ -s "${CMP-}" ] || CODE=5 die "Can not use comparator '$CMP'"

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done
[ -s "$JSONSH" ] || CODE=7 die "Can not find JSON.sh"
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

# web/commands dir contains the request commands
cd web/commands || CODE=6 die "Can not change to `pwd`/web/commands"

# positive parameters are included to test, negative excluded
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

# if POSITIVE parameters variable is empty, then all tests are included
[ -n "$POSITIVE" ] || POSITIVE="*"

echo_summarizeTestedScriptlets() {
    logmsg_info "This ${_SCRIPT_NAME} ${_SCRIPT_ARGS} run selected the following scriptlets from web/commands :"
    logmsg_info "  Execution pattern (POSITIVE) = $POSITIVE"
    logmsg_info "  Ignored pattern (NEGATIVE)   = $NEGATIVE"
    logmsg_info "  SKIP_NONSH_TESTS = $SKIP_NONSH_TESTS (so skipped ${SKIPPED_NONSH_TESTS+0} tests)"
}

exit_summarizeTestedScriptlets() {
    echo_summarizeTestedScriptlets
    if [ -n "$TESTLIB_LOG_SUMMARY" ]; then
        echo_summarizeTestedScriptlets >> "$TESTLIB_LOG_SUMMARY"
    fi
    return 0
}

# ERRCODE is maintained by settraps()
settraps 'exit_summarizeTestedScriptlets ; exit_summarizeTestlibResults $ERRCODE'

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

    # start testcase $NAME and put the result to $NAME.log
    STACKED_HTTPERRORS_COUNT_BEFORE="${STACKED_HTTPERRORS_COUNT-}"
    . ./"$NAME" 5>"$REALLIFE_RESULT"
    RES=$?

    test_it "compare_curlfail_stackdepth"
    [ "$STACKED_HTTPERRORS_COUNT_BEFORE" = "${STACKED_HTTPERRORS_COUNT-}" ]
    print_result $? "Had STACKED_HTTPERRORS_COUNT='$STACKED_HTTPERRORS_COUNT_BEFORE' before scriptlet '$NAME', got '${STACKED_HTTPERRORS_COUNT-}' after the test"

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
                OUT_JSONV="`echo "$line" | jsonsh_cli 2>&1`"
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

