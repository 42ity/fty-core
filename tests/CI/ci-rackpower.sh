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
#! \file   ci-rackpower.sh aliased as vte-rackpower.sh
#  \brief  tests the total rack power
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#
# ***** ABBREVIATIONS *****
    # *** SUT - System Under Test - remote server with BIOS
    # *** MS - Management Station - local server with this script
    # *** TRP - Total Rack Power

# ***** DESCRIPTION *****
    # *** The new topology is assets
    # *** test creates defined messages through nut using dummy nut driver
    # *** data contained in the messages pass through nut into DB
    # *** restAPI req. for TRP (arg1="$RACK"'&'arg2=total_power) is sent
    # *** expected value of TRP is compared with the one get from restapi req.
    # *** this way it contains also smoke test of the chain nut->DB->restAPI

# ***** PREREQUISITES for remote VTE/SUT testing *****
    # *** SUT_SSH_PORT should be passed as parameter --port <value>
    # *** it is currently from interval <2206;2209>, for restAPI reguests are generated ports from <8006;8009>
    # *** must run as root without using password 
    # *** BIOS image must be installed and running on SUT 
    # *** upsd.conf, upssched.conf and upsmon.conf are present on SUT in the /etc/nut dir 
    # *** tools directory containing tools/initdb.sql database/mysql/rack_power.sql present on MS for assets
    # *** tests/CI directory (on MS) contains weblib.sh (api_get_json and CURL functions needed) and scriptlib.sh
#
# ***** requirements for local (ci/dev) testing: *****
#   Must run as root (nut configuration)
#   bios must be running
#   nut must be installed
#
# TODO: change to use testlib.sh routines (and maybe weblib.sh properly
# for the few REST API / license-enforcing bits) - rewrite for test_it()
# TODO: somehow use ci-test-restapi.sh (include? merge? rewrite this to
# be a test_web.sh scriptlet?) otherwise there is lots of duplicated code
# that may differ in nuances
# TODO: now that CI and VTE versions are combined back into one body, review
# closer the possible repetitions or possibilities for common code usage
# (e.g. lock-files, trap code)

case "`basename "$0"`" in
    vte*) SUT_IS_REMOTE=yes ;; # Unconditionally remote
    *) [ -z "${SUT_IS_REMOTE-}" ] && SUT_IS_REMOTE="" ;;
          # Determine default SUT_IS_REMOTE when we include script libs
          # or assign SUT-related CLI settings
esac

# Set to "yes" to un-block tests that currently fail (to develop them)
[ -z "${CI_RACKPOWER_NONTRIVIAL-}" ] && CI_RACKPOWER_NONTRIVIAL="no"

# *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp)
            SUT_SSH_PORT="$2"
            SUT_IS_REMOTE=yes
            shift
            ;;
        --port-web|--sut-port-web|-wp)
            SUT_WEB_PORT="$2"
            shift
            ;;
        --host|--machine|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift
            ;;
        --use-https|--sut-web-https)    SUT_WEB_SCHEMA="https"; export SUT_WEB_SCHEMA;;
        --use-http|--sut-web-http)      SUT_WEB_SCHEMA="http"; export SUT_WEB_SCHEMA;;
        --sut-user|-su)
            SUT_USER="$2"
            SUT_IS_REMOTE=yes
            shift
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift
            ;;
        -p|--passwd|--bios-passwd|--password|--bios-password)
            BIOS_PASSWD="$2"
            shift
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift
            ;;
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            break
            ;;
    esac
    shift
done

[ x"${SUT_WEB_SCHEMA-}" = x- ] && SUT_WEB_SCHEMA=""
    # *** default connection parameters values:
case "${SUT_IS_REMOTE}" in
no)
    [ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="http"
    ;;
yes)
    # default values:
    [ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="https"
    [ -z "${SUT_USER-}" ] && SUT_USER="root"
    [ -z "${SUT_HOST-}" ] && SUT_HOST="debian.roz53.lab.etn.com"
    # port used for ssh requests:
    [ -z "${SUT_SSH_PORT-}" ] && SUT_SSH_PORT="2206"
    # port used for REST API requests:
    if [ -z "${SUT_WEB_PORT-}" ]; then
        if [ -n "${BIOS_PORT-}" ]; then
            SUT_WEB_PORT="$BIOS_PORT"
        else
            SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
            [ "${SUT_SSH_PORT-}" -ge 2200 ] && \
                SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
        fi
    fi

    # unconditionally calculated values for current setup
    BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
    ;;
#auto|""|*) ;; ### Defaulted in the script libraries below
esac

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
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build, and BUILDSUBDIR='$BUILDSUBDIR' to run"
detect_nut_cfg_dir || CODE=$? die "NUT config dir not found"

PATH="$BUILDSUBDIR/tools:$CHECKOUTDIR/tools:${DESTDIR:-/root}/libexec/bios:/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

WEBTESTPID=""
AGNUTPID=""
AGPWRPID=""
AGLEGMETPID=""
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

    killall -INT tntnet bios-agent-legacy-metrics agent-nut lt-agent-nut agent-tpower lt-agent-tpower agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1
    killall      tntnet bios-agent-legacy-metrics agent-nut lt-agent-nut agent-tpower lt-agent-tpower agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1

    ps -ef | grep -v grep | egrep "tntnet|agent-(nut|dbstore|tpower)|legacy-metrics" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "At least one of: tntnet, bios-agent-legacy-metrics, agent-dbstore, agent-nut, agent-tpower still alive, trying SIGKILL" && \
        { killall -KILL tntnet bios-agent-legacy-metrics agent-nut lt-agent-nut agent-tpower lt-agent-tpower agent-dbstore lt-agent-dbstore 2>/dev/null ; exit 1; }

    return 0
}

