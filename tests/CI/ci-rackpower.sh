#!/bin/bash
#
# Copyright (C) 2014 Eaton
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
# Author(s): Radomir Vrajik <RadomirVrajik@Eaton.com>,
#
# Description: tests the total rack power


#
# requirements:
#   Must run as root (nut configuration)
#   bios must be running
#   nut must be installed
#


if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="$(dirname $0)"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project

cd $CHECKOUTDIR
if [ ! -f Makefile ] ; then
    autoreconf -vfi
    ./configure
fi
make V=0 web-test-deps
make web-test >/tmp/web-test.log 2>&1 &
WEBTESTPID=$!

DB1="$CHECKOUTDIR/tools/initdb.sql"
DB2="$CHECKOUTDIR/tools/rack_power.sql"
mysql -u root < "$DB1"
mysql -u root < "$DB2"
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
    echo "NUT config dir not found"
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
    echo "creating nut config"
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
    echo "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    systemctl start nut-server
    echo "waiting for a while"
    sleep 15
}

do_select(){
    echo "$1;" | mysql -u root box_utf8 | tail -n +2
}

testcase() {
    UPS1=$1
    UPS2=$2
    SAMPLES=$3
    RACK=$4
    PARAM=
echo "starting the test"
 
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
        URL="http://127.0.0.1:8000/api/v1/metric/computed/rack_total?arg1=$RACK&arg2=total_power"
        POWER=$(curl -s "$URL" | awk '/total_power/{ print $NF; }')
        STR1="$(printf "%f" $TP)"  # this returns "2000000.000000"
        STR2="$(printf "%f" $POWER)"  # also returns "2000000.000000"
        DEL="$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')"
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

kill $WEBTESTPID >/dev/null 2>&1

if [ $SUM_ERR = 0 ] ; then
    exit 0
fi


exit 1
