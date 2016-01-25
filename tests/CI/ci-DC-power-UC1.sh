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
#! \file   ci-DC-power-UC1.sh
#  \brief  Tests to validate Datacentre power
#  \author Yves Clauzel <ClauzelYves@Eaton.com>
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#
# requirements:
#   Must run as root (nut configuration)
#   INSTALLDIR should be set as the installation directory (make DESTDIR install)
#

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
# This should have pulled also testlib.sh and testlib-db.sh
. "`dirname $0`/testlib-nut.sh" || CODE=$? die "Can not include testlib-NUT script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"
detect_nut_cfg_dir || CODE=$? die "NUT config dir not found"

# We customize the webserver config for this test
XML_TNTNET="tntnet.xml"

# Bamboo local DESTDIR
[ -n "${INSTALLDIR-}" ] || INSTALLDIR="$CHECKOUTDIR/Installation"
logmsg_info "Installation directory : $INSTALLDIR"

PATH="$BUILDSUBDIR/tools:$CHECKOUTDIR/tools:$INSTALLDIR/libexec/bios:${DESTDIR:-/root}/libexec/bios:/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

set -u
#set -x

# NUT options
UPS1="UPS1-US477"
UPS2="UPS2-US477"
UPS3="UPS3-US477"

# Fill the .dev file for UPS parameters
# arg1 : UPS name (for .dev file)
# arg2 : Realpower value
# Wrapped to populate a file by testlib-nut::create_ups_dev_file()
custom_create_ups_dev_file() {
    DEVNAME="`basename "$1" .dev`"
    WATTS="$2"
    VOLTS="230"
    AMPS="`expr 14 \* $WATTS / $VOLTS / 10`"

    echo "battery.charge: 90
device.type: ups
output.current: $AMPS
output.voltage: ${VOLTS}.0
ups.realpower: $WATTS
ups.temperature: 25
outlet.realpower: 20
ups.load: 10
ups.mfr: MGE UPS SYSTEMS
ups.model: Pulsar Evolution 500
ups.serial: AV2G3300L
ups.status: OL
outlet.1.voltage: 230
outlet.2.voltage: 230
outlet.3.voltage: 230
"
}

custom_create_ups_dev_files() {
    # Called below via testlib-nut::create_nut_config()
    create_ups_dev_file "$NUTCFGDIR/$UPS1" 1500
    create_ups_dev_file "$NUTCFGDIR/$UPS2" 700
    create_ups_dev_file "$NUTCFGDIR/$UPS3" 1200
}

# drop and fill the database
fill_database(){
    test_it "fill_database"
    for SQLFILE in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DC_POWER_UC1" ; do
        if [ -s "$SQLFILE" ]; then
            loaddb_file "$SQLFILE" || die "Error importing $SQLFILE"
        else
            die "`basename "$SQLFILE"` not found"
        fi
    done
    print_result 0
}

# stop all processes launched in the script
stop_bios_daemons(){
    for d in agent-dbstore agent-nut ; do
        killall -KILL $d lt-$d || true
    done
}

stop_tntnet() {
    killall -KILL tntnet || true
}

stop_processes(){
    stop_bios_daemons
    stop_tntnet
}

# start built daemons as a subprocess
start_bios_daemons(){
    stop_bios_daemons
    for d in agent-dbstore agent-nut ; do
        test_it "start_bios_daemons:$d"
        if [ -x "$INSTALLDIR/usr/local/bin/$d" ] ; then
            "$INSTALLDIR/usr/local/bin/$d" &
            print_result $?
        else
            if [ -x "${BUILDSUBDIR}/$d" ] ; then
                "${BUILDSUBDIR}/$d" &
                print_result $?
            else
                print_result 127
                die "Can't find $d"
            fi
        fi
    done

    # wait a bit
    logmsg_info "Sleeping after daemon startup..."
    sleep 15
}

