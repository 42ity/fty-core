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
#! \file   vte-rackpower.sh
#  \brief  tests the total rack power
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

# ***** ABBREVIATIONS *****
    # *** SUT - System Under Test - remote server with BIOS
    # *** MS - Management Station - local server with this script
    # *** TRP - Total Rack Power

# ***** DESCRIPTION *****
    # *** The new topology is assets
    # *** test creates defined messages through nut using dummy nut driver
    # *** data contained in the messages pass through nut into DB
    # *** restAPI req. for TRP (arg1="$RACK"'&'arg2=total_power) is sent
    # *** expected value of TRP is compared with the one get from restapi req.
    # *** this way it contains also smoke test of the chain nut->DB->restAPI

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT should be passed as parameter --port <value>
    # *** it is currently from interval <2206;2209>, for restAPI reguests are generated ports from <8006;8009>
    # *** must run as root without using password 
    # *** BIOS image must be installed and running on SUT 
    # *** upsd.conf, upssched.conf and upsmon.conf are present on SUT in the /etc/nut dir 
    # *** tools directory containing tools/initdb.sql database/mysql/rack_power.sql present on MS for assets
    # *** tests/CI directory (on MS) contains weblib.sh (api_get_json and CURL functions needed) and scriptlib.sh

TIME_START=$(date +%s)
#export BIOS_LOG_LEVEL=LOG_DEBUG

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
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            break
            ;;
    esac
done

    # *** default connection parameters values:
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
    # *** set SUT base URL and SUT name
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
    # *** local or remote?
SUT_IS_REMOTE=yes

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2;exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
# *** weblib include
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"

        # * config dir for the nut dummy driver parameters allocated in config files
    # *** working directories
CFGDIR="/etc/nut"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
echo "SCRIPTDIR =       $SCRIPTDIR"
echo "CHECKOUTDIR =     $CHECKOUTDIR"
echo "BUILDSUBDIR =     $BUILDSUBDIR"
DB_LOADDIR="$CHECKOUTDIR/database/mysql"

logmsg_info "Will use BASE_URL = '$BASE_URL'"

# ***** GLOBAL VARIABLES *****
    # *** config dir for the nut dummy driver parameters allocated in config files
CFGDIR="/etc/nut"
    # *** parameters monitored and counted with
PARAM1="ups.realpower"
PARAM2="outlet.realpower"
    # *** user and password for upsrw
USR=user1
PSW=user1
    # *** create lockfile name ***
LOCKFILE="`echo "/tmp/ci-test-rackpower-vte__${SUT_USER}@${SUT_HOST}:${SUT_SSH_PORT}:${SUT_WEB_PORT}.lock" | sed 's, ,__,g'`"

# ***** INIT *****
function cleanup {
    set +e
    rm -f "$LOCKFILE"
}
    # *** is system running?
if [ -f "$LOCKFILE" ]; then
    ls -la "$LOCKFILE" >&2
    die "Script already running. Aborting."
    cleanup
fi
    # *** lock the script with creating $LOCKFILE
echo $$ > "$LOCKFILE"

    # ***  SET trap FOR EXIT SIGNALS
TRAP_SIGNALS=EXIT settraps cleanup

logmsg_info "Will use BASE_URL = '$BASE_URL'"

# TODO: replace by calls to proper rc-bios script
logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-server-agent bios-agent-nut bios-agent-inventory bios-agent-cm; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done;exit $R' || \
    die "Some required services are not running on the VTE"

# ***** FILL AND START DB *****
    # *** write power rack base test data to DB on SUT
set -o pipefail 2>/dev/null || true
set -e
{ loaddb_file "$DB_LOADDIR"/initdb.sql && \
  loaddb_file "$DB_LOADDIR"/initdb_ci_patch.sql && \
  loaddb_file "$DB_LOADDIR"/rack_power.sql \
; } 2>&1 | tee $CHECKOUTDIR/ci-rackpower-vte.log

