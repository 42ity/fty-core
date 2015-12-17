#/!bin/sh
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
}

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
        --help|-h)
            usage
            exit 1
            ;;
        *)  # fall through
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
[ -z "$BASE_URL"] && BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
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
[ -z "$WEBLIB_QUICKFAIL" ] && \
    WEBLIB_QUICKFAIL=no
[ -z "$WEBLIB_CURLFAIL" ] && \
    WEBLIB_CURLFAIL=no
[ -z "$SKIP_NONSH_TESTS" ] && \
    SKIP_NONSH_TESTS=yes
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_QUICKFAIL WEBLIB_CURLFAIL SKIP_NONSH_TESTS

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

logmsg_info "Will use BASE_URL = '$BASE_URL'"

set -u
set -e

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

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

# ***** AUTHENTICATION ISSUES *****
# check SASL is working
logmsg_info "Checking remote SASL Auth Daemon"
sut_run "testsaslauthd -u '$BIOS_USER' -p '$BIOS_PASSWD' -s '$SASL_SERVICE'" && \
  logmsg_info "saslauthd is responsive and configured well!" || \
  logmsg_error "saslauthd is NOT responsive or not configured!" >&2

# ***** FUNCTIONS *****
    # *** starting the testcases
test_web() {
    echo "==== Calling vte-test_web.sh ==============================="
    /bin/bash "${CHECKOUTDIR}"/tests/CI/vte-test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD" \
        -s "$SASL_SERVICE" -sh "$SUT_HOST" -su "$SUT_USER" -sp "$SUT_SSH_PORT" "$@"
    RESULT=$?
    echo "==== test_web RESULT: ($RESULT) =================================="
    return $RESULT
}

    # *** load default db setting
ci_loaddb_default() {
    echo "--------------- reset db: default ----------------"
    loaddb_file "$DB_BASE" && \
    loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" && \
    loaddb_file "$DB_DATA" && \
    loaddb_file "$DB_DATA_TESTREST"
}
    # *** start the default set of TC
test_web_default() {
    ci_loaddb_default && \
    test_web "$@"
}

    # *** start the power topology set of TC
test_web_topo_p() {
    echo "----------- reset db: topology : power -----------"
    loaddb_file "$DB_BASE" && \
    loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" && \
    loaddb_file "$DB_TOPOP" && \
    test_web "$@"
}

    # *** start the location topology set of TC
test_web_topo_l() {
    echo "---------- reset db: topology : location ---------"
    loaddb_file "$DB_BASE" && \
    loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" && \
    loaddb_file "$DB_TOPOL" && \
    test_web "$@"
}

# Try to accept the BIOS license on server
SKIP_SANITY=yes test_web 00_license-CI-forceaccept.sh.test || \
    logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

# ***** PERFORM THE TESTCASES *****
set +e
    # *** start default admin network(s) TC's

RESULT_OVERALL=0
# admin_network needs a clean state of database, otherwise it does not work
test_web_default admin_networks admin_network || RESULT_OVERALL=$?
    # *** start the other default TC's instead of sysinfo
test_web_default -topology -admin_network -admin_networks -sysinfo || RESULT_OVERALL=$?
    # *** start power topology TC's
test_web_topo_p topology_power || RESULT_OVERALL=$?
    # *** start location topology TC's
test_web_topo_l topology_location || RESULT_OVERALL=$?

# ***** RESULTS *****
if [ "$RESULT_OVERALL" = 0 ]; then
    logmsg_info "Overall result: SUCCESS"
else
    logmsg_error "Overall result: FAILED ($RESULT_OVERALL), seek details above"
fi

exit $RESULT_OVERALL