# ***** INIT *****
function cleanup {
    set +e
    rm -f "$LOCKFILE"
}

# Ensure that no processes remain dangling when test completes
if [ "$SUT_IS_REMOTE" = yes ]; then
        # *** create lockfile name ***
        LOCKFILE="`echo "/tmp/ci-test-rackpower-vte__${SUT_USER}@${SUT_HOST}:${SUT_SSH_PORT}:${SUT_WEB_PORT}.lock" | sed 's, ,__,g'`"
        settraps "cleanup; exit_summarizeTestlibResults"
        # *** is system running?
        if [ -f "$LOCKFILE" ]; then
                ls -la "$LOCKFILE" >&2
                die "Script already running. Aborting."
        fi
        # *** lock the script with creating $LOCKFILE
        echo $$ > "$LOCKFILE"

        # TODO: replace by calls to proper rc-bios script
        logmsg_info "Ensuring that needed remote daemons are running on VTE"
        sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-agent-nut bios-agent-inventory bios-agent-cm bios-agent-tpower; do systemctl start $SVC ; done'
        sleep 5
        sut_run 'R=0; for SVC in saslauthd malamute mysql tntnet@bios bios-agent-dbstore bios-agent-nut bios-agent-inventory bios-agent-cm bios-agent-tpower; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done;exit $R' || \
                die "Some required services are not running on the VTE"

        # TODO: The different contents (2ci vs 3vte files to be revised)
        # *** write power rack base test data to DB on SUT
        # These are defined in testlib-db.sh
        test_it "initialize_db_rackpower"
        loaddb_file "$DB_BASE" && \
        LOADDB_FILE_REMOTE_SLEEP=1 loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" && \
        LOADDB_FILE_REMOTE_SLEEP=2 loaddb_file "$DB_RACK_POWER"
        print_result $? || CODE=$? die "Could not prepare database"
else
        settraps "kill_daemons; exit_summarizeTestlibResults"

        logmsg_info "Ensuring that the tested programs have been built and up-to-date"
        if [ ! -f "$BUILDSUBDIR/Makefile" ] ; then
                test_it "config-deps"
                ./autogen.sh --nodistclean --configure-flags \
                    "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
                    ${AUTOGEN_ACTION_CONFIG}
                print_result $? || CODE=$? die "Could not prepare binaries"
        fi
        test_it "make-deps"
        ./autogen.sh ${AUTOGEN_ACTION_MAKE} web-test-deps agent-dbstore agent-nut agent-tpower
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

        logmsg_info "Spawning the tntnet web server in the background..."
        ./autogen.sh --noparmake ${AUTOGEN_ACTION_MAKE} web-test \
                >> ${BUILDSUBDIR}/web-test.log 2>&1 &
        [ $? = 0 ] || CODE=$? die "Could not spawn tntnet"
        WEBTESTPID=$!

        # TODO: this requirement should later become the REST AGENT
        logmsg_info "Spawning the agent-dbstore server in the background..."
        ${BUILDSUBDIR}/agent-dbstore &
        [ $? = 0 ] || CODE=$? die "Could not spawn agent-dbstore"
        DBNGPID=$!

        logmsg_info "Spawning the agent-nut service in the background..."
        ${BUILDSUBDIR}/agent-nut &
        [ $? = 0 ] || CODE=$? die "Could not spawn agent-nut"
        AGNUTPID=$!

        logmsg_info "Spawning the agent-tpower service in the background..."
        ${BUILDSUBDIR}/agent-tpower &
        [ $? = 0 ] || CODE=$? die "Could not spawn agent-tpower"
        AGPWRPID=$!
fi

# Let the webserver settle
sleep 5
accept_license

if [ "$SUT_IS_REMOTE" = yes ]; then
        sut_run 'systemctl restart bios-agent-tpower'
        sut_run 'systemctl restart bios-agent-dbstore'
fi

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
device.mfr: ci-rackpower dummy ePDU
device.model: `basename "$FILE" .dev`
outlet.realpower: 0
#outlet.1.voltage: 220
#outlet.2.voltage: 220
#outlet.3.voltage: 220
"
    else
        echo "# ups power sequence file
