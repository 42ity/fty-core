#!/bin/sh -u

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
# Author(s): Barbora Stepankova <BarboraStepankova@Eaton.com>,
#            Tomas Halman <TomasHalman@eaton.com>
#
# Description: tests the nut driver and propagation of events


#
# requirements:
#   Must run as root (nut configuration)
#   simple must be running
#   db must be filled
#

#
# list of values in samples
#
PARAMS=("output.current" "output.voltage" "ups.load" "ups.temperature" "outlet.1.voltage" "outlet.2.voltage" "ups.status")

#
# random samples
# number of columns must be equal to number of $PARAMS
# in lines values must be differ at leas of 5% otherways
# the change is not propagetad.
#
SAMPLES=(
  6 250 42 80  201 203 OB
  5 229 64 103 251 252 OL
  3 200 80 -5  201 203 CHRG
)

CFGDIR=/etc/ups
USR=user1
PSW=user1
UPS1="UPS1"
UPS2="UPS2"


set_value_in_ups() {
    UPS=$1
    PARAM=$2
    VALUE=$3

    sed -r -e "s/^$PARAM *:.+$/$PARAM: $VALUE/i" <$CFGDIR/$UPS.dev >$CFGDIR/$UPS.new
    mv -f $CFGDIR/$UPS.new $CFGDIR/$UPS.dev
    upsrw -s $PARAM=$VALUE -u $USR -p $PSW $UPS@localhost >/dev/null 2>&1
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


    echo "# dummy-ups1 example power sequence file
#
# Base is the same as .dev files, generated using:
#  $ upsc ups@host > evolution500.seq
#

battery.charge: 90
device.type: pdu
output.current: 0.00
output.voltage: 230.0
ups.realpower: 25
ups.temperature: 25
outlet.realpower: 20
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
device.type: ups
output.current: 0.00
output.voltage: 230.0
ups.realpower: 25
ups.temperature: 25
outlet.realpower: 20
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

expected_db_value() {
    PARAM="$1"
    SAMPLE="$2"
    case "$PARAM" in
	"ups.status")
	    case "$SAMPLE" in
		"OL")
		    echo "8"
		    ;;
		"OB")
		    echo "16"
		    ;;
		"CHRG")
		    echo "1024"
		    ;;
	    esac
	    ;;
	*)
	    expr $SAMPLE \* 10
	    ;;
    esac
}

NPAR=${#PARAMS[*]}
NSAM=${#SAMPLES[*]}
SAMPLESCNT=$(expr ${NSAM} / ${NPAR} - 1)
PARAMSCNT=$(expr ${NPAR} - 1)
ERRORS=0
SUCCESSES=0

create_nut_config

echo "starting the test"
for UPS in $UPS1 $UPS2 ; do
    for s in $(seq 0 $SAMPLESCNT); do
	SAMPLECURSOR=$(expr $s \* ${NPAR} )
	TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")	
	# set values
	for i in $(seq 0 $PARAMSCNT ); do
	    PARAM=${PARAMS[$i]}
	    NEWVALUE=${SAMPLES[$SAMPLECURSOR+$i]}
	    set_value_in_ups $UPS $PARAM $NEWVALUE
	    sleep 2 # give time to nut dummy driver for change
	    if [ "$(upsc $UPS@localhost $PARAM)" = "$NEWVALUE" ]; then 	
		echo "Parameter $PARAM succesfuly modified. New value: $NEWVALUE"
		SUCCESSES=$(expr $SUCCESSES + 1)
	    else	
		echo "Failed to set $PARAM value to $NEWVALUE in NUT dummy driver."
		ERRORS=$(expr $ERRORS + 1)
	    fi
	done
	sleep 8  # 8s is max time for propagating into DB (poll ever 5s in nut actor + some time to process)
	for i in $(seq 0 $PARAMSCNT ); do
	    PARAM=${PARAMS[$i]}
	    NEWVALUE=${SAMPLES[$SAMPLECURSOR+$i]}
	    SELECT="select count(*) from t_bios_measurements where timestamp >= '$TIME' and value = $(expected_db_value $PARAM $NEWVALUE);"
	    #echo $SELECT
	    if [ $(do_select "$SELECT") = 1 ]; then
		echo "Looking for $PARAM: $NEWVALUE OK."
		SUCCESSES=$(expr $SUCCESSES + 1)
	    else 
		echo "Looking for $PARAM: $NEWVALUE failed."
		ERRORS=$(expr $ERRORS + 1)
	    fi
	done	
    done
done

echo "Pass: ${SUCCESSES}/Fails: ${ERRORS}"
if [ $ERRORS = 0 ] ; then
    exit 0
fi
exit 1
