#!/bin/bash
#
# Copyright (C) 2015 Eaton
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
#! \file   vte-restapi.sh
#  \brief  Sets up the sandbox and runs the tests of REST API for the $BIOS project.
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT and BASE_URL should be set to values corresponded to chosen server ***
    # *** Must run as root without using password ***
    # *** BIOS image must be installed and running on SUT ***
    # *** SUT port and SUT name should be set properly (see below) ***
    # *** directory with initdb.sql load_data.sql power_topology.sql and location_topology.sql present on MS ***
    # *** tests/CI directory (on MS) contains weblib.sh (api_get_json and CURL functions needed) ***
    # *** tests/CI/web directory containing results, commands and log subdirectories with the proper content

usage(){
    echo "Usage: $(basename $0) [options...] [test_name...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
    echo "  -s|--service service for SASL/PAM (Default: '$SASL_SERVICE')"
}

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
        --user|-u|--bios-user)
            BIOS_USER="$2"
            shift 2
            ;;
        --passwd|--bios-passwd|-p|--password|--bios-password)
            BIOS_PASSWD="$2"
            shift 2
            ;;
        --service|-s)
            SASL_SERVICE="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 1
            ;;
        *)  # Assume that list of test names follows
            # (positive or negative, see test_web.sh)
            break
            ;;
    esac
done

# default values:
[ -z "$SUT_USER" ] && SUT_USER="root"
[ -z "$SUT_HOST" ] && SUT_HOST="debian.roz53.lab.etn.com"
# port used for ssh requests:
[ -z "$SUT_SSH_PORT" ] && SUT_SSH_PORT="2206"
# port used for REST API requests:
if [ -z "$SUT_WEB_PORT" ]; then
    if [ -n "$BIOS_PORT" ]; then
        SUT_WEB_PORT="$BIOS_PORT"
    else
        SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
        [ "$SUT_SSH_PORT" -ge 2200 ] && \
            SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
    fi
fi
# unconditionally calculated values
BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

# ***** SET CHECKOUTDIR *****
# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"


# ***** GLOBAL VARIABLES *****
RESULT=0

# Set up weblib test engine preference defaults for automated CI tests
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="fatal"
[ -z "$WEBLIB_CURLFAIL" ] && \
    WEBLIB_CURLFAIL=no
[ -z "$SKIP_NONSH_TESTS" ] && \
    SKIP_NONSH_TESTS=yes
[ -z "${SKIP_OBSOLETE_TESTS-}" ] && \
    SKIP_OBSOLETE_TESTS=yes
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_CURLFAIL SKIP_NONSH_TESTS SKIP_OBSOLETE_TESTS

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

logmsg_info "Will use BASE_URL = '$BASE_URL'"

# TODO. TOHLE PREDELAT, ZATIM MOZNO VYNECHAT
# zacatek vynechavky ********************************
if [ 1 = 2 ]; then
# NETSTAT ZAVOLAT PRES SSH
# KONTROLOVAT PORT 80 PROCESS TNTNET A STAV LISTEN
test_web_port() {
    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}
fi
# konec vynechavky **********************************

# ***** FUNCTIONS *****
    # *** starting the testcases
test_web() {
    echo "==== Calling vte-test_web.sh ==============================="
    RES_TW=0
    test_it "vte-test-restapi::test_web::$@"
    /bin/bash "${CHECKOUTDIR}"/tests/CI/vte-test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD" \
        -s "$SASL_SERVICE" -sh "$SUT_HOST" -su "$SUT_USER" -sp "$SUT_SSH_PORT" "$@" || \
        RES_TW=$?
    print_result $RES_TW
    echo "==== test_web RESULT: ($RES_TW) =================================="
    echo ""
    echo ""
    return $RES_TW
}

    # *** start the default set of TC
test_web_default() {
    init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "test_web_default() $*" || true
    test_it "init_script_default"
    init_script_default
    print_result $? && \
    test_web "$@" || return $?
    return 0
}

    # *** start the power topology set of TC
test_web_topo_p() {
    init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "test_web_topo_p() $*" || true
    test_it "init_script_topo_pow"
    echo "----------- reset db: topology : power -----------"
    init_script_topo_pow
    print_result $? && \
    test_web "$@" || return $?
    return 0

}

    # *** start the location topology set of TC
test_web_topo_l() {
    init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "test_web_topo_l() $*" || true
    test_it "init_script_topo_loc"
    echo "---------- reset db: topology : location ---------"
    init_script_topo_loc
    print_result $? && \
    test_web "$@" || return $?
    return 0
}

