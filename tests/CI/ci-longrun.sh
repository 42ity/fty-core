#!/bin/bash -u

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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: tests the nut driver and propagation of events


#
# requirements:
#   Must run as root (nut configuration)
#   simple must be running
#   db must be filled
#

SCRIPTDIR=$(dirname $0)
CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
CFGDIR=""
USER=nut
PASSWORD=secret

nut_cfg_dir() {
    for cfgd in "/etc/ups" "/etc/nut"; do
	if [ -d "$cfgd" ] ; then
            CFGDIR="$cfgd"
            break
	fi
    done
    if [ "$CFGDIR" = "" ] ; then
	echo "NUT config dir not found"
	exit 1
    fi
}

set_value_in_ups() {
    local UPS=$(basename $1 .dev)
    local PARAM=$2
    local VALUE=$3

    sed -i -r -e "s/^$PARAM *:.+$/$PARAM: $VALUE/i" $CFGDIR/$UPS.dev
    upsrw -s $PARAM=$VALUE -u $USER -p $PASSWORD $UPS@localhost >/dev/null 2>&1
}

get_value_from_ups() {
    local UPS=$(basename $1 .dev)
    local PARAM=$2
    upsc $UPS $PARAM
}

do_select(){
    echo "$1;" | mysql -u root box_utf8 | tail -n +2
}

create_epdu_dev_file() {
    local FILE=$1
    echo -e \
	"device.type: epdu" \
	"\ndevice.model: A12" \
	"\ndevice.mfr: BIOS" \
	"\ndevice.serial: $(echo $FILE | md5sum | cut -d\  -f 1 )" \
	"\ndevice.description: epdu $(basename $FILE)" \
	"\ndevice.contact: root@bios" \
	"\ndevice.location: server room 10" \
	"\ninput.frequency: 50" \
	"\ninput.load: 23" \
	"\ninput.power: 240" \
	"\noutlet.current: 1.00" \
	"\noutlet.voltage: 230.0" \
	"\noutlet.realpower: 20" \
	"\noutlet.1.current: 1.3" \
	"\noutlet.1.realpower: 25" \
	"\noutlet.1.voltage: 230" \
	"\noutlet.2.current: 1.3" \
	"\noutlet.2.realpower: 25" \
	"\noutlet.2.voltage: 230" \
	"\noutlet.3.current: 1.3" \
	"\noutlet.3.realpower: 25" \
	"\noutlet.3.voltage: 230" \
	"\noutlet.4.current: 1.3" \
	"\noutlet.4.realpower: 25" \
	"\noutlet.4.voltage: 230" \
	> $FILE
}

create_ups_dev_file() {
    local FILE=$1
    echo -e \
	"device.type: ups" \
	"\ndevice.model: B32" \
	"\ndevice.mfr: BIOS" \
	"\ndevice.serial: $(echo $FILE | md5sum | cut -d\  -f 1 )" \
	"\ndevice.description: ups $(basename $FILE)" \
	"\ndevice.contact: root@bios" \
	"\ndevice.location: server room 10" \
	"\nbattery.charge: 90" \
	"\noutput.current: 1.20" \
	"\noutput.voltage: 230.0" \
	"\nups.realpower: 25" \
	"\nups.temperature: 25" \
	"\noutlet.realpower: 20" \
	"\nups.load: 10" \
	"\nups.status: OL" \
	> $FILE
}

