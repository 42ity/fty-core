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
#! \file   ci-longrun.sh
#  \brief  tests the bios system stability for longer period
#  \author Tomas Halman <TomasHalman@Eaton.com>

#
# requirements:
#   Must run as root (nut configuration)
#

### Fixed settings for the test
CFGDIR=""
NUTUSER=nut
NUTPASSWORD=secret
DBUSER=root
DATABASE=box_utf8

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
DB_LOADDIR="$CHECKOUTDIR/database/mysql"

nut_cfg_dir() {
    for cfgd in "/etc/ups" "/etc/nut"; do
        if [ -d "$cfgd" ] ; then
            CFGDIR="$cfgd"
            break
        fi
    done
    if [ "$CFGDIR" = "" ] ; then
        die "NUT config dir not found"
    fi
}

set_value_in_ups() {
    local UPS=$(basename $1 .dev)
    local PARAM=$2
    local VALUE=$3

    sed -i -r -e "s/^$PARAM *:.+$/$PARAM: $VALUE/i" $CFGDIR/$UPS.dev
    upsrw -s $PARAM=$VALUE -u $NUTUSER -p $NUTPASSWORD $UPS@localhost >/dev/null 2>&1
}

get_value_from_ups() {
    local UPS=$(basename $1 .dev)
    local PARAM=$2
    upsc $UPS $PARAM
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
        "\n[$NUTUSER]" \
        "\npassword=$NUTPASSWORD" \
        "\nactions=SET" \
        "\ninstcmds=ALL" \
        > $CFGDIR/upsd.users
    create_epdu_dev_file $CFGDIR/epdu101_1_.dev
    create_epdu_dev_file $CFGDIR/epdu101_2_.dev
    create_ups_dev_file  $CFGDIR/ups103_1_.dev
    create_ups_dev_file  $CFGDIR/ups103_2_.dev

    chown nut:root $CFGDIR/*.dev
    logmsg_info "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    sleep 3
    systemctl start nut-driver
    sleep 3
    systemctl start nut-server
    logmsg_info "waiting for a while"
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
            awk -vVALUE=$VALUE -vSEED=$RANDOM '
                BEGIN{
                   srand(SEED);
                   change=(rand() * 20 - 10 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   if( newvalue > 100 ) newvalue = 100;
                   printf( "%.2f\n", newvalue );
                }'
            ;;
        *.load)
            # load 0 - 120
            awk -vVALUE=$VALUE -vSEED=$RANDOM '
                BEGIN{
                   srand(SEED);
                   change=(rand() * 15 - 7.5 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   if( newvalue > 120 ) newvalue = 120;
                   printf( "%.2f\n", newvalue );
                }'
            ;;
        *)
            # default only positive number
            awk -vVALUE=$VALUE -vSEED=$RANDOM '
                BEGIN{
                   srand(SEED);
                   change=(rand() * 15 - 7.5 )/100
                   newvalue = VALUE + VALUE * change
                   if( newvalue < 0 ) newvalue = 0;
                   printf( "%.2f\n", newvalue );
                }'
            ;;
    esac
}

create_random_samples() {
    local DEVICES=(epdu101_1_.dev epdu101_2_.dev ups103_1_.dev ups103_2_.dev)
    local TOTALTIME=$1
    local FREQ=$2
    local TIME=0
    while [ $TIME -lt $TOTALTIME ] ; do
        I=$(expr $RANDOM % 4)
        DEVICE=${DEVICES[$I]}
        ITEM=$(random_thing $DEVICE)
        NEWVALUE=$(new_value $DEVICE $ITEM)
        SLEEP=$(expr $RANDOM % $FREQ)
        echo "nut:$DEVICE:$ITEM:$NEWVALUE:$SLEEP"
        set_value_in_ups $DEVICE $ITEM $NEWVALUE
        TIME=$(expr $TIME + $SLEEP)
    done
}

produce_events(){
    MEASUREMENTS="`do_select 'select count(*) from t_bios_measurement'`"
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
        sleep $(expr $SLEEPAFTER )
        if expr $(date +%s) \> $LASTCHECK + 300 >/dev/null 2>&1 ; then
            # 5 min since last check
            # check measurement flow
            NEWCNT="`do_select 'select count(*) from t_bios_measurement'`"
            if [ $NEWCNT = $MEASUREMENTS ] ; then
                # no data flow
                logmsg_error "nothing appeared in measurement table since last check ($NEWCNT lines in table)"
            else
                logmsg_info "OK: new measurements ($NEWCNT lines in table)"
            fi
            MEASUREMENTS=$NEWCNT
            # check last 5 min data
            TS6MINAGO="`date '+%s' --date '6 minutes ago'`"
            CNT6MIN="`do_select \"select count(*) from t_bios_measurement where timestamp > ( $TS6MINAGO )\"`"
            if [ "$CNT6MIN" = "0" ] ; then
                # no data flow
                logmsg_error "nothing appeared in measurement table in last 6 minutes"
            else
                logmsg_info "OK: $CNT6MIN new measurements in last 6 minutes"
            fi
            # check servises
            if $SCRIPTDIR/ci-rc-bios.sh --status >/dev/null 2>&1 ; then
                logmsg_info "OK: all services running"
            else
                logmsg_error "some services are not running"
                $SCRIPTDIR/ci-rc-bios.sh --status
            fi
            LASTCHECK=$(date +%s)
        fi
    done < $SAMPLEFILE
}

usage() {
    echo "usage: $(basename $0) [options]"
    echo "options:"
    echo "    -h|--help                   print this help"
    echo "    --create-samples TIME FREQ  create random samles instead of"
    echo "                                running test and print samples to STDOUT."
    echo "                                Events are generated as often as (\$RANDOM % FREQ)."
    echo "                                So many events are generated to take TIME to produce them."
    echo "                                TIME and FREQ are in seconds."
    echo "    -s|--samples FILE           use this sample file [default ci-longrun.data]"
}

ACTION=test
SAMPLEFILE="$SCRIPTDIR/ci-longrun.data"

while [ "$#" -gt 0 ] ; do 
    case "$1" in
        --create-samples)
            TIME=$2
            FREQ=$3
            ACTION=samples
            shift 3
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

if [ "$(id -u)" != 0 ] ; then
    die "must run as root"
fi

nut_cfg_dir
case "$ACTION" in
    samples)
        create_nut_config >/dev/null 2>&1
        create_random_samples "$TIME" "$FREQ"
        ;;
    test)
        $SCRIPTDIR/ci-rc-bios.sh --stop
        create_nut_config
        $SCRIPTDIR/ci-empty-db.sh
        loaddb_file "$DB_LOADDIR"/initdb_ci_patch.sql
        loaddb_file "$DB_LOADDIR"/rack_power.sql
        $SCRIPTDIR/ci-rc-bios.sh --start
        produce_events
        $SCRIPTDIR/ci-rc-bios.sh --stop
        ;;
esac
