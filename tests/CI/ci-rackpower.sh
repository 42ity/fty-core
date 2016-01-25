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
#! \file   ci-rackpower.sh
#  \brief  tests the total rack power
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#
# requirements:
#   Must run as root (nut configuration)
#   bios must be running
#   nut must be installed
#
# TODO: change to use testlib.sh routines (and maybe weblib.sh properly
# for the few REST API / license-enforcing bits) - rewrite for test_it()
# TODO: somehow use ci-test-restapi.sh (include? merge? rewrite this to
# be a test_web.sh scriptlet?) otherwise there is lots of duplicated code
# that may differ in nuances

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
# This should have pulled also testlib.sh and testlib-db.sh
. "`dirname $0`/testlib-nut.sh" || CODE=$? die "Can not include testlib-NUT script library"
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR' (it may be empty but should exist)"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] && [ -n "$DB_RACK_POWER" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build, and BUILDSUBDIR='$BUILDSUBDIR' to run"
detect_nut_cfg_dir || CODE=$? die "NUT config dir not found"

PATH="$BUILDSUBDIR/tools:$CHECKOUTDIR/tools:${DESTDIR:-/root}/libexec/bios:/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

WEBTESTPID=""
AGNUTPID=""
DBNGPID=""
kill_daemons() {
    set +e
    if [ -n "$WEBTESTPID" -a -d "/proc/$WEBTESTPID" ]; then
        logmsg_info "Killing make web-test PID $WEBTESTPID to exit"
        kill -INT "$WEBTESTPID"
    fi
    if [ -n "$AGNUTPID" -a -d "/proc/$AGNUTPID" ]; then
        logmsg_info "Killing agent-nut PID $AGNUTPID to exit"
        kill -INT "$AGNUTPID"
    fi
    if [ -n "$DBNGPID" -a -d "/proc/$DBNGPID" ]; then
        logmsg_info "Killing agent-dbstore PID $DBNGPID to exit"
        kill -INT "$DBNGPID"
    fi

    killall -INT tntnet agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1
    killall      tntnet agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1

    ps -ef | grep -v grep | egrep "tntnet|agent-nut|agent-dbstore" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "tntnet and/or agent-dbstore and/or agent-nut still alive, trying SIGKILL" && \
        { killall -KILL tntnet agent-nut lt-agent-nut agent-dbstore lt-agent-dbstore 2>/dev/null ; exit 1; }

    return 0
}

# Ensure that no processes remain dangling when test completes
settraps "kill_daemons; exit_summarizeTestlibResults"

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

logmsg_info "Spawning the agent-nut server in the background..."
${BUILDSUBDIR}/agent-nut &
AGNUTPID=$!

# These are defined in testlib-db.sh
test_it "initialize_db_rackpower"
loaddb_file "$DB_BASE" && \
loaddb_file "$DB_RACK_POWER"
print_result $? || CODE=$? die "Could not prepare database"

# Let the webserver settle
sleep 5
accept_license

#
# only one parameter - ups.realpower for ups or outlet.realpower for epdu
# is used for the total rack power value counting
#
PARAM1="ups.realpower"
PARAM2="outlet.realpower"

custom_create_ups_dev_file() {
    FILE="$1"
    TYPE=ups
    if basename "$FILE" .dev | grep --silent "epdu" ; then
        TYPE=epdu
    fi
    if [ "$TYPE" = "epdu" ] ; then
        echo "# epdu power sequence file
device.type: epdu
manufacturer: ci-rackpower dummy ePDU
model: `basename "$FILE" .dev`
outlet.realpower: 0
#outlet.1.voltage: 220
#outlet.2.voltage: 220
#outlet.3.voltage: 220
"
    else
        echo "# ups power sequence file
device.type: ups
manufacturer: ci-rackpower dummy UPS
model: `basename "$FILE" .dev`
ups.realpower: 0
outlet.realpower: 0
#battery.charge: 90
"
    fi
}