create_nut_config() {
    echo "\nMODE=standalone" > $CFGDIR/nut.conf 
    echo -e \
	"\n[epdu101_1_]" \
	"\ndriver=dummy-ups" \
	"\nport=epdu101_1_.dev" \
	"\n" \
	"\n[epdu101_2_]" \
	"\ndriver=dummy-ups" \
	"\nport=epdu101_2_.dev" \
	"\n" \
	"\n[ups103_1_]" \
	"\ndriver=dummy-ups" \
	"\nport=ups103_1_.dev" \
	"\n" \
	"\n[ups103_2_]" \
	"\ndriver=dummy-ups" \
	"\nport=ups103_2_.dev" \
	"\n" \
	> $CFGDIR/ups.conf
    echo -e \
	"\n[$USER]" \
	"\npassword=$PASSWORD" \
	"\nactions=SET" \
	"\ninstcmds=ALL" \
	> $CFGDIR/upsd.users
    create_epdu_dev_file $CFGDIR/epdu101_1_.dev
    create_epdu_dev_file $CFGDIR/epdu101_2_.dev
    create_ups_dev_file  $CFGDIR/ups103_1_.dev
    create_ups_dev_file  $CFGDIR/ups103_2_.dev

    chown nut:root $CFGDIR/*.dev
    echo "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    systemctl start nut-server
    echo "waiting for a while"
    sleep 15
}

random_thing(){
    local DEVICE=$1
    LINES=$(grep -v -E ^device $CFGDIR/$DEVICE | wc -l)
    LINE=$(expr $RANDOM % $LINES + 1)
    grep -v -E ^device $CFGDIR/$DEVICE | sed -n -r -e 's/:.+//g' -e ${LINE}p 
}

new_value() {
    local DEVICE=$(basename $1 .dev)
    local ITEM=$2
    local VALUE=$(get_value_from_ups $DEVICE $ITEM)
    case "$ITEM" in
	ups.status)
	    statuses=("OL" "OB DISCHRG" "OL CHRG" "BYPASS" "OVER")
	    cnt=${#statuses[@]}
	    i=$(expr $RANDOM % $cnt)
	    echo ${statuses[$i]}
	    ;;
	battery.charge)
	    # charge 0 - 100
 	    awk -vVALUE=$VALUE '
                BEGIN{
                   srand();
                   change=(rand() * 20 - 10 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   if( newvalue > 100 ) newvalue = 100;
                   printf( "%.2f\n", newvalue );
                }'
	    ;;
	*.load)
	    # load 0 - 120
 	    awk -vVALUE=$VALUE '
                BEGIN{
                   srand();
                   change=(rand() * 15 - 7.5 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   if( newvalue > 120 ) newvalue = 120;
                   printf( "%.2f\n", newvalue );
                }'
	    ;;
	*)
	    # default only positive number
 	    awk -vVALUE=$VALUE '
                BEGIN{
                   srand();
                   change=(rand() * 15 - 7.5 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   printf( "%.2f\n", newvalue );
                }'
	    ;;
    esac
}

create_random_samples() {
    DEVICES=(epdu101_1_.dev epdu101_2_.dev ups103_1_.dev ups103_2_.dev)
    TOTALTIME=$1
    TIME=0
    while [ $TIME -lt $TOTALTIME ] ; do
	I=$(expr $RANDOM % 4)
	DEVICE=${DEVICES[$I]}
	ITEM=$(random_thing $DEVICE)
	NEWVALUE=$(new_value $DEVICE $ITEM)
	SLEEP=$(expr $RANDOM % 100)
	echo "nut:$DEVICE:$ITEM:$NEWVALUE:$SLEEP"
	set_value_in_ups $DEVICE $ITEM $NEWVALUE
	TIME=$(expr $TIME + $SLEEP)
    done
}


produce_events(){
    MEASUREMENTS=$(do_select "select count(*) from t_bios_measurements")
    LASTCHECK=$(date +%s)
    while read sample
    do
	TYPE=$(cut <<< "$sample" -d:  -f1)
	DEVICE=$(cut <<< "$sample" -d:  -f2)
	ITEM=$(cut <<< "$sample" -d:  -f3)
	VALUE=$(cut <<< "$sample" -d:  -f4)
	SLEEPAFTER=$(cut <<< "$sample" -d:  -f5)
	case "$TYPE" in
	    nut)
		set_value_in_ups $DEVICE $ITEM $VALUE
		echo "$(date +%T) $DEVICE $ITEM = $VALUE"
		;;
	esac
	sleep $SLEEPAFTER
	if expr $(date +%s) \> $LASTCHECK + 300 >/dev/null 2>&1 ; then
	    # 5 min since last check
	    NEWCNT=$(do_select "select count(*) from t_bios_measurements")
	    if [ $NEWCNT = $MEASUREMENTS ] ; then
		# no data flow
		echo "ERROR: nothing appeared in measurement table since last check"
	    fi
	    LASTCHECK=$(date +%s)
	    MEASUREMENTS=$NEWCNT
	fi
    done < $SAMPLEFILE
}

usage() {
    echo "usage: $(basename $0) [options]"
    echo "options:"
    echo "    -h|--help              print this help"
    echo "    --create-samples TIME  create random samles instead of"
    echo "                           running test and print it to STDOUT"
    echo "    -s|--samples FILE      use this sample file [default ci-longrun.data]"
}

ACTION=test
SAMPLEFILE=$SCRIPTDIR/ci-longrun.data

while [ "$#" -gt 0 ] ; do 
    case "$1" in
	--create-samples)
	    TIME=$2
	    ACTION=samples
	    shift 2
	    ;;
	-s|--samples)
	    SAMPLEFILE=$2
	    shift 2
	    ;;
	--help|-h)
	    usage
	    exit 1
	    ;;
    esac
done

nut_cfg_dir
case "$ACTION" in
    samples)
	create_nut_config >/dev/null 2>&1
	create_random_samples "$TIME"
	;;
    test)
	$SCRIPTDIR/ci-rc-bios.sh --stop
	create_nut_config
	$SCRIPTDIR/ci-fill-db.sh
	mysql -u root box_utf8 < $CHECKOUTDIR/tools/power_topology.sql
	$SCRIPTDIR/ci-rc-bios.sh --start
	produce_events
	$SCRIPTDIR/ci-rc-bios.sh --stop
	;;
esac
