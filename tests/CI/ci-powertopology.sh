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
#   simple must be running
#   nut must be running
#   web-test must be running
#   db must be filled
#


if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
echo "CHECKOUTDIR = "$CHECKOUTDIR
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project

cd $CHECKOUTDIR
if [ ! -f Makefile ] ; then
    autoreconf -vfi
    ./configure
fi
make V=0 web-test-deps
make web-test >/tmp/tmp &
WEBTESTPID=$!

#SCRIPTDIR=$CHECKOUTDIR"/tools/CI"
DB1="$CHECKOUTDIR/tools/initdb.sql"
DB2="$CHECKOUTDIR/tools/rack_power.sql"
mysql -u root < "$DB1"
mysql -u root < "$DB2"



#
# only one parameter - ups.realpower is used for the total rack power value counting
#
PARAM1="ups.realpower"
PARAM2="outlet.realpower"
#
# random samples
# in lines values must be differ at least of 5% otherways - 
# in the other case nut won't pass the value through and the change is not propagated.
# earlier than in the next 5 minutes sample
#

# samples used
#SAMPLES=(
#   20
#   30
#)
# config dir for the nut dummy driver parameters allocated in config files
CFGDIR="/etc/ups"
[ -d $CFGDIR ] || CFGDIR="/etc/nut"
if [ ! -d $CFGDIR ] ; then
    echo "NUT config dir not found"
    kill $WEBTESTPID
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

create_nut_config() {
TYPE=$1
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


    echo "# dummy-ups1 example power sequence file
#
# Base is the same as .dev files, generated using:
#  $ upsc ups@host > evolution500.seq
#

battery.charge: 90
device.type: $TYPE
output.current: 0.00
output.voltage: 230.0
ups.realpower: 0
ups.temperature: 25
outlet.realpower: 0
ups.load: 10
ups.mfr: MGE UPS SYSTEMS
ups.model: Pulsar Evolution 500
ups.serial: AV2G3300L
ups.status: OL
outlet.1.voltage: 220
outlet.2.voltage: 220
outlet.3.voltage: 220
" > $CFGDIR/$UPS1.dev

    echo "# dummy-ups2 example power sequence file
#
# Base is the same as .dev files, generated using:
#  $ upsc ups@host > evolution500.seq
#

battery.charge: 90
device.type: $TYPE
output.current: 0.00
output.voltage: 230.0
ups.realpower: 0
ups.temperature: 25
outlet.realpower: 0
ups.load: 10
ups.mfr: MGE UPS SYSTEMS
ups.model: Pulsar Evolution 500
ups.serial: AV2G3300L
ups.status: OL
outlet.1.voltage: 220
outlet.2.voltage: 220
outlet.3.voltage: 220
" > $CFGDIR/$UPS2.dev

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
        sleep 2
        # set values
        NEWVALUE=${SAMPLES[$SAMPLECURSOR]}
        TYPE=`echo $UPS|grep ^pdu|wc -l`
	#echo "TYPE = " $TYPE
        if [ "$TYPE" = 1 ]; then
               NEWVALUE=0
        fi
        TYPE2=`echo $UPS|grep ^epdu|wc -l`
	if [ "$TYPE2 = 1" ]; then
            set_value_in_ups $UPS $PARAM1 0
            set_value_in_ups $UPS $PARAM2 $NEWVALUE
	else
            set_value_in_ups $UPS $PARAM1 $NEWVALUE
            set_value_in_ups $UPS $PARAM2 0
	fi
        sleep 2 # give time to nut dummy driver for change
        sleep 8  # 8s is max time for propagating into DB (poll ever 5s in nut actor + some time to process)
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
        TP=`expr ${LASTPOW[0]} + ${LASTPOW[1]}`
#        POWER=(`curl -s 'http://127.0.0.1:8000/api/v1/metric/computed/rack_total?arg1=8101&arg2=total_power'|grep total_power`)
#        CUR="curl -s 'http://127.0.0.1:8000/api/v1/metric/computed/rack_total?arg1=$RACK&arg2=total_power'|grep total_power"
        URL="http://127.0.0.1:8000/api/v1/metric/computed/rack_total?arg1=$RACK&arg2=total_power"
        POWER=$(curl -s "$URL" | awk '/total_power/{ print $NF; }')
        if [ "$TP" = "$POWER" ]; then
           echo "The total power has an expected value $TP = $POWER. Test PASSED."
           SUCCESSES=$(expr $SUCCESSES + 1)
        else
            echo "$TP does not equal expected value $TP <> $POWER - Test FAILED."
            ERRORS=$(expr $ERRORS + 1)
        fi
    done
done
#SUM_PASS=$(expr $SUM_PASS + SUCCESSES)
#SUM_ERR=$(expr $SUM_ERR= + ERRORS)
}

results() {
SUCCESSES=$1
ERRORS=$2

echo "Pass: ${SUCCESSES}/Fails: ${ERRORS}"
#if [ $ERRORS = 0 ] ; then
#    exit 0
#fi
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
   20
   30
   40
)
UPS1="epdu101_1_"
UPS2="epdu101_2_"
RACK="8101"
create_nut_config "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test1 results:"
results $SUCCESSES $ERRORS
SUM_PASS=`expr ${SUM_PASS} + ${SUCCESSES}`
SUM_ERR=`expr ${SUM_ERR} + ${ERRORS}`
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 2"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100
  106
  113
)

UPS1="epdu102_1_"
UPS2="epdu102_2_"
RACK="8108"
create_nut_config "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test2 rezults:"
results $SUCCESSES $ERRORS
SUM_PASS=`expr ${SUM_PASS} + ${SUCCESSES}`
SUM_ERR=`expr ${SUM_ERR} + ${ERRORS}`
echo "+++++++++++++++++++++++++++++++++++"
echo "Test 3"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100
  80
  120
)

UPS1="ups103_1_"
UPS2="ups103_2_"
RACK="8116"

create_nut_config "ups"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test3 results:"
results $SUCCESSES $ERRORS
SUM_PASS=`expr ${SUM_PASS} + ${SUCCESSES}`
SUM_ERR=`expr ${SUM_ERR} + ${ERRORS}`

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 6"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100
  80
  120
)

UPS1="epdu105_1_"
UPS2="pdu105_1_"
RACK="8134"

create_nut_config "epdu"
testcase $UPS1 $UPS2 $SAMPLES $RACK
echo "Test6 results:"
results $SUCCESSES $ERRORS
SUM_PASS=`expr ${SUM_PASS} + ${SUCCESSES}`
SUM_ERR=`expr ${SUM_ERR} + ${ERRORS}`

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
SUM_PASS=`expr ${SUM_PASS} + ${SUCCESSES}`
SUM_ERR=`expr ${SUM_ERR} + ${ERRORS}`

echo ""
echo "*** Summary ***"
echo "Passed: $SUM_PASS / Failed: $SUM_ERR"
#echo "*******************************************"
#echo "SUMMARY RESULT:"
#results $SUM_PASS $SUM_ERR
#echo "*******************************************"
kill $WEBTESTPID

if [ $SUM_ERR = 0 ] ; then
    exit 0
fi


exit 1
