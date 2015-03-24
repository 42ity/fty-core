#!/bin/bash
#
# Copyright (C) 2014 Eaton

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
# Author(s): Radomir Vrajik <RadomirVrajik@Eaton.com>,
#
# Description: tests the total rack power

# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** SUT_PORT and BASE_URL should be set to values corresponded to chosen server ***
    # *** Must run as root without using password ***
    # *** BIOS image must be installed and running on SUT ***
    # *** SUT port and SUT name should be set properly (see below) ***
    # *** upsd.conf, upssched.conf and upsmon.conf are present on SUT in the /etc/nut dir ***
    # *** tool directory with tools/initdb.sql tools/rack_power.sql present on MS ***
    # *** tests/CI directory (on MS) contains weblib.sh (api_get_content and CURL functions needed) ***

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)
    # *** required SUT port and SUT name
SUT_PORT="2207"
SUT_NAME="root@debian.roz.lab.etn.com"
BASE_URL="http://$SUT_NAME:8007/api/v1"

    # *** config dir for the nut dummy driver parameters allocated in config files
CFGDIR="/etc/nut"

    # *** parameters monitored and counted with
PARAM1="ups.realpower"
PARAM2="outlet.realpower"

    # *** user and password for upsrw
USR=user1
PSW=user1

    # *** Numbers of passed/failed subtest
SUM_PASS=0
SUM_ERR=0

# ***** INIT *****
function cleanup {
    rm -f "$LOCKFILE"
}
    # *** weblib include ***
SCRIPTDIR=$(dirname $0)
CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
. "$SCRIPTDIR/weblib.sh"
    # *** web is listening ***

trap cleanup EXIT SIGINT SIGQUIT SIGTERM
# ***** FILL AND START DB *****
    # *** write power rack base test data to DB on SUT
#(cat tools/initdb.sql tools/rack_power.sql | ssh -p $SUT_PORT $SUT_NAME "systemctl start mysql && mysql"; sleep 20 ; echo "DB updated.") 2>&1| tee /tmp/tmp
(cat $CHECKOUTDIR/tools/initdb.sql $CHECKOUTDIR/tools/rack_power.sql | ssh -p $SUT_PORT $SUT_NAME "systemctl start mysql && mysql"; sleep 20 ; echo "DB updated.") 2>&1| tee /tmp/tmp

# ***** COMMON FUNCTIONS ***
    # *** rem_copy_file()
rem_copy_file() {
SRC_FILE=$1
DST_FILE=$2
COPY_CMD="cd / ; tar -xf - ;mv -f /tmp/$SRC_FILE $CFGDIR/$DST_FILE"
(cd /;tar -cf - tmp/$SRC_FILE | ssh -p $SUT_PORT $SUT_NAME "$COPY_CMD" & )
}

    # *** rem_cmd()
#Send remote command from MS to be performed in SUT
rem_cmd() {
    REM_CMD=$1
    (ssh -p $SUT_PORT $SUT_NAME "$REM_CMD" & ) | tee /tmp/ci-rackpower.log
}
    # *** set_values_in_ups()
set_values_in_ups() {
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
sed -r -e "s/UPS1/$UPS1/g" </tmp/pattern.conf >/tmp/pattern.tmp
sed -r -e "s/UPS2/$UPS2/g" </tmp/pattern.tmp >/tmp/ups.new

    # *** Copy the .dev .conf files to SUT
if [ $UPS = $UPS1 ]; then
    if [ "$TYPE2" = epdu ]; then
        rem_copy_file pattern-epdu.dev $UPS2.dev
    else
        rem_copy_file pattern-ups.dev $UPS2.dev
    fi
fi
rem_copy_file $UPS.new $UPS.dev
rem_copy_file ups.new ups.conf
sleep 3
    # *** start upsrw
echo "start upsrw"
rem_cmd "upsrw -s $PARAM=$VALUE -u $USR -p $PSW $UPS@localhost >/dev/null 2>&1"
    # *** restart NUT server
echo 'restart NUT server'
rem_cmd "systemctl stop nut-server"
rem_cmd "systemctl stop nut-driver"
rem_cmd "systemctl start nut-server"
echo 'Wait ...' 
sleep 10
}

    # *** testcase()
testcase() {

echo "starting the test"

SAMPLESCNT=$(expr ${#SAMPLES[*]} - 1) # sample counter begin from 0
ERRORS=0
SUCCESSES=0
LASTPOW=(0 0)
for UPS in $UPS1 $UPS2 ; do
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

        TP=$(awk -vX=${LASTPOW[0]} -vY=${LASTPOW[1]} 'BEGIN{ print X + Y; }')
        PAR=/metric/computed/rack_total?arg1="$RACK"'&'arg2=total_power
        RACK_TOTAL_POWER1_CONTENT=`api_get_content $PAR`
        POWER=$(echo "$RACK_TOTAL_POWER1_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
        STR1="$(printf "%f" $TP)"  # this returns "2000000.000000"
        STR2="$(printf "%f" $POWER)"  # also returns "2000000.000000"
        DEL=$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')
        if [ $DEL = 0 ]; then
           echo "The total power has an expected value $TP = $POWER. Test PASSED."
           SUCCESSES=$(expr $SUCCESSES + 1)
        else
            echo "$TP does not equal expected value $TP <> $POWER - Test FAILED."
            ERRORS=$(expr $ERRORS + 1)
        fi
    done
done
}

    # *** results()
results() {
    SUCCESSES=$1
    ERRORS=$2
    echo "Pass: ${SUCCESSES}/Fails: ${ERRORS}"
}

# ***** INIT *****
    # *** weblib include ***
SCRIPTDIR=$(dirname $0)
CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
. "$SCRIPTDIR/weblib.sh"

# ***** START *****
    # *** check the processes running on SUT
# TODO

    # *** check the nut config files on SUTT 
# TODO
# upsd.conf, upssched.conf and upsmon.conf ARE PRESENT ON SUT
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

        # * Copy the cfgfiles to SUT
rem_copy_file nut.conf nut.conf
rem_copy_file upsd.users upsd.users

    # *** create pattern .dev AND .conf files
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
UPS1="epdu101_1_"
UPS2="epdu101_2_"
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
SAMPLES=(
  1004567.34
  1064.34
  1130000
)
UPS1="epdu102_1_"
UPS2="epdu102_2_"
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
SAMPLES=(
  100.999
  80.001
  120.499
)
UPS1="ups103_1_"
UPS2="ups103_2_"
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
SAMPLES=(
  100.501
  80.499
  120.99999999999999
)
UPS1="epdu105_1_"
UPS2="pdu105_1_"
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
SAMPLES=(
  48
  55
  63
)

UPS1="ups106_1_"
UPS2="pdu106_2_"
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
    exit 0
fi
exit 1
