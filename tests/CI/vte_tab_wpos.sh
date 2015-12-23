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
#! \file   vte_tab_wpos.sh
#  \brief  tests the csv import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

# ***** HOW TO RUN IT *****
# test/CI/vte_tab_wpos.sh

# ***** ABBREVIATIONS *****
    # *** SUT - System Under Test - remote server with BIOS
    # *** MS - Management Station - local server with this script
    # *** TAB - Tabelator

# ***** DESCRIPTION *****
    # *** test imports the assets from csv file with tab used like separator
    # *** tests creates initial (near to empty assets) using database/mysql/initdb.sql file.
    # *** tests finds if the location_w_pos is mandatory, max 2 pdu/epdu devices in one rack,
    # *** only on right or left in the rack

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT should be passed as parameter -sp <value>
    # *** it is currently from interval <2206;2209>
    # *** must run as root without using password
    # *** BIOS image must be installed and running on SUT
    # *** directories containing database/mysql/initdb.sql tools/bam_import_16_tab_008.csv present on MS
    # *** tests/CI directory (on MS) contains weblib.sh and scriptlib.sh library files
    # *** tools/bam_import_16_wpos1.csv - bam_import_16_wpos4.csv MUST be present

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)
[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""
[ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="https"

    # *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        --port-web|--sut-port-web|-wp|-o|--port)
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
        *)  echo "$0: Unknown param and all after it are ignored: $@"
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

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
NEED_TESTLIB=yes
. "`dirname $0`"/weblib.sh || die "Can not include web script library"

logmsg_info "Will use BASE_URL = '$BASE_URL'"

determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl start $SVC ; done'
sleep 3
sut_run 'R=0; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

LOGFILE_LOADDB="$BUILDSUBDIR/vte-tab-loaddb-${_SCRIPT_NAME}.log"
LOGFILE_IMPORT="$BUILDSUBDIR/vte-tab-import_TP-${_SCRIPT_NAME}.log"

subtest() {
    # ***** INIT DB *****
    # *** write power rack base test data to DB on SUT
    test_it "re-initialize database"
    loaddb_file "$DB_BASE" 2>&1 | tee "${LOGFILE_LOADDB}"
    LOADDB_FILE_REMOTE_SLEEP=1 loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" 2>&1 | tee -a "${LOGFILE_LOADDB}"
    print_result $?

    # ***** POST THE CSV FILE *****
    ASSET="$CSV_LOADDIR_BAM/$1"
    # TODO: Original script missed some step that actually imports the data and
    # generates the "${LOGFILE_IMPORT}". I guess the one below is it...
    # QA said, maybe we should port this logic into web/commands/asset*sh tests

    logmsg_info "Importing asset file: $ASSET"
    test_it "import_asset::$1"
    api_auth_post_file_form /asset/import assets="@$ASSET" | tee "${LOGFILE_IMPORT}"
    print_result $?

    test_it "inspect_asset::$1"
    case "$1" in
        bam_import_16_wpos1.csv)
            N_EXPECT="`grep -c "more than 2 PDU is not supported" "${LOGFILE_IMPORT}"`"
            echo "N_EXPECT = $N_EXPECT (1)"
            [ "$N_EXPECT" = "1" ]
            print_result $?
            ;;
        bam_import_16_wpos2.csv)
            N_EXPECT="`grep -c "location_w_pos should be set" "${LOGFILE_IMPORT}"`"
            echo "N_EXPECT = $N_EXPECT (4)"
            [ "$N_EXPECT" = "4" ]
            print_result $?
            ;;
        bam_import_16_wpos3.csv)
            N_EXPECT="`grep -c '"imported_lines" : 7' "${LOGFILE_IMPORT}"`"
            echo "N_EXPECT = $N_EXPECT (1)"
            [ "$N_EXPECT" = "1" ]
            print_result $?
            ;;
        bam_import_16_wpos4.csv)
            N_EXPECT="`grep -c '"imported_lines" : 7' "${LOGFILE_IMPORT}"`"
            echo "N_EXPECT = $N_EXPECT (1)"
            [ "$N_EXPECT" = "1" ]
            print_result $?
            ;;
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            print_result 1
            ;;
    esac
}

# Note: this default log filename will be ignored if already set by caller
init_summarizeTestlibResults "${BUILDSUBDIR}/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'exit_summarizeTestlibResults'

# Try to accept the BIOS license on server
( . $CHECKOUTDIR/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
    if [ x"$CITEST_QUICKFAIL" = xyes ] ; then
        die "BIOS license not accepted on the server, subsequent tests will fail"
    else
        logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"
    fi

subtest "bam_import_16_wpos1.csv"
subtest "bam_import_16_wpos2.csv"
subtest "bam_import_16_wpos3.csv"
subtest "bam_import_16_wpos4.csv"

exit
