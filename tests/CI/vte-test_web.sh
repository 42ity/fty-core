#!/bin/bash
#
# Copyright (C) 2014-2015 Eaton
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
#! \file   vte-test_web.sh
#  \brief  REST API automated tests for the $BIOS project
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \details This script automates tests of REST API for the $BIOS project
# It works with the image installation of the BIOS on SUT (System Under Test) 
# server and is started on another server - MS (test Management Station)
#
# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** dealing with external parameter, some are mandatory. Te script must be
        # * The script must be called in the following format:
        # * test_web.sh -u "<user on SUT>" -p "<pasword>" -s "<SASL service>" \
        # * "<substring for the testcase filtering>"
   # *** no parameters? ERROR!
if [ $# -eq 0 ]; then
    echo "ERROR: vte_test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi

[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""
[ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="https"

    # *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp|-o|--port)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        --port-web|--sut-port-web|-wp)
            SUT_WEB_PORT="$2"
            shift 2
            ;;
        --host|--machine|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        --use-https|--sut-web-https)    SUT_WEB_SCHEMA="https"; export SUT_WEB_SCHEMA; shift;;
        --use-http|--sut-web-http)      SUT_WEB_SCHEMA="http"; export SUT_WEB_SCHEMA; shift;;
        --sut-user|-su)
            SUT_USER="$2"
            shift 2
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift 2
            ;;
        -p|--passwd|--bios-passwd)
            BIOS_PASSWD="$2"
            shift 2
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift 2
            ;;
        *)  # fall through - these are lists of tests to do
            break
            ;;
    esac
done

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

# unconditionally calculated values
BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

    # *** find the SCRIPTDIR (... test/CI dir) and CHECKOUTDIR
# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
# *** include weblib.sh
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"


    # *** set TESTLIB_COUNT_* to 0 and TESTLIB_LIST_* to ""
TESTLIB_COUNT_PASS=0
TESTLIB_COUNT_SKIP=0
TESTLIB_COUNT_FAIL=0
TESTLIB_COUNT_TOTAL=0
TESTLIB_LIST_FAILED=""
TESTLIB_LIST_FAILED_IGNORED=""
TESTLIB_LIST_PASSED=""

echo '*************************************************************************************************************'
logmsg_info "Will use BASE_URL = '$BASE_URL'"
echo $BIOS_USER
echo $BIOS_PASSWD
echo $SASL_SERVICE
echo $SUT_HOST
echo $SUT_SSH_PORT
echo $SUT_WEB_PORT


PATH="$PATH:/sbin:/usr/sbin"

    # *** is sasl running on SUT?
if [ "$(sut_run 'pidof saslauthd'|wc -l| sed 's, ,,g')" -gt 0 ];then
    logmsg_info "saslauthd is running"
else
    CODE=1 die "saslauthd is not running, please start it first!"
fi

# is bios user present?
# Check the user account in system
# We expect SASL uses Linux PAM, therefore getent will tell us all we need
LINE="$(sut_run "getent passwd '$BIOS_USER'")"
if [ $? != 0 -o -z "$LINE" ]; then
#if ! getent passwd "$BIOS_USER" > /dev/null; then
    logmsg_error "User $BIOS_USER is not known to system administrative" \
        "database at $SUT_HOST:$SUT_SSH_PORT." \
    logmsg_info "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
    echo "and don't forget the password '$BIOS_PASSWD'"
    CODE=2 die "BIOS_USER absent on remote system"
fi >&2

# is bios access to sasl right?
SASLTEST=$(sut_run "which testsaslauthd")
LINE="$(sut_run "$SASLTEST -u '$BIOS_USER' -p '$BIOS_PASSWD' -s '$SASL_SERVICE'")"
if [ $? != 0 -o -z "$LINE" ]; then
    CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
        "Please check the existence of /etc/pam.d/bios (and maybe" \
        "/etc/sasl2/bios.conf for some OS distributions)"
fi

if [ "$SKIP_SANITY" = yes ]; then
    # This is hit e.g. when a wget-based "curl emulator" is used for requests
    logmsg_info "$0: REST API sanity checks skipped due to SKIP_SANITY=$SKIP_SANITY"
else
    logmsg_info "Testing webserver ability to serve the REST API"
    # is web server running?

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
            # We expect that the login service responds
            logmsg_error "api_get() returned an error:"
            api_get "/admin/ifaces" >&2
            CODE=4 die "Webserver is not running or serving the REST API, please start it first!"
        fi

        TMP_TOKEN="`_api_get_token`" 
        if [ $? != 0 ] || [ -n "`echo "$TMP_TOKEN" | grep 'errors'`" ]; then
            logmsg_error "cannot get a token:"
            echo "$TMP_TOKEN" >&2
            logmsg_warn "Webserver does not allow to get the token, is the license accepted?"
        fi
        unset TMP_TOKEN
        curlfail_pop
    fi
fi

# log dir contents the real responses
cd "`dirname "$0"`" || die
[ -n "${LOG_DIR-}" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4

# cmpjson.sh compares json like files
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "${CMP-}" ] && CMP="$CMPJSON_SH"
[ -s "${CMP-}" ] || CODE=5 die "Can not use comparator '$CMP'"

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

# Note: this default log filename will be ignored if already set by caller
init_summarizeTestlibResults "${BUILDSUBDIR}/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps '_TRAP_RES=$?; exit_summarizeTestedScriptlets ; exit_summarizeTestlibResults; exit $_TRAP_RES'

# A bash-ism, should set the exitcode of the rightmost failed command
# in a pipeline, otherwise e.g. exitcode("false | true") == 0
set -o pipefail 2>/dev/null || true

# TODO: Port recent changes from main test_web.sh
# ... or merge these two via sut_run() commands etc.

for i in $POSITIVE; do
    for NAME in *$i*; do
    SKIP=""
    for n in $NEGATIVE; do
        if expr match $NAME .\*"$n".\* > /dev/null; then
            SKIP="true"
        fi
    done
    [ -z "$SKIP" ] || continue
# start testcase $NAME and put the result to $NAME.log
    . ./"$NAME" 5> "$LOG_DIR/$NAME".log
    if [ -r "../results/$NAME".res ]; then
        RESULT="../results/$NAME".res
        EXPECT="$LOG_DIR/$NAME".log
        if [ -x "../results/$NAME".cmp ]; then
            test_it "compare_expectation_custom"
            ls -la "$RESULT" "$EXPECT"
            ../results/"$NAME".cmp "$RESULT" "$EXPECT"
        else
            test_it "compare_expectation_`basename "$CMP"`"
            ls -la "$RESULT" "$EXPECT"
            "$CMP" "$RESULT" "$EXPECT"
        fi
        RES=$?
        if [ $RES -ne 0 ]; then
            diff -Naru "../results/$NAME".res "$LOG_DIR/$NAME".log
        fi
        print_result $RES
    fi
    done
done