# Try to accept the BIOS license on server
( . $CHECKOUTDIR/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
    logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

set +e

sut_run 'systemctl restart bios-agent-tpower'
sut_run 'systemctl restart bios-agent-dbstore'

# ***** COMMON FUNCTIONS ***
    # *** rem_copy_file()
        # * make copy of the file from MS (the first parameter) to SUT (the second parameter)
rem_copy_file() {
    SRC_FILE=$1
    DST_FILE=$2
    COPY_CMD="cd / ; tar -xf - ;mv -f /tmp/$SRC_FILE $CFGDIR/$DST_FILE; chown nut:root $CFGDIR/$DST_FILE"
    ( cd /;tar -cf - tmp/$SRC_FILE | sut_run "$COPY_CMD" )
}

    # *** rem_cmd()
        # * Send remote command from MS to be performed in SUT
rem_cmd() {
    sut_run "$@" | tee -a $CHECKOUTDIR/ci-rackpower-vte.log
}
    # *** set_values_in_ups()
        # * set new future values of measured data 
set_values_in_ups() {
    ### TODO: Rewrite so that regular dev-file changes and upsrw are separate
    ### from new dev-file definitions and NUT restart (that's once per testcase
    ### or even once at all if we pre-define all devices at once).
    UPS="$1"
    TYPE="$2"
    VALUE="$3"

    echo "set values in <upsX>.dev"
    if [ "$TYPE" = epdu ]; then
        sed -r -e "s/^$PARAM2 *:.+$/$PARAM2: $VALUE/i" </tmp/pattern-epdu.dev >/tmp/$UPS.new
        PARAM=$PARAM2
    elif [ "$TYPE" = pdu ]; then
        sed -r -e "s/^$PARAM2 *:.+$/$PARAM2: 0/i" </tmp/pattern-ups.dev >/tmp/$UPS.new
    else
        sed -r -e "s/^$PARAM2 *:.+$/$PARAM2: $VALUE/i" </tmp/pattern-ups.dev >/tmp/$UPS.new
        PARAM=$PARAM2
    fi

    echo "set values in ups.conf"
    sed -r -e "s/UPS1/$UPS1/g" -e "s/UPS2/$UPS2/g" </tmp/pattern.conf >/tmp/ups.new

            # * Copy the .dev .conf files to SUT
    if [ $UPS = $UPS1 ]; then
        if [ "$TYPE2" = epdu ]; then
            rem_copy_file pattern-epdu.dev $UPS2.dev
        else
            rem_copy_file pattern-ups.dev $UPS2.dev
        fi
    fi
    rem_copy_file $UPS.new $UPS.dev
    rem_copy_file ups.new ups.conf

            # * restart NUT server
#    echo 'Restart NUT server with updated config'
    rem_cmd "systemctl stop nut-server; sleep 5; systemctl stop nut-driver; sleep 5; systemctl start nut-driver; sleep 5; systemctl start nut-server"
#    echo 'Wait for NUT to start responding...' 
    sleep 60
#    # some agents may be requesting every 5 sec, so exceed that slightly to be noticed
#    N=20
#    while [ "$N" -gt 0 ]; do
#        OUT="$(sut_run "upsrw -u $USR -p $PSW $UPS@localhost")"
#        if [ "$?" = 0 ] || [ -n "$OUT" ]; then N=-$N; break; fi
#        sleep 3
#        N="`expr $N - 1`"
#    rem_cmd "systemctl stop nut-server; sleep 5; systemctl stop nut-driver; sleep 5; systemctl start nut-driver; sleep 5; systemctl start nut-server";sleep 15
#    done
#    [ "$N" = 0 ] && \
#        echo "NOTE: The wait loop for NUT response has expired without success"
#
#    # *** start upsrw (output hidden because this can fail for some target variables)
#    echo "Execute upsrw to try set $PARAM=$VALUE on $UPS@localhost"
#    rem_cmd "upsrw -s $PARAM=$VALUE -u $USR -p $PSW $UPS@localhost 2>/dev/null"
}

    # *** testcase()
        # * count, expected result, read restAPI result, compare them and set the result
testcase() {
    echo "starting the test"

    SAMPLESCNT=$(expr ${#SAMPLES[*]} - 1) # sample counter begin from 0
    ERRORS=0
    SUCCESSES=0
    LASTPOW=(0 0)
    for UPS in $UPS1 $UPS2 ; do
                       # count expected value of total power
        for SAMPLECURSOR in $(seq 0 $SAMPLESCNT); do
            # set values
            NEWVALUE=${SAMPLES[$SAMPLECURSOR]}
            if [ $UPS = $UPS1 ]; then
                set_values_in_ups "$UPS" "$TYPE1" "$NEWVALUE"
                if [ $TYPE1 = "pdu" ]; then
                    LASTPOW[0]=0
                else
                    LASTPOW[0]=$NEWVALUE
                fi
            else
                set_values_in_ups $UPS $TYPE2 $NEWVALUE
                if [ $TYPE2 = "pdu" ]; then
                    LASTPOW[1]=0
                else
                    LASTPOW[1]=$NEWVALUE
                fi
            fi
            TP="$(awk -vX=${LASTPOW[0]} -vY=${LASTPOW[1]} 'BEGIN{ print X + Y; }')"
                       # send restAPI request to find generated value of total power
            PAR="/metric/computed/rack_total?arg1=${RACK}&arg2=total_power"
            RACK_TOTAL_POWER1_CONTENT="`api_get_json "$PAR"`" && \
                logmsg_info "SUCCESS: api_get_json '$PAR': $RACK_TOTAL_POWER1_CONTENT" || \
                logmsg_error "FAILED ($?): api_get_json '$PAR'" 
            POWER="$(echo "$RACK_TOTAL_POWER1_CONTENT" | grep total_power | sed 's/: /%/' | cut -d'%' -f2)"
                       # synchronize format of the expected and generated values of total power
            STR1="$(printf "%f" $TP)"  # this returns "2000000.000000"
            STR2="$(printf "%f" $POWER)"  # also returns "2000000.000000"
                       # round both numbers and compare them
                       # decide the test is successfull or failed
            DEL="$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')"
            if [ $DEL = 0 ]; then
                echo "The total power has an expected value: '$TP' = '$POWER'. Test PASSED."
                SUCCESSES=$(expr $SUCCESSES + 1)
            else
                echo "The total power does not equal expected value: '$TP' <> '$POWER' - Test FAILED."
                ERRORS=$(expr $ERRORS + 1)
            fi
        done
    done
}

    # *** results()
                       # show number of the passed/failed sub TC's
results() {
    SUCCESSES=$1
    ERRORS=$2
    echo "Pass: ${SUCCESSES}/Fails: ${ERRORS}"
}


# ***** START *****
    # *** remove old .dev files ON SUT
rem_cmd "rm -f $CFGDIR/*.dev"
    # *** create the nut.conf and upsd.users files on SUT
        # * nut.conf
echo "creating nut config"
echo "MODE=standalone" > /tmp/nut.conf
        # * upsd.users
echo "[$USR]
password=$PSW
actions=SET
instcmds=ALL" > /tmp/upsd.users
chmod 640 /tmp/upsd.users

        # * Copy the cfgfiles to SUT
rem_copy_file nut.conf nut.conf
rem_copy_file upsd.users upsd.users
    # *** create pattern .dev AND .conf file
                 # pattern.dev (for <upsX.dev>) and pattern.conf (for ups.conf)
        # * create the paterns of ups or epdu file on MS
TYPE=$1
        # * Create "epdu" .dev pattern
echo "# epdu power sequence file
device.type: epdu
outlet.realpower: 0
#outlet.1.voltage: 220
#outlet.2.voltage: 220
#outlet.3.voltage: 220
" > /tmp/pattern-epdu.dev

        # * Create "ups" .dev pattern
echo "# ups power sequence file
device.type: ups
ups.realpower: 0
outlet.realpower: 0
#battery.charge: 90
" > /tmp/pattern-ups.dev

        # * create .conf pattern
    echo "[UPS1]
driver=dummy-ups
port=UPS1.dev
desc='dummy-pdu in dummy mode'

[UPS2]
driver=dummy-ups
port=UPS2.dev
desc='dummy-ups 2 in dummy mode'" > /tmp/pattern.conf

USR=user1
PSW=user1
SUM_PASS=0
SUM_ERR=0

UPS1="epdu101_1"
UPS2="epdu101_2"
RACK="8101"
TYPE1="epdu"
TYPE2="epdu"
UPS=$UPS1

set_values_in_ups "$UPS" "$TYPE1" "0"
UPS=$UPS2
set_values_in_ups "$UPS" "$TYPE2" "0"

   # *** restart NUT server
    echo 'Restart NUT server with updated config'
    rem_cmd "systemctl stop nut-server; sleep 5; systemctl stop nut-driver; sleep 5; systemctl start nut-driver; sleep 5; systemctl start nut-server"
    sleep 60
#    rem_cmd "systemctl stop nut-server; systemctl stop nut-driver; sleep 3; systemctl start nut-driver; sleep 3; systemctl start nut-server";sleep 30
    echo 'Wait for NUT to start responding...'

# ***** TESTCASES *****

    # *** TC1
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 1"
echo "+++++++++++++++++++++++++++++++++++"
TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Start time is "$TIME
        # * set TC1 specific variables
SAMPLES=(
   50.56
   30.85
   40.43
)
UPS1="epdu101_1"
UPS2="epdu101_2"
RACK="8101"
TYPE1="epdu"
TYPE2="epdu"
        # * start TC1
testcase
        # * TC1 results
echo "@@@@@@@@@@ TEST1 RESULTS @@@@@@@@@@"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

    # *** TC2
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 2"
echo "+++++++++++++++++++++++++++++++++++"
        # * set TC2 specific variables
TIME_ACT=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Time is "$TIME_ACT

SAMPLES=(
  1004567.34
  1064.34
  1130000
)
UPS1="epdu102_1"
UPS2="epdu102_2"
RACK="8108"
TYPE1="epdu"
TYPE2="epdu"
        # * start TC2
testcase
        # * TC2 results
echo "@@@@@@@@@@ TEST2 RESULTS @@@@@@@@@@"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

    # *** TC3
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 3"
echo "+++++++++++++++++++++++++++++++++++"
        # * set TC3 specific variables
TIME_ACT=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Time is "$TIME_ACT
SAMPLES=(
  100.999
  80.001
  120.499
)
UPS1="ups103_1"
UPS2="ups103_2"
RACK="8116"
TYPE1="ups"
TYPE2="ups"
        # * start TC3
testcase
        # * TC3 results
echo "@@@@@@@@@@ TEST3 RESULTS @@@@@@@@@@"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})
    # *** TC6
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 6"
echo "+++++++++++++++++++++++++++++++++++"
        # * set TC6 specific variables
TIME_ACT=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Time is "$TIME_ACT
SAMPLES=(
  100.501
  80.499
  120.99999999999999
)
UPS1="epdu105_1"
UPS2="pdu105_1"
RACK="8134"
TYPE1="epdu"
TYPE2="pdu"
        # * start TC6
testcase
        # * TC6 results
echo "@@@@@@@@@@ TEST6 RESULTS @@@@@@@@@@"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 8"
echo "+++++++++++++++++++++++++++++++++++"
        # * set TC8 specific variables
TIME_ACT=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Time is "$TIME_ACT
SAMPLES=(
  48
  55
  63
)

UPS1="ups106_1"
UPS2="pdu106_2"
RACK="8141"
TYPE1="ups"
TYPE2="pdu"
echo "@@@@@@@@@@ TEST8 RESULTS @@@@@@@@@@"
        # * start TC8
testcase
        # * TC8 results
echo "Test8 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

echo # ***** SUMMARY ***
echo "Passed: $SUM_PASS / Failed: $SUM_ERR"

TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Finish time is "$TIME
TIME_END=$(date +%s)
TEST_LAST=$(expr $TIME_END - $TIME_START)
echo "Test lasts "$TEST_LAST" second."
if [ $SUM_ERR = 0 ] ; then
    echo "TEST PASSED."
    cleanup
    exit 0
fi
echo "TEST FAILED."
cleanup
exit 1
