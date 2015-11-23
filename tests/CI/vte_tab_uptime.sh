#!/bin/sh

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
#! \file   vte_tab_uptime.sh
#  \brief  tests the csv import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

# ***** ABBREVIATIONS *****
    # *** SUT - System Under Test - remote server with BIOS
    # *** MS - Management Station - local server with this script
    # *** TAB - Tabelator

# ***** DESCRIPTION *****
    # *** test imports the assets from csv file with tab used like separator
    # *** tests creates initial (near to empty assets) using database/mysql/initdb.sql file.
    # *** tests compares the exported table's files with expected (only INSERT command)

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT should be passed as parameter -sp <value>
    # *** it is currently from interval <2206;2209>
    # *** must run as root without using password 
    # *** BIOS image must be installed and running on SUT 
    # *** tools directory containing tools/initdb.sql database/mysql/bam_import_16_tab_008.csv present on MS 
    # *** tests/CI directory (on MS) contains weblib.sh and scriptlib.sh library files

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)

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
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "CI-FATAL: $0: Can not web script library" >&2; exit 1; }

logmsg_info "Will use BASE_URL = '$BASE_URL'"

determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
DB_LOADDIR="$CHECKOUTDIR/database/mysql"

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl start $SVC ; done'
sleep 3
sut_run 'R=0; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

# ***** INIT DB *****
    # *** write power rack base test data to DB on SUT
set -o pipefail 2>/dev/null || true
set -e
loaddb_file "$DB_LOADDIR"/initdb.sql 2>&1 | tee $CHECKOUTDIR/vte-tab-${_SCRIPT_NAME}.log
loaddb_file "$DB_LOADDIR"/initdb_ci_patch.sql 2>&1 | tee -a $CHECKOUTDIR/vte-tab-${_SCRIPT_NAME}.log
set +e

# Try to accept the BIOS license on server
( . $CHECKOUTDIR/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
    logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

# ***** POST THE CSV FILE *****
ASSET="$CHECKOUTDIR/tools/bam_import_16_vte_uptime_2_DC.csv"

# Import the bam_import_16_vte_total_power_2_DC.csv file
api_auth_post_file /asset/import assets=@$ASSET -H "Expect:" | tee $CHECKOUTDIR/import_TP-${_SCRIPT_NAME}.log

grep -q '"imported_lines" : 16' $CHECKOUTDIR/import_TP-${_SCRIPT_NAME}.log || die "ERROR : 'Test of the number of imported lines FAILED'"
echo "Test of the number of imported lines 			PASSED"

# create sql file
settraps 'rm -f $CHECKOUTDIR/tmp_uptime.sql'
echo "use box_utf8;"> $CHECKOUTDIR/tmp_uptime.sql

# dates formats
# UPS101_1
date_from_1_1_1=`date -d "2 day ago" '+%F 22:50:00'`
date_to_1_1_1=`date -d "2 day ago" '+%F 23:10:00'`
date_from_2_1_1=`date -d "2 day ago" '+%F 23:05:00'`
date_to_2_1_1=`date -d "1 day ago" '+%F 00:10:00'`

# UPS101_2
date_from_1_1_2=`date -d "1 day ago" '+%F 23:05:00'`
date_to_1_1_2=`date -d "0 day ago" '+%F 00:10:00'`
date_from_2_1_2=`date -d "1 day ago" '+%F 2:50:00'`
date_to_2_1_2=`date -d "0 day ago" '+%F 00:10:00'`

# UPS201_1
date_from_2_1=`date -d "1 day ago" '+%F '`;date_from_2_1=$date_from_2_1`echo 22:50:00` 
date_to_2_1=`date -d "1 day ago" '+%F '`;date_to_2_1=$date_to_2_1`echo 23:10:00`
date_from_2_1=`date -d "1 day ago" '+%F '`;date_from_2_1=$date_from_2_1`echo 22:55:00`
date_to_2_1=`date -d "0 day ago" '+%F '`;date_to_2_1=$date_to_2_1`echo 00:10:00`

# UPS201_2
date_from_2_2=`date -d "2 day ago" '+%F '`;date_from_2_2=$date_from_2_2`echo 23:59:00`
date_to_2_2=`date -d "1 day ago" '+%F '`;date_to_2_2=$date_to_2_2`echo 00:01:00`
date_from_2_2=`date -d "1 day ago" '+%F '`;date_from_2_2=$date_from_2_2`echo 01:00:00`
date_to_2_2=`date -d "1 day ago" '+%F '`;date_to_2_2=$date_to_2_2`echo 02:00:00`

#insert line for UPS101_1
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS101_1', UNIX_TIMESTAMP('$date_from_1_1_1') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_1_1_1') ,            0 , 1);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS101_1', UNIX_TIMESTAMP('$date_from_2_1_1') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_2_1_1') ,            0 , 1);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql

#insert line for UPS101_2
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS101_2', UNIX_TIMESTAMP('$date_from_1_1_2') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_1_1_2') ,            0 , 1);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS101_2', UNIX_TIMESTAMP('$date_from_1_1_2') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_1_1_2') ,            0 , 1);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql


#insert line for UPS201_1
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS201_1', UNIX_TIMESTAMP('$date_from_2_1') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_2_1') ,            0 , 9);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql
#insert line for UPS201_2
sqlline="INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES ( 'upsonbattery@UPS201_2', UNIX_TIMESTAMP('$date_from_2_2') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('$date_to_2_2') ,            0 , 9);"
echo $sqlline >> $CHECKOUTDIR/tmp_uptime.sql
cat $CHECKOUTDIR/tmp_uptime.sql
loaddb_file $CHECKOUTDIR/tmp_uptime.sql 2>&1 | tee -a $CHECKOUTDIR/vte-tab-${_SCRIPT_NAME}.log

sut_run 'systemctl restart biostimer-outage.service'

PAR="/metric/computed/uptime?arg1=1"
UPTIME_RESP="`api_get_json "$PAR"`" && \
logmsg_info "SUCCESS." || \
logmsg_error "FAILED ($?)."
UPTIME="$(echo "$UPTIME_RESP" | grep outage | grep -v '\[' | sed 's/: /%/' | cut -d'%' -f2 | cut -d',' -f1)"
echo $UPTIME
if [ $UPTIME = "3900" ]; then
   echo "The uptime for DC-LAB has an expected value: UPTIME = $UPTIME. Test PASSED."
else
   echo "The uptime for DC-LAB has not an expected value: UPTIME = $UPTIME. Test FAILED."
   exit 1
fi

PAR="/metric/computed/uptime?arg1=9"
UPTIME_RESP="`api_get_json "$PAR"`" && \
logmsg_info "SUCCESS." || \
logmsg_error "FAILED ($?) ."
UPTIME="$(echo "$UPTIME_RESP" | grep outage | grep -v '\[' | sed 's/: /%/' | cut -d'%' -f2 | cut -d',' -f1)"
if [ $UPTIME = "7500" ]; then
   echo "The uptime for DC-LAB2 has an expected value: UPTIME = $UPTIME. Test PASSED."
else
   echo "The uptime for DC-LAB2 has not an expected value: UPTIME = $UPTIME. Test FAILED."
   exit 1
fi


TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Finish time is $TIME"
TIME_END=$(date +%s)
TEST_LAST=$(expr $TIME_END - $TIME_START)
echo "Test lasted $TEST_LAST second."
exit 0
 

