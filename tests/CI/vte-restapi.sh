#/!bin/sh

# Copyright (C) 2015 Eaton
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
# Author(s): Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: sets up the sandbox and runs the tests of REST API for
# the $BIOS project.

# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** SUT_PORT and BASE_URL should be set to values corresponded to chosen server ***
    # *** Must run as root without using password ***
    # *** BIOS image must be installed and running on SUT ***
    # *** SUT port and SUT name should be set properly (see below) ***
    # *** tool directory with initdb.sql load_data.sql power_topology.sql and location_topology.sql present on MS ***
    # *** tests/CI directory (on MS) contains weblib.sh (api_get_content and CURL functions needed) ***
    # *** tests/CI/web directory containing results, commands and log subdirectories with the proper content 

# ***** READ PARAMETERS IF PRESENT *****
if [ $# -eq 0 ];then   # parameters missing
    SUT_PORT="2206"
    BIOS_PORT="8006"
    BASE_URL="http://$SUT_NAME:8006/api/v1"
else
    while [ $# -gt 0 ]; do
        case "$1" in
            -o|--port)
            SUT_PORT="$2"
            shift 2
            ;;
        *)
            break
            ;;
        esac
    done
fi
BIOS_PORT=$(expr $SUT_PORT - 2200 + 8000)

# ***** SET CHECKOUTDIR *****
# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the REST API webserver"

RESULT=0
# ***** SET (MANUALY) SUT_NAME - MANDATORY *****
#SUT_PORT="2206"
SUT_NAME="root@debian.roz.lab.etn.com"
#[ -z "$BIOS_PORT" ] && BIOS_PORT="8006"
    # *** if used set BIOS_USER and BIOS_PASSWD
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"

# ***** GLOBAL VARIABLES *****
DB_LOADDIR="$CHECKOUTDIR/tools"
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_TOPOP="power_topology.sql"
DB_TOPOL="location_topology.sql"

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

# ***** USE BIOS_USER AND BIOS_PASSWD *****
usage(){
    echo "Usage: $(basename $0) [options...] [test_name...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
}

    # *** Use user and passwd given in call parameters
while [ $# -gt 0 ] ; do
    case "$1" in
        --user|-u)
            BIOS_USER="$2"
            shift
            ;;
        --passwd|-p)
            BIOS_PASSWD="$2"
            shift
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
    shift
done

set -u
set -e

# TODO. TOHLE PREDELAT, ZATIM MOZNO VYNECHAT
# zacatek vynechavky ********************************
if [ 1 = 2 ]; then
# NETSTAT ZAVOLAT PRES SSH
# KONTROLOVAT PORT 80 PROCESS TNTNET A STAV LISTEN
test_web_port() {
    netstat -tan | grep -w "${BIOS_PORT}" | egrep 'LISTEN' >/dev/null
}
fi
# konec vynechavky **********************************

# ***** AUTHENTICATION ISSUES *****
# check SASL is working
logmsg_info "Checking remote SASL Auth Daemon"
ssh -p $SUT_PORT $SUT_NAME "testsaslauthd -u '$BIOS_USER' -p '$BIOS_PASSWD' -s bios" && \
  logmsg_info "saslauthd is responsive and configured well!" || \
  logmsg_error "saslauthd is NOT responsive or not configured!" >&2

# ***** FUNCTIONS *****
    # *** starting the testcases
test_web() {
    echo "============================================================"
    /bin/bash $CHECKOUTDIR/tests/CI/vte-test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD" \
    -s $SUT_NAME -o $SUT_PORT "$@"
    RESULT=$?
    echo "============================================================"
    return $RESULT
}
    # *** load db file specified in parameter
loaddb_file() {
    DB=$1
    (cat $DB | ssh -p $SUT_PORT $SUT_NAME "systemctl start mysql && mysql" || \
        CODE=$? die "Failed to load $DB to remote system"
     sleep 20 ; echo "DB updated.") || exit $?
}

    # *** load default db setting
loaddb_default() {
    echo "--------------- reset db: default ----------------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_DATA"
}
    # *** start the default set of TC
test_web_default() {
    loaddb_default && \
    test_web "$@"
}

    # *** start the power topology set of TC
test_web_topo_p() {
    echo "----------- reset db: topology : power -----------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_TOPOP" && \
    test_web "$@"
}

    # *** start the location topology set of TC
test_web_topo_l() {
    echo "---------- reset db: topology : location ---------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_TOPOL" && \
    test_web "$@"
}

# ***** PERFORM THE TESTCASES *****
set +e
    # *** start default admin network(s) TC's
# admin_network needs a clean state of database, otherwise it does not work
test_web_default admin_networks admin_network
    # *** start the other default TC's instead of sysinfo
test_web_default -topology -admin_network -admin_networks -sysinfo
    # *** start power topology TC's
test_web_topo_p topology_power
    # *** start location topology TC's
test_web_topo_l topology_location

# ***** RESULTS *****
if [ "$RESULT" = 0 ]; then
    logmsg_info "Overall result: SUCCESS"
else
    logmsg_error "Overall result: FAILED ($RESULT) seek details above" >&2
fi

exit $RESULT