custom_create_epdu_dev_file() {
    # custom_create_ups_dev_file() defined above handles both device types
    custom_create_ups_dev_file "$@"
}

create_device_definition_file() {
    # This calls the testlib-nut wraper to create the file and handle errors
    # and calls back the custom_create_ups_dev_file() defined above
    create_ups_dev_file "$@"
}

testcase() {
    UPS1="$1"
    UPS2="$2"
    SAMPLES="$3"
    RACK="$4"
    PARAM=""
    logmsg_info "Starting the testcase for $UPS1 and $UPS2 in rack $RACK ..."

    SAMPLESCNT="$((${#SAMPLES[*]} - 1))" # sample counter begins from 0
    LASTPOW=(0 0)
    for UPS in $UPS1 $UPS2 ; do
        for SAMPLECURSOR in $(seq 0 $SAMPLESCNT); do
            # set values
            NEWVALUE="${SAMPLES[$SAMPLECURSOR]}"
            TYPE="$(echo "$UPS"|egrep '^pdu'|wc -l)"
            #echo "TYPE = " $TYPE
            if [[ "$TYPE" -eq 1 ]]; then
               NEWVALUE=0
            fi
            TYPE2="$(echo "$UPS"|egrep '^epdu'|wc -l)"
            test_it "configure_total_power_nut:$RACK:$UPS:$SAMPLECURSOR"
            if [[ "$TYPE2" -eq 1 ]]; then
                set_value_in_ups "$UPS" "$PARAM1" 0 0 || logmsg_info "Note: ePDU can fail to set ups.realpower, it is OK"
                set_value_in_ups "$UPS" "$PARAM2" "$NEWVALUE"
                print_result $?
            else
                set_value_in_ups "$UPS" "$PARAM1" "$NEWVALUE" 0 && \
                set_value_in_ups "$UPS" "$PARAM2" 0
                print_result $?
            fi
            sleep 10  # 10 s is max time for propagating into DB (poll ever 5s in nut actor + some time to process)

            NEWVALUE="${SAMPLES[$SAMPLECURSOR]}"
            case "$UPS" in
            "$UPS1")
                if [[ "$TYPE" -eq 1 ]]; then
                   NEWVALUE=0
                fi
                LASTPOW[0]="$NEWVALUE"
                ;;
            "$UPS2")
                if [[ "$TYPE" -eq 1 ]]; then
                   NEWVALUE=0
                fi
                LASTPOW[1]="$NEWVALUE"
                ;;
            esac

            test_it "verify_total_power_restapi:$RACK:$UPS:$SAMPLECURSOR"
            TP="$(awk -vX=${LASTPOW[0]} -vY=${LASTPOW[1]} 'BEGIN{ print X + Y; }')"
            URL="/metric/computed/rack_total?arg1=$RACK&arg2=total_power"
            POWER="$(api_get "$URL" >/dev/null && echo "$OUT_CURL" | awk '/total_power/{ print $NF; }')"
            STR1="$(printf "%f" "$TP")"  # this returns "2000000.000000"
            STR2="$(printf "%f" "$POWER")"  # also returns "2000000.000000"
            DEL="$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')"
            if [[ "$DEL" -eq 0 ]]; then
                logmsg_info "The total power has an expected value: $TP = $POWER"
                print_result 0
            else
                print_result 1 "Total power does not equal expected value: $TP <> $POWER "
            fi
        done
    done
}

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 1"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
   20.56
   30.85
   40.41
)
UPS1="epdu101_1_"
UPS2="epdu101_2_"
RACK="8101"
create_nut_config "" "$UPS1 $UPS2"
testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"


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
create_nut_config "" "$UPS1 $UPS2"
testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"


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
create_nut_config "$UPS1 $UPS2" ""
testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"


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
create_nut_config "" "$UPS1 $UPS2"
testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"


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
create_nut_config "$UPS1" "$UPS2"
testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"


# The trap-handler should kill_daemons() and display the summary (if any)
exit
