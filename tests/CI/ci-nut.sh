#!/bin/bash
#
# Copyright (C) 2014-2016 Eaton
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
#! \file   ci-nut.sh
#  \brief  tests the nut driver and propagation of events
#  \author Barbora Stepankova <BarboraStepankova@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#
# requirements:
#   Must run as root (nut configuration)
#   simple must be running
#   db must be filled
#

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

set -o pipefail || true
set -u

#
# list of values in samples
#
PARAMS=("output.current" "output.voltage" "ups.load" "ups.temperature" "outlet.1.voltage" "outlet.2.voltage" "ups.status")

#
# random samples
# number of columns must be equal to number of $PARAMS;
# in lines the values must differ by at least 5% otherwise
# the change is not propagated.
#
SAMPLES=(
  6 250 42 80  201 203 OB
  5 229 64 103 251 252 OL
  3 200 80 -5  201 203 CHRG
)

# NUT options
CFGDIR=""
USR=user1
PSW=user1
UPS1="UPS1-LAB"
UPS2="UPS2-LAB"
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
    local UPS="$(basename "$1" .dev)"
    local PARAM="$2"
    local VALUE="$3"

    sed -r -e "s/^$PARAM *:.+"'$'"/$PARAM: $VALUE/i" <"$CFGDIR/$UPS.dev" >"$CFGDIR/$UPS.new" && \
    mv -f "$CFGDIR/$UPS.new" "$CFGDIR/$UPS.dev" || \
        logmsg_error "Could not generate '$CFGDIR/$UPS.dev'"

    case "$UPS" in
        ""|@*) logmsg_error "get_value_from_ups() got no reasonable UPS parameter ('$UPS')"; return 1 ;;
        *@*) ;;
        *)   UPS="$UPS@localhost" ;;
    esac

    upsrw -s "$PARAM=$VALUE" -u "$USR" -p "$PSW" "$UPS" >/dev/null 2>&1
}

get_value_from_ups() {
    local UPS="$(basename "$1" .dev)"
    local PARAM="$2"
    case "$UPS" in
        ""|@*) logmsg_error "get_value_from_ups() got no reasonable UPS parameter ('$UPS')"; return 1 ;;
        *@*) ;;
        *)   UPS="$UPS@localhost" ;;
    esac
    upsc "$UPS" "$PARAM"
}

create_ups_dev_file() {
    local FILE="$1"
    logmsg_debug "create_ups_dev_file($FILE)"
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
        "\noutlet.count: 2" \
        "\noutlet.1.voltage: 220" \
        "\noutlet.2.voltage: 220" \
        "\nups.load: 10" \
        "\nups.status: OL" \
        > "$FILE" || CODE=$? die "create_ups_dev_file($FILE) FAILED ($?)"
}

list_nut_devices() {
    awk '/^\[.+\]/{ print substr($0,2,index($0,"]") - 2); }' < "$CFGDIR/ups.conf"
}

have_nut_target() {
    local STATE="`systemctl show nut-driver.target | egrep '^LoadState=' | cut -d= -f2`"
    if [ "$STATE" = "not-found" ] ; then
        echo N
        return 1
    else
        echo Y
        return 0
    fi
}

stop_nut() {
    if [ "$(have_nut_target)" = Y ] ; then
        systemctl stop nut-server
        systemctl stop "nut-driver@*"
        systemctl disable "nut-driver@*"
    else
        systemctl stop nut-server.service
        systemctl stop nut-driver.service
    fi
    sleep 3
}

start_nut() {
    local ups
    if [ "$(have_nut_target)" = Y ] ; then
        for ups in $(list_nut_devices) ; do
            systemctl enable "nut-driver@$ups" || return $?
            systemctl start "nut-driver@$ups" || return $?
        done
        systemctl start nut-server || return $?
    else
        systemctl start nut-driver.service || return $?
        systemctl start nut-server.service || return $?
    fi
    sleep 3
}

create_nut_config() {
    stop_nut
    test_it "create_nut_config"
    RES=0

    echo "MODE=standalone" > "$CFGDIR/nut.conf" || \
        die "Can not tweak 'nut.conf'"

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
        > "$CFGDIR/ups.conf" || \
        die "Can not tweak 'ups.conf'"

    echo -e \
        "[$USR]" \
        "\npassword=$PSW" \
        "\nactions=SET" \
        "\ninstcmds=ALL" \
        > "$CFGDIR/upsd.users" || \
        die "Can not tweak 'upsd.users'"

    create_ups_dev_file "$CFGDIR/$UPS1.dev" || RES=$?
    create_ups_dev_file "$CFGDIR/$UPS2.dev" || RES=$?

    chown nut:root "$CFGDIR/"*.dev
    logmsg_info "restart NUT server"
    stop_nut || true
    start_nut || RES=$?
    logmsg_info "waiting for a while"
    sleep 10
    print_result $RES
    return $RES
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
            echo "$(($SAMPLE*100))"
            ;;
    esac
}

# Note: this default log filename will be ignored if already set by caller
# ERRCODE is maintained by settraps()
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'exit_summarizeTestlibResults $ERRCODE'

NPAR="${#PARAMS[*]}"
NSAM="${#SAMPLES[*]}"
SAMPLESCNT="$(($NSAM/$NPAR - 1))"
PARAMSCNT="$(($NPAR - 1))"

nut_cfg_dir
create_nut_config

logmsg_info "Starting the test"
for UPS in $UPS1 $UPS2 ; do
    for s in $(seq 0 $SAMPLESCNT); do
        SAMPLECURSOR="$(($s*$NPAR))"
        TIME="$(date --utc '+%Y-%m-%d %H:%M:%S')"
        # set values
        for i in $(seq 0 $PARAMSCNT); do
            PARAM="${PARAMS[$i]}"
            NEWVALUE="${SAMPLES[$SAMPLECURSOR+$i]}"
            test_it "set_value_in_ups:$UPS:$PARAM:$NEWVALUE"
            set_value_in_ups "$UPS" "$PARAM" "$NEWVALUE"
            print_result $?
            sleep 2 # give time to nut dummy driver for change

            test_it "verify_value_in_ups:$UPS:$PARAM:$NEWVALUE"
            OUT="`get_value_from_ups "$UPS" "$PARAM"`"
            if [ $? = 0 ] && [ x"$OUT" = x"$NEWVALUE" ]; then  
                print_result 0
            else
                print_result 1 "Failed to set $PARAM value to $NEWVALUE in NUT dummy driver"
            fi
        done
        sleep 8  # 8s is max time for propagating into DB (poll every 5s in nut actor + some time to process)

        for i in $(seq 0 $PARAMSCNT); do
            PARAM="${PARAMS[$i]}"
            NEWVALUE="${SAMPLES[$SAMPLECURSOR+$i]}"
            test_it "verify_value_in_db:$UPS:$PARAM:$NEWVALUE"
            SELECT='select count(*) from t_bios_measurement where timestamp >= '"UNIX_TIMESTAMP('$TIME') and value = $(expected_db_value "$PARAM" "$NEWVALUE");"
            #echo $SELECT
            if [[ "$(do_select "$SELECT")" -eq 1 ]]; then
                print_result 0
            else
                print_result 2 "Looking for exactly one result failed: $SELECT"
            fi
        done
    done
done

# The trap-handler should display the summary (if any)
exit
