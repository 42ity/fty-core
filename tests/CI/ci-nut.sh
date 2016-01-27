#!/bin/bash
# NOTE: Bash or compatible syntax interpreter required in code below
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
. "`dirname $0`/testlib-nut.sh" || CODE=$? die "Can not include testlib-NUT script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"
detect_nut_cfg_dir || CODE=$? die "NUT config dir not found"

set -o pipefail || true
set -u

PATH="$BUILDSUBDIR/tools:$CHECKOUTDIR/tools:${DESTDIR:-/root}/libexec/bios:/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

AGNUTPID=""
AGPWRPID=""
AGLEGMETPID=""
DBNGPID=""
kill_daemons() {
    set +e
    if [ -n "$AGNUTPID" -a -d "/proc/$AGNUTPID" ]; then
        logmsg_info "Killing make agent-nut PID $AGNUTPID to exit"
        kill -INT "$AGNUTPID"
    fi
    if [ -n "$AGPWRPID" -a -d "/proc/$AGPWRPID" ]; then
        logmsg_info "Killing agent-tpower PID $AGPWRPID to exit"
        kill -INT "$AGPWRPID"
    fi
    if [ -n "$AGLEGMETPID" -a -d "/proc/$AGLEGMETPID" ]; then
        logmsg_info "Killing bios-agent-legacy-metrics PID $AGLEGMETPID to exit"
        kill -INT "$AGLEGMETPID"
    fi
    if [ -n "$DBNGPID" -a -d "/proc/$DBNGPID" ]; then
        logmsg_info "Killing agent-dbstore PID $DBNGPID to exit"
        kill -INT "$DBNGPID"
    fi

    killall -INT bios-agent-legacy-metrics agent-tpower lt-agent-tpower agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1
    killall      bios-agent-legacy-metrics agent-tpower lt-agent-tpower agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1

    ps -ef | grep -v grep | egrep "agent-(nut|dbstore|tpower)|legacy-metrics" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "At least one of: bios-agent-legacy-metrics, agent-nut, agent-tpower, agent-dbstore still alive, trying SIGKILL" && \
        { killall -KILL bios-agent-legacy-metrics agent-tpower lt-agent-tpower agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null ; exit 1; }

    return 0
}



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
UPS1="UPS1-LAB"
UPS2="UPS2-LAB"

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
        "\noutlet.count: 2" \
        "\noutlet.1.voltage: 220" \
        "\noutlet.2.voltage: 220" \
        "\nups.load: 10" \
        "\nups.status: OL"
}

expected_db_value() {
    local PARAM="$1"
    local SAMPLE="$2"
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
            #echo "$(($SAMPLE*100))"
            echo "$SAMPLE"
            ;;
    esac
}

# Note: this default log filename will be ignored if already set by caller
# ERRCODE is maintained by settraps()
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'kill_daemons; exit_summarizeTestlibResults $ERRCODE'

logmsg_info "Ensuring that the tested programs have been built and up-to-date"

if [ ! -f "$BUILDSUBDIR/Makefile" ] ; then
    test_it "config-deps"
    ./autogen.sh --nodistclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG}
    print_result $? || CODE=$? die "Could not prepare binaries"
fi
test_it "make-deps"
./autogen.sh ${AUTOGEN_ACTION_MAKE} agent-dbstore agent-nut agent-tpower
print_result $? || CODE=$? die "Could not prepare binaries"

# These are defined in testlib-db.sh
test_it "initialize_db_rackpower"
loaddb_file "$DB_BASE" && \
loaddb_file "$DB_RACK_POWER"
print_result $? || CODE=$? die "Could not prepare database"

# This program is delivered by another repo, should "just exist" in container
logmsg_info "Spawning the bios-agent-legacy-metrics service in the background..."
bios-agent-legacy-metrics ipc://@/malamute legacy-metrics bios METRICS &
[ $? = 0 ] || CODE=$? die "Could not spawn bios-agent-legacy-metrics"
AGLEGMETPID=$!

# TODO: this requirement should later become the REST AGENT
logmsg_info "Spawning the agent-dbstore server in the background..."
${BUILDSUBDIR}/agent-dbstore &
[ $? = 0 ] || CODE=$? die "Could not spawn agent-dbstore"
DBNGPID=$!

