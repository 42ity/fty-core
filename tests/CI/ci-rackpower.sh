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
#! \file   ci-rackpower.sh
#  \brief  tests the total rack power
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#
# requirements:
#   Must run as root (nut configuration)
#   bios must be running
#   nut must be installed
#

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR' (it may be empty but should exist)"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build, and BUILDSUBDIR='$BUILDSUBDIR' to run"

WEBTESTPID=""
DBNGPID=""
kill_daemons() {
    set +e
    if [ -n "$WEBTESTPID" -a -d "/proc/$WEBTESTPID" ]; then
        logmsg_info "Killing make web-test PID $WEBTESTPID to exit"
        kill -INT "$WEBTESTPID"
    fi
    if [ -n "$DBNGPID" -a -d "/proc/$DBNGPID" ]; then
        logmsg_info "Killing agent-dbstore PID $DBNGPID to exit"
        kill -INT "$DBNGPID"
    fi

    killall -INT tntnet agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1
    killall      tntnet agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1

    ps -ef | grep -v grep | egrep "tntnet|agent-dbstore" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "tntnet and/or agent-dbstore still alive, trying SIGKILL" && \
        { killall -KILL tntnet agent-dbstore lt-agent-dbstore 2>/dev/null ; exit 1; }

    return 0
}

logmsg_info "Ensuring that the tested programs have been built and up-to-date"
if [ ! -f "$BUILDSUBDIR/Makefile" ] ; then
    ./autogen.sh --nodistclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG}
fi
./autogen.sh ${AUTOGEN_ACTION_MAKE} web-test-deps agent-dbstore agent-nut 
./autogen.sh --noparmake ${AUTOGEN_ACTION_MAKE} web-test \
    >> ${BUILDSUBDIR}/web-test.log 2>&1 &
WEBTESTPID=$!

# TODO: this requirement should later become the REST AGENT
logmsg_info "Spawning the agent-dbstore server in the background..."
${BUILDSUBDIR}/agent-dbstore &
DBNGPID=$!

# Ensure that no processes remain dangling when test completes
TRAP_SIGNALS=EXIT settraps 'echo "CI-EXIT: $0: test finished (up to the proper exit command)..." >&2; kill_daemons'
TRAP_SIGNALS="HUP INT QUIT TERM" settraps 'echo "CI-EXIT: $0: got signal, aborting test..." >&2; kill_daemons'

DB_LOADDIR="$CHECKOUTDIR/tools"
DB1="initdb.sql"
DB2="rack_power.sql"
loaddb_file "$DB_LOADDIR/$DB1"
loaddb_file "$DB_LOADDIR/$DB2"
#
# only one parameter - ups.realpower for ups ot outlet.realpower for epdu is used for the total rack power value counting
#
PARAM1="ups.realpower"
PARAM2="outlet.realpower"
#
# config dir for the nut dummy driver parameters allocated in config files
CFGDIR="/etc/ups"
[ -d $CFGDIR ] || CFGDIR="/etc/nut"
if [ ! -d $CFGDIR ] ; then
    logmsg_error "NUT config dir not found"
    kill $WEBTESTPID >/dev/null 2>&1
    exit 1
fi
set_value_in_ups() {
    UPS=$1
    PARAM=$2
    VALUE=$3

    sed -r -e "s/^$PARAM *:.+$/$PARAM: $VALUE/i" <$CFGDIR/$UPS.dev >$CFGDIR/$UPS.new
    mv -f $CFGDIR/$UPS.new $CFGDIR/$UPS.dev
    upsrw -s $PARAM=$VALUE -u $USR -p $PSW $UPS@localhost >/dev/null 2>&1
}

create_device_definition_file() {
    FILE=$1
    TYPE=ups
    if (basename "$FILE" | grep --silent "epdu" ) ; then
        TYPE=epdu
    fi
    if [ "$TYPE" = "epdu" ] ; then
	echo "# epdu power sequence file
device.type: epdu
outlet.realpower: 0
#outlet.1.voltage: 220
#outlet.2.voltage: 220
#outlet.3.voltage: 220
" > $FILE
    else
    echo "# ups power sequence file
device.type: ups
ups.realpower: 0
outlet.realpower: 0
#battery.charge: 90
" > $FILE
    fi
}