# start tntnet in order to make REST API request
start_tntnet(){
    stop_tntnet
    # Mod xml file and start tntnet
    test_it "start_tntnet"
    if [ -s "$CHECKOUTDIR/src/web/$XML_TNTNET" ] ; then
        cp "$CHECKOUTDIR/src/web/$XML_TNTNET" "$SCRIPTDIR/$XML_TNTNET"
        sed -i '$ d' "$SCRIPTDIR/$XML_TNTNET"
        { echo "<dir>$CHECKOUTDIR/src/web</dir>"
          echo "<compPath><entry>$BUILDSUBDIR/.libs</entry></compPath>"
          echo "</tntnet>"
        } >> "$SCRIPTDIR/$XML_TNTNET"
        tntnet -c "$SCRIPTDIR/$XML_TNTNET" &
        print_result $?
    else
        logmsg_error "$XML_TNTNET not found"
        stop_processes
        print_result 127
        exit 1
    fi
}

# Note: this default log filename will be ignored if already set by caller
# ERRCODE is maintained by settraps()
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'stop_processes; exit_summarizeTestlibResults $ERRCODE'

for d in mysql saslauthd malamute ; do
    test_it "systemctl_start_3rdParty:$d"
    systemctl start $d
    print_result $?
done

create_nut_config "" "" -- custom_create_ups_dev_files
fill_database
start_bios_daemons
start_tntnet

# Try to accept the BIOS license on server
accept_license

logmsg_info "starting the test"

# Get first rack total power
test_it "api_get_rackpower-1"
RACK_TOTAL_POWER1_CONTENT="`api_get_json '/metric/computed/rack_total?arg1=477002&arg2=total_power'`" && \
RACK_TOTAL_POWER1=$(echo "$RACK_TOTAL_POWER1_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
[ $? = 0 ] && [ -n "$RACK_TOTAL_POWER1" ] && [ -n "$RACK_TOTAL_POWER1_CONTENT" ]
print_result $?

test_it "api_get_rackpower-2"
RACK_TOTAL_POWER2_CONTENT="`api_get_json '/metric/computed/rack_total?arg1=477003&arg2=total_power'`" && \
RACK_TOTAL_POWER2=$(echo "$RACK_TOTAL_POWER2_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
[ $? = 0 ] && [ -n "$RACK_TOTAL_POWER2" ] && [ -n "$RACK_TOTAL_POWER2_CONTENT" ]
print_result $?

test_it "api_get_rackpower-3"
RACK_TOTAL_POWER3_CONTENT="`api_get_json '/metric/computed/rack_total?arg1=477004&arg2=total_power'`" && \
RACK_TOTAL_POWER3=$(echo "$RACK_TOTAL_POWER3_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
[ $? = 0 ] && [ -n "$RACK_TOTAL_POWER3" ] && [ -n "$RACK_TOTAL_POWER3_CONTENT" ]
print_result $?

test_it "api_get_dcpower"
DATACENTER_POWER_CONTENT="`api_get_json '/metric/computed/datacenter_indicators?arg1=477000&arg2=power'`" && \
DATACENTER_POWER=$(echo "$DATACENTER_POWER_CONTENT" | grep power | sed "s/: /%/" | cut -d'%' -f2)
[ $? = 0 ] && [ -n "$DATACENTER_POWER" ] && [ -n "$DATACENTER_POWER_CONTENT" ]
print_result $? "No DC data"

test_it "calculate_racks_totalpower"
RACKS_TOTAL_POWER=$(($RACK_TOTAL_POWER1+$RACK_TOTAL_POWER2+$RACK_TOTAL_POWER3))
[ $? = 0 ] && [ -n "$RACKS_TOTAL_POWER" ]
print_result $?

# Print test data
logmsg_info "Rack1 total power :       $RACK_TOTAL_POWER1"
logmsg_info "Rack2 total power :       $RACK_TOTAL_POWER2"
logmsg_info "Rack3 total power :       $RACK_TOTAL_POWER3"
logmsg_info "Sum of rack total power : $RACKS_TOTAL_POWER"
logmsg_info "Datacenter power :        $DATACENTER_POWER"

stop_processes

test_it "DCpower==RacksTotalPower"
[ "$DATACENTER_POWER" -eq "$RACKS_TOTAL_POWER" ]
print_result $?

# The trap-handler should display the summary (if any)
exit
