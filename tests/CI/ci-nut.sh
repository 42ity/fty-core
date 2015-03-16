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


CFGDIR=""
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

USR=user1
PSW=user1
UPS1="UPS1-LAB"
UPS2="UPS2-LAB"


set_value_in_ups() {
    UPS=$1
    PARAM=$2
    VALUE=$3

    sed -r -e "s/^$PARAM *:.+$/$PARAM: $VALUE/i" <$CFGDIR/$UPS.dev >$CFGDIR/$UPS.new
    mv -f $CFGDIR/$UPS.new $CFGDIR/$UPS.dev
    upsrw -s $PARAM=$VALUE -u $USR -p $PSW $UPS@localhost >/dev/null 2>&1
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
        "\noutlet.1.voltage: 220" \
        "\noutlet.2.voltage: 220" \
        "\nups.load: 10" \
        "\nups.status: OL" \
        > $FILE
}


create_nut_config() {
    echo "creating nut config"
    echo "MODE=standalone" > $CFGDIR/nut.conf 

    echo -e \
        "[$UPS1]" \
        "\ndriver=dummy-ups" \
        "\nport=$UPS1.dev" \
        "\ndesc=\"dummy-pdu in dummy mode\"" \
        "\n" \
        "\n[$UPS2]" \
        "\ndriver=dummy-ups" \
        "\nport=$UPS2.dev" \
        "\ndesc=\"dummy-ups 2 in dummy mode\"" \
        > $CFGDIR/ups.conf

    echo -e \
        "[$USR]" \
        "\npassword=$PSW" \
        "\nactions=SET" \
        "\ninstcmds=ALL" \
        > $CFGDIR/upsd.users

    create_ups_dev_file $CFGDIR/$UPS1.dev
    create_ups_dev_file $CFGDIR/$UPS2.dev

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
            expr $SAMPLE \* 100
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
            SELECT="select count(*) from t_bios_measurement where timestamp >= '$TIME' and value = $(expected_db_value $PARAM $NEWVALUE);"
            #echo $SELECT
            if [ $(do_select "$SELECT") = 1 ]; then
                echo "Looking for $PARAM: $NEWVALUE OK."
                SUCCESSES=$(expr $SUCCESSES + 1)
            else 
                echo "Looking for $PARAM: $NEWVALUE failed."
                echo "    $SELECT"
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