create_nut_config() {
    logmsg_info "creating nut config"
    echo "MODE=standalone" > $CFGDIR/nut.conf 
    echo "[$UPS1]
driver=dummy-ups
port=$UPS1.dev
desc=\"dummy-pdu in dummy mode\" 

[$UPS2]
driver=dummy-ups
port=$UPS2.dev
desc=\"dummy-ups 2 in dummy mode\" " > $CFGDIR/ups.conf

    echo "[$USR]
password=$PSW
actions=SET
instcmds=ALL" > $CFGDIR/upsd.users

    create_device_definition_file "$CFGDIR/$UPS1.dev"
    create_device_definition_file "$CFGDIR/$UPS2.dev"

    chown nut:root $CFGDIR/*.dev
    logmsg_info "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    sleep 3
    systemctl start nut-driver
    sleep 3
    systemctl start nut-server
    logmsg_info "waiting for a while..."
    sleep 15
}

testcase() {
    UPS1=$1
    UPS2=$2
    SAMPLES=$3
    RACK=$4
    PARAM=""
    logmsg_info "starting the test"

SAMPLESCNT=$(expr ${#SAMPLES[*]} - 1) # sample counter begin from 0
ERRORS=0
SUCCESSES=0
LASTPOW=(0 0)
for UPS in $UPS1 $UPS2 ; do
    for SAMPLECURSOR in $(seq 0 $SAMPLESCNT); do
        # set values
        NEWVALUE=${SAMPLES[$SAMPLECURSOR]}
        TYPE=$(echo $UPS|grep ^pdu|wc -l)
	#echo "TYPE = " $TYPE
        if [ "$TYPE" = 1 ]; then
               NEWVALUE=0
        fi
        TYPE2=$(echo $UPS|grep ^epdu|wc -l)
	if [ "$TYPE2 = 1" ]; then
            set_value_in_ups $UPS $PARAM1 0
            set_value_in_ups $UPS $PARAM2 $NEWVALUE
	else
            set_value_in_ups $UPS $PARAM1 $NEWVALUE
            set_value_in_ups $UPS $PARAM2 0
	fi
        sleep 10  # 10 s is max time for propagating into DB (poll ever 5s in nut actor + some time to process)
        NEWVALUE=${SAMPLES[$SAMPLECURSOR]}
        case "$UPS" in
            "$UPS1")
                if [ "$TYPE" = 1 ]; then
                   NEWVALUE=0
                fi
                LASTPOW[0]=$NEWVALUE
                ;;
            "$UPS2")
                if [ "$TYPE" = 1 ]; then
                   NEWVALUE=0
                fi
                LASTPOW[1]=$NEWVALUE
                ;;
        esac
        TP=$(awk -vX=${LASTPOW[0]} -vY=${LASTPOW[1]} 'BEGIN{ print X + Y; }')
        # TODO: parametrize
        # TODO: use weblib.sh
        # Try to accept the BIOS license on server
        ( BASE_URL='http://127.0.0.1:8000/api/v1'; export BASE_URL
          . "`dirname $0`"/weblib.sh && \
          . $CHECKOUTDIR/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
            logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

        URL="http://127.0.0.1:8000/api/v1/metric/computed/rack_total?arg1=$RACK&arg2=total_power"
        POWER=$(curl -s "$URL" | awk '/total_power/{ print $NF; }')
        STR1="$(printf "%f" $TP)"  # this returns "2000000.000000"
        STR2="$(printf "%f" $POWER)"  # also returns "2000000.000000"
        DEL="$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')"
        if [ "$DEL" = 0 ]; then
            echo "The total power has an expected value: $TP = $POWER. Test PASSED."
            SUCCESSES=$(expr $SUCCESSES + 1)
        else
            echo "Total power does not equal expected value $TP <> $POWER - Test FAILED."
            ERRORS=$(expr $ERRORS + 1)
        fi
    done
done
}

results() {
    SUCCESSES=$1
    ERRORS=$2
    echo "Pass: ${SUCCESSES}/Fails: ${ERRORS}"
}

USR=user1
PSW=user1
SUM_PASS=0
SUM_ERR=0

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 1"
echo "+++++++++++++++++++++++++++++++++++"
TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Time is "$TIME

SAMPLES=(
   20.56
   30.85
   40.41
)
UPS1="epdu101_1_"
UPS2="epdu101_2_"
RACK="8101"
create_nut_config "epdu" "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test1 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 2"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  1004567.34
  1064.34
  1130000
)

UPS1="epdu102_1_"
UPS2="epdu102_2_"
RACK="8108"
create_nut_config "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test2 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 3"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100.999
  80.001
  120.499
)

UPS1="ups103_1_"
UPS2="ups103_2_"
RACK="8116"

create_nut_config "ups"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test3 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 6"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100.501
  80.499
  120.99999999999999
)

UPS1="epdu105_1_"
UPS2="pdu105_1_"
RACK="8134"

create_nut_config "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test6 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 8"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  48
  55
  63
)

UPS1="ups106_1_"
UPS2="pdu106_2_"
RACK="8141"

create_nut_config "ups"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test6 results:"
results $SUCCESSES $ERRORS
SUM_PASS=$(expr ${SUM_PASS} + ${SUCCESSES})
SUM_ERR=$(expr ${SUM_ERR} + ${ERRORS})

echo ""
echo "*** Summary ***"
echo "Passed: $SUM_PASS / Failed: $SUM_ERR"

kill $WEBTESTPID >/dev/null 2>&1 || true

if [ $SUM_ERR = 0 ] ; then
    exit 0
fi

exit 1