test_web_asset_create() {
    init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "test_web_asset_create() $*" || true
    test_it "init_script_sampledata"
    echo "---------- reset db: asset : create ---------"
    init_script_sampledata
    print_result $? && \
    test_web "$@" || return $?
    return 0
}

test_web_averages() {
    init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "test_web_averages() $*" || true

    test_it "generate_averages"
    echo "----------- Re-generating averages sql files -----------"
    CI_TEST_AVERAGES_DATA="`$DB_LOADDIR/generate_averages.sh "$DB_LOADDIR"`"
    print_result $?
    export CI_TEST_AVERAGES_DATA

    test_it "init_script_averages"
    echo "----------- reset db: averages -----------"
    init_script_averages
    print_result $? && \
    test_web "$@" || return $?
    return 0
}

RESULT_OVERALL=0
trap_cleanup(){
    cleanTRAP_RES="${1-}"
    logmsg_debug "trap_cleanup(): initial cleanTRAP_RES='$cleanTRAP_RES' RESULT_OVERALL='$RESULT_OVERALL'"
    [ -n "$cleanTRAP_RES" ] || cleanTRAP_RES=0
    [ "$cleanTRAP_RES" = 0 ] && [ "$RESULT_OVERALL" != 0 ] && cleanTRAP_RES="$RESULT_OVERALL"
    [ "$cleanTRAP_RES" != 0 ] && [ "$RESULT_OVERALL" = 0 ] && RESULT_OVERALL="$cleanTRAP_RES"

    init_script_default || cleanTRAP_RES=$?
    # ***** RESULTS *****
    if [ "$RESULT_OVERALL" = 0 ]; then
        logmsg_info "Overall test suite result: SUCCESS"
        if [ -n "$TESTLIB_LOG_SUMMARY" ] ; then
            { logmsg_info "`date -u`: Finished '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' test suite: SUCCESS"; \
              echo ""; echo ""; } >> "$TESTLIB_LOG_SUMMARY"
        fi
    else
        logmsg_error "Overall test suite result: FAILED ($RESULT_OVERALL), seek details above"
        if [ -n "$TESTLIB_LOG_SUMMARY" ] ; then
            { logmsg_error "`date -u`: Finished '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' test suite: FAILED ($RESULT_OVERALL)"; \
              echo ""; echo ""; } >> "$TESTLIB_LOG_SUMMARY" 2>&1
        fi
    fi

    if [ "$cleanTRAP_RES" = 0 ]; then
        logmsg_info "Overall test-suite script result (including cleanup): SUCCESS"
        if [ -n "$TESTLIB_LOG_SUMMARY" ] ; then
            { logmsg_info "`date -u`: Finished and cleaned up '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' test-suite script: SUCCESS"; \
              echo ""; echo ""; } >> "$TESTLIB_LOG_SUMMARY"
        fi
    else
        logmsg_error "Overall test-suite script result (including cleanup): FAILED ($cleanTRAP_RES) seek details above"
        if [ -n "$TESTLIB_LOG_SUMMARY" ] ; then
            { logmsg_error "`date -u`: Finished and cleaned up '${_SCRIPT_NAME} ${_SCRIPT_ARGS}' test-suite script: FAILED ($cleanTRAP_RES)"; \
          echo ""; echo ""; } >> "$TESTLIB_LOG_SUMMARY" 2>&1
        fi
    fi

    (exit_summarizeTestlibResults) || true

    if [ -n "$TESTLIB_LOG_SUMMARY" ] && [ -s "$TESTLIB_LOG_SUMMARY" ]; then
        echo ""
        echo "================================================================"
        echo ""
        echo "###########################################################"
        echo "############### TESTLIB_LOG_SUMMARY contents: #############"
        echo "### ($TESTLIB_LOG_SUMMARY) ###"
        echo "###########################################################"
        awk '{print "|| "$0}' < "$TESTLIB_LOG_SUMMARY"
        echo "###########################################################"
        echo "########### END OF TESTLIB_LOG_SUMMARY contents ###########"
        echo "###########################################################"
    fi

    logmsg_debug "trap_cleanup(): final cleanTRAP_RES='$cleanTRAP_RES' RESULT_OVERALL='$RESULT_OVERALL'"
    return $cleanTRAP_RES
}

# ************** Prepare the test suite ***
set -u
#set -e

# Ensure that no processes remain dangling when test completes
# The ERRCODE is defined by settraps() as the program exitcode
# as it enters the trap
settraps '[ "$ERRCODE" = 0 ] && ERRCODE=123; trap_cleanup $ERRCODE'
TRAP_SIGNALS=EXIT settraps 'trap_cleanup $ERRCODE'

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-cm bios-agent-dbstore bios-agent-nut bios-agent-inventory ; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-cm bios-agent-dbstore bios-agent-nut bios-agent-inventory ; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