logmsg_info "Spawning the agent-nut server in the background..."
${BUILDSUBDIR}/agent-nut &
[ $? = 0 ] || CODE=$? die "Could not spawn agent-nut"
AGNUTPID=$!

logmsg_info "Spawning the agent-tpower service in the background..."
${BUILDSUBDIR}/agent-tpower &
[ $? = 0 ] || CODE=$? die "Could not spawn agent-tpower"
AGPWRPID=$!


NPAR="${#PARAMS[*]}"
NSAM="${#SAMPLES[*]}"
SAMPLESCNT="$(($NSAM/$NPAR - 1))"
PARAMSCNT="$(($NPAR - 1))"

create_nut_config "$UPS1 $UPS2" ""

logmsg_info "Starting the test"
for UPS in $UPS1 $UPS2 ; do
    for s in $(seq 0 $SAMPLESCNT); do
        SAMPLECURSOR="$(($s*$NPAR))"
        # String representation used in SQL query below
        TIME="$(date --utc '+%Y-%m-%d %H:%M:%S')"
        TIMENUM="$(date --utc -d "$TIME" '+%s')"
        # set values
        for i in $(seq 0 $PARAMSCNT); do
            PARAM="${PARAMS[$i]}"
            NEWVALUE="${SAMPLES[$SAMPLECURSOR+$i]}"
            test_it "set_value_in_ups:$TIMENUM:$UPS:$PARAM:$NEWVALUE"
            set_value_in_ups "$UPS" "$PARAM" "$NEWVALUE"
            print_result $? "Failed to set $PARAM value to $NEWVALUE in NUT dummy driver"
        done
        logmsg_debug "Sleeping 3sec to propagate NUT driver changes..."
        sleep 3 # give time to nut dummy driver for change
        logmsg_debug "Sleeping time is over!"

        for i in $(seq 0 $PARAMSCNT); do
            PARAM="${PARAMS[$i]}"
            NEWVALUE="${SAMPLES[$SAMPLECURSOR+$i]}"
            test_it "verify_value_in_ups:$TIMENUM:$UPS:$PARAM:$NEWVALUE"
            OUT="`get_value_from_ups "$UPS" "$PARAM"`"
            if [[ $? = 0 ]] && [[ x"$OUT" = x"$NEWVALUE" ]]; then
                print_result 0
            else
                print_result 1 "Failed to see that we could set $PARAM value to $NEWVALUE in NUT dummy driver"
            fi
        done
        logmsg_debug "Sleeping 8sec to propagate measurements..."
        sleep 8  # 8s is max time for propagating into DB (poll every 5s in nut actor + some time to process)
        logmsg_debug "Sleeping time is over!"

        for i in $(seq 0 $PARAMSCNT); do
            PARAM="${PARAMS[$i]}"
            NEWVALUE="${SAMPLES[$SAMPLECURSOR+$i]}"
            test_it "verify_value_in_db:$TIMENUM:$UPS:$PARAM:$NEWVALUE"
            SELECT='select count(*) from t_bios_measurement where timestamp >= '"UNIX_TIMESTAMP('$TIME') and CAST( ((0.0 + value)*(pow(10,scale))) AS DECIMAL(50,6)) = $(expected_db_value "$PARAM" "$NEWVALUE")"
            # NOTE: The comparison above requires that numbers match up at
            # least when rounded to some same precision. Currently NEWVALUE
            # items are integers, so this is a little concern. Otherwise
            # we'd have to detect periods, count the digits after a dot,
            # and feed that to DECIMAL (totaldigits, afterdot) second param.
            #CAST( ((0.0 + value)*(pow(10,scale))) AS DECIMAL(50,1))
            #echo $SELECT
            OUT="$(do_select "$SELECT")"
            if [[ $? = 0 ]] && [[ "$OUT" -eq 1 ]]; then
                print_result 0
            else
                print_result 2 "Looking for exactly one result failed: got '$OUT' for query: $SELECT"
            fi
        done
    done
done

# The trap-handler should display the summary (if any)
exit
