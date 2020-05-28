#!/bin/bash
# Note: Bash-specific syntax is in use!
#
# Copyright (C) 2014 - 2020 Eaton
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
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#
# requirements:
#   Must run as root (nut configuration)
#

### Fixed settings for the test
NUTUSER=nut
NUTPASSWORD=secret
DBUSER=root
DATABASE=box_utf8

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
. "`dirname $0`/testlib-nut.sh" || CODE=$? die "Can not include testlib-NUT script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"
detect_nut_cfg_dir || CODE=$? die "NUT config dir not found"

custom_create_epdu_dev_file() {
    local FILE="$1"
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
        "\nTIMER: 1800"
}

custom_create_ups_dev_file() {
    local FILE="$1"
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
        "\nTIMER: 1800"
}

random_thing(){
    local DEVICE="$1"
    LINES="$(egrep -v '^device' "$NUTCFGDIR/$DEVICE" | wc -l)"
    LINE="$(($RANDOM % $LINES + 1))"
    egrep -v '^device' "$NUTCFGDIR/$DEVICE" | sed -n -r -e 's/:.+//g' -e "${LINE}p"
}

new_value() {
    local DEVICE="$(basename "$1" .dev)"
    local ITEM="$2"
    local VALUE="$(get_value_from_ups "$DEVICE" "$ITEM")"
    case "$ITEM" in
        ups.status)
            statuses=("OL" "OB DISCHRG" "OL CHRG" "BYPASS" "OVER")
            cnt="${#statuses[@]}"
            i="$(($RANDOM % $cnt))"
            echo "${statuses[$i]}"
            ;;
        battery.charge)
            # charge 0 - 100
            awk -vVALUE="$VALUE" -vSEED="$RANDOM" '
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
            awk -vVALUE="$VALUE" -vSEED="$RANDOM" '
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
            awk -vVALUE="$VALUE" -vSEED="$RANDOM" '
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
    local DEVICES=(epdu101_1.dev epdu101_2.dev ups103_1.dev ups103_2.dev)
    local TOTALTIME="$1"
    local FREQ="$2"
    local TIME=0
    while [[ "$TIME" -lt "$TOTALTIME" ]] ; do
        I="$(($RANDOM % 4))"
        DEVICE="${DEVICES[$I]}"
        ITEM="$(random_thing "$DEVICE")"
        NEWVALUE="$(new_value "$DEVICE" "$ITEM")"
        SLEEP="$(($RANDOM % $FREQ))"
        echo "nut:$DEVICE:$ITEM:$NEWVALUE:$SLEEP"
        set_value_in_ups "$DEVICE" "$ITEM" "$NEWVALUE" 0
        TIME="$(($TIME + $SLEEP))"
    done
}

produce_events(){
    # TODO: Instead of DB, ask REST API
    MEASUREMENTS="`do_select 'select count(*) from t_bios_measurement'`"
    LASTCHECK="$(date +%s)"
    NUMLINE=0
    while IFS=: read TYPE DEVICE ITEM VALUE SLEEPAFTER
    do
        NUMLINE="$((NUMLINE+1))"
        case "$TYPE" in
            nut)
                set_value_in_ups "$DEVICE" "$ITEM" "$VALUE"
                echo "$(date +%T) $DEVICE $ITEM = $VALUE"
                ;;
        esac
        sleep $SLEEPAFTER
        NOW="$(date +%s)"
        if [[ "$NOW" -gt "$(($LASTCHECK + 300))" ]] >/dev/null 2>&1 ; then
            # 5 min since last check
            # check measurement flow
            test_it "check_measurement_flow_since_last_check:line=$NUMLINE:now=$NOW"
            # TODO: Instead of DB, ask REST API
            NEWCNT="`do_select 'select count(*) from t_bios_measurement'`"
            if [[ "$NEWCNT" -eq "$MEASUREMENTS" ]] ; then
                # no data flow
                logmsg_error "nothing appeared in measurement table since last check ($NEWCNT lines in table)"
                print_result 5 "TODO:DATABASE_NO_LONGER_HOLDS_THE_ANSWER"
            else
                logmsg_info "OK: new measurements ($NEWCNT lines in table)"
                print_result 0
            fi
            MEASUREMENTS="$NEWCNT"

            # check last 5 min data
            TS6MINAGO="`date '+%s' --date '6 minutes ago'`"
            test_it "check_measurement_flow_for_last_6min:line=$NUMLINE:now=$NOW"
            # TODO: Instead of DB, ask REST API
            CNT6MIN="`do_select 'select count(*) from t_bios_measurement where timestamp > ( '"${TS6MINAGO}"' )'`"
            if [[ "$CNT6MIN" -eq "0" ]] ; then
                # no data flow
                logmsg_error "nothing appeared in measurement table in last 6 minutes"
                print_result 6 "TODO:DATABASE_NO_LONGER_HOLDS_THE_ANSWER"
            else
                logmsg_info "OK: $CNT6MIN new measurements in last 6 minutes"
                print_result 0
            fi

            # check servises
            test_it "check_service_status:line=$NUMLINE:now=$NOW"
            if "$SCRIPTDIR/"ci-rc-bios.sh --status >/dev/null 2>&1 ; then
                logmsg_info "OK: all services running"
                print_result 0
            else
                logmsg_error "some services are not running:" \
                    "`"$SCRIPTDIR/"ci-rc-bios.sh --status`"
                print_result 1
            fi
            LASTCHECK="$(date +%s)"
        fi
    done < "$SAMPLEFILE"
}

[ -n "${SAMPLEFILE_DEFAULT-}" ] || \
    SAMPLEFILE_DEFAULT="$SCRIPTDIR/../fixtures/ci-longrun.data"
usage() {
    echo "Usage: $(basename $0) [options]"
    echo "Options:"
    echo "    -h|--help                   print this help"
    echo "    --create-samples TIME FREQ  create random samles instead of"
    echo "                                running test and print samples to STDOUT."
    echo "                                Events are generated as often as (\$RANDOM % FREQ)."
    echo "                                So many events are generated to take TIME to produce them."
    echo "                                TIME and FREQ are in seconds."
    echo "    -s|--samples FILE           use this sample file [default $SAMPLEFILE_DEFAULT]"
}

ACTION=test
SAMPLEFILE="$SAMPLEFILE_DEFAULT"
while [ "$#" -gt 0 ] ; do 
    case "$1" in
        --create-samples)
            TIME="$2"
            FREQ="$3"
            ACTION=samples
            shift 3
            ;;
        -s|--samples)
            SAMPLEFILE="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 1
            ;;
    esac
done

# Note: this default log filename will be ignored if already set by caller
# ERRCODE is maintained by settraps()
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'exit_summarizeTestlibResults $ERRCODE'

if [ "$(id -u)" != 0 ] ; then
    die "must run as root"
fi

case "$ACTION" in
    samples)
        create_nut_config "ups103_1 ups103_2" "epdu101_1 epdu101_2" >/dev/null 2>&1
        create_random_samples "$TIME" "$FREQ"
        ;;
    test)
        # Not wrapped with testlib-db routines - this test is in full control
        "$SCRIPTDIR/"ci-rc-bios.sh --stop
        create_nut_config "ups103_1 ups103_2" "epdu101_1 epdu101_2"
        loaddb_rack_power
        "$SCRIPTDIR/"ci-rc-bios.sh --start
        produce_events
        "$SCRIPTDIR/"ci-rc-bios.sh --stop
        ;;
esac

logmsg_warn "This test can fail with data verification errors, see http://jira.mbt.lab.etn.com/browse/BIOS-2285 in this case"

# The trap-handler should display the summary (if any)
exit