device.type: ups
device.mfr: ci-rackpower dummy UPS
device.model: `basename "$FILE" .dev`
ups.mfr: ci-rackpower dummy UPS
ups.model: `basename "$FILE" .dev`
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
    logmsg_info "Starting the testcase for power devices '$UPS1' and '$UPS2' in rack '$RACK' ..."

    SAMPLESCNT="$((${#SAMPLES[*]} - 1))" # sample counter begins from 0
    LASTPOW=(0 0)
    for UPS in $UPS1 $UPS2 ; do
        # count expected value of total power
        for SAMPLECURSOR in $(seq 0 $SAMPLESCNT); do
            # set values
            NEWVALUE="${SAMPLES[$SAMPLECURSOR]}"

            case "$UPS" in
            	ups*)  TYPE="ups";;
            	pdu*)  TYPE="pdu";;
            	epdu*) TYPE="epdu";;
            	*) die "Unknown device name pattern: '$UPS'" ;;
            esac
### It makes sense that a dumb PDU has no measurements...
            if [[ "$TYPE" = "pdu" ]]; then
               NEWVALUE=0
            fi
            test_it "configure_total_power_nut:$RACK:$UPS:$SAMPLECURSOR"
            if [[ "$TYPE" = "epdu" ]]; then
                set_value_in_ups "$UPS" "$PARAM1" 0 0 || logmsg_info "Note: ePDU can fail to set ups.realpower, it is OK"
                set_value_in_ups "$UPS" "$PARAM2" "$NEWVALUE"
                print_result $?
            else
                set_value_in_ups "$UPS" "$PARAM1" "$NEWVALUE" 0 && \
                set_value_in_ups "$UPS" "$PARAM2" 0
                print_result $?
            fi

            case "$UPS" in
            "$UPS1")
                LASTPOW[0]="$NEWVALUE"
                ;;
            "$UPS2")
                LASTPOW[1]="$NEWVALUE"
                ;;
            esac

            logmsg_debug "Sleeping 8sec to propagate measurements..."
            sleep 8  # 8s is max time for propagating into DB (poll ever 5s in nut actor + some time to process)
            logmsg_debug "Sleep time is over!"

            test_it "verify_total_power_restapi:$RACK:$UPS:$SAMPLECURSOR"
            TP="$(awk -vX=${LASTPOW[0]} -vY=${LASTPOW[1]} 'BEGIN{ print X + Y; }')"
            # send restAPI request to find generated value of total power
            URL="/metric/computed/rack_total?arg1=${RACK}&arg2=total_power"
            POWER="$(api_get "$URL" >/dev/null && echo "$OUT_CURL" | awk '/total_power/{ print $NF; }')" || { print_result $? "REST API call failed"; continue; }
            # synchronize format of the expected and generated values of total power
            STR1="$(printf "%f" "$TP")"  # this returns "2000000.000000"
            STR2="$(printf "%f" "$POWER")"  # also returns "2000000.000000"
            # round both numbers and compare them to
            # decide if the test is successfull or failed
            DEL="$(awk -vX=${STR1} -vY=${STR2} 'BEGIN{ print int( 10*(X - Y) - 0.5 ); }')"
            if [[ "$DEL" -eq 0 ]]; then
                logmsg_info "The total power on rack $RACK has an expected value: '$TP' = '$POWER'"
                print_result 0
            else
                print_result 1 "Total power on rack $RACK does not equal expected value: exp '$TP' <> api '$POWER'"
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
UPS1="epdu101_1"
UPS2="epdu101_2"
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
UPS1="epdu102_1"
UPS2="epdu102_2"
RACK="8108"
if [ "$CI_RACKPOWER_NONTRIVIAL" = yes ]; then
    create_nut_config "ups102_1" "$UPS1 $UPS2"
    testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"
else
    logmsg_warn "Test skipped: topology too complex for now"; echo ""
fi

echo "+++++++++++++++++++++++++++++++++++"
echo "Test 3"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100.999
  80.001
  120.499
)
UPS1="ups103_1"
UPS2="ups103_2"
RACK="8116"
if [ "$CI_RACKPOWER_NONTRIVIAL" = yes ]; then
    create_nut_config "$UPS1 $UPS2" "pdu103_1 pdu103_2"
    testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"
else
    logmsg_warn "Test skipped: topology too complex for now"; echo ""
fi


echo "+++++++++++++++++++++++++++++++++++"
echo "Test 6"
echo "+++++++++++++++++++++++++++++++++++"
SAMPLES=(
  100.501
  80.499
  120.99999999999999
)
UPS1="epdu105_1"
UPS2="pdu105_1"
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
UPS1="ups106_1"
UPS2="pdu106_2"
RACK="8141"
if [ "$CI_RACKPOWER_NONTRIVIAL" = yes ]; then
    create_nut_config "$UPS1" "$UPS2 pdu106_1"
    testcase "$UPS1" "$UPS2" "$SAMPLES" "$RACK"
else
    logmsg_warn "Test skipped: topology too complex for now"; echo ""
fi


# The trap-handler should kill_daemons() and display the summary (if any)
exit