# ***** AUTHENTICATION ISSUES *****
# check SASL is working
logmsg_info "Checking remote SASL Auth Daemon"
sut_run "testsaslauthd -u '$BIOS_USER' -p '$BIOS_PASSWD' -s '$SASL_SERVICE'" && \
  logmsg_info "saslauthd is responsive and configured well!" || \
  logmsg_error "saslauthd is NOT responsive or not configured!" >&2


[ x"${SKIP_LICENSE_FORCEACCEPT-}" = xyes ] && \
logmsg_warn "SKIP_LICENSE_FORCEACCEPT=$SKIP_LICENSE_FORCEACCEPT so not running '00_license-CI-forceaccept.sh.test' first" || \
case "$*" in
    *license*) # We are specifically testing license stuff
        logmsg_warn "The tests requested on command line explicitly include 'license', so $0 will not interfere by running '00_license-CI-forceaccept.sh.test' first"
        ;;
    *) # Try to accept the BIOS license on server
        init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" "00_license-CI-forceaccept"
        SKIP_SANITY=yes WEBLIB_CURLFAIL=no CITEST_QUICKFAIL=no WEBLIB_QUICKFAIL=no test_web 00_license-CI-forceaccept.sh.test || \
            if [ x"$CITEST_QUICKFAIL" = xyes ] || [ x"$WEBLIB_QUICKFAIL" = xyes ] ; then
                die "BIOS license not accepted on the server, subsequent tests will fail"
            else
                logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"
            fi
        ;;
esac

# ***** PERFORM THE TESTCASES *****
set +e
ONLYNEG=yes # Also yes for zero args
[ $# -gt 0 ] && \
for ARG in "$@" ; do
    case "$ARG" in
        -*) ;; # Still a negative argument
        *) ONLYNEG=no;;
    esac
done


# do the test
set +e
if [ $ONLYNEG = yes ]; then
    # *** start the default TC's instead of subsequent topology tests
    test_web_default -topology_power -topology_location -asset_create -averages "$@" || RESULT_OVERALL=$?
    [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
        CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"

    if [ "$SKIP_OBSOLETE_TESTS" = no ]; then
	    # *** start the asset_create TC's
        if [[ "$*" =~ \-asset_create ]]; then
            logmsg_info "SKIPPED special test by request: asset_create"
        else
		    test_web_asset_create asset_create || RESULT_OVERALL=$?
		    [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
		        CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"
		fi

	    # *** start power topology TC's
        if [[ "$*" =~ \-topology_power ]] || [[ "$*" =~ \-topology ]]; then
            logmsg_info "SKIPPED special test by request: topology_power"
        else
		    test_web_topo_p topology_power || RESULT_OVERALL=$?
		    [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
		        CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"
		fi

	    # *** start location topology TC's
        if [[ "$*" =~ \-topology_power ]] || [[ "$*" =~ \-topology ]]; then
            logmsg_info "SKIPPED special test by request: topology_power"
        else
		    test_web_topo_l topology_location || RESULT_OVERALL=$?
		    [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
		        CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"
		fi

        if [[ "$*" =~ \-averages ]] ; then
            logmsg_info "SKIPPED special test by request: averages"
        else
		    test_web_averages "$@" averages || RESULT_OVERALL=$?
		    [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
		        CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"
		fi
	fi
else
    # selective test routine
    # Note we still support here the obsoleted tests,
    # just in case someone runs them explicitly
    while [ $# -gt 0 ]; do
        case "$1" in
            topology_power*)
                test_web_topo_p "$1" || \
                RESULT_OVERALL=$? ;;
            topology_location*)
                test_web_topo_l "$1" || \
                RESULT_OVERALL=$? ;;
            asset_create*)
                test_web_asset_create "$1" || \
                RESULT_OVERALL=$? ;;
            averages*)
                test_web_averages "$1" || \
                RESULT_OVERALL=$? ;;
            *)  test_web_default "$1" || \
                RESULT_OVERALL=$? ;;
        esac
        shift
        [ "$RESULT_OVERALL" != 0 ] && [ x"$CITEST_QUICKFAIL" = xyes ] && \
            CODE=$RESULT_OVERALL die "Quickly aborting the test suite after failure, as requested"
    done
fi

# trap_cleanup() should handle the cleanup and final logging
logmsg_debug "Got to the end of $0 with RESULT_OVERALL='$RESULT_OVERALL'"
exit $RESULT_OVERALL
