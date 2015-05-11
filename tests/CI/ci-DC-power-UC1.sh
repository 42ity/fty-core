#!/bin/bash -ux

#
# requirements:
#   Must run as root (nut configuration)
#   INSTALLDIR should be set as the installation directory (make DESTDIR install)
#

SQL_INIT="initdb.sql"
SQL_LOAD="ci-DC-power-UC1.sql"
XML_TNTNET="tntnet.xml"

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"


# Disable intermediate failures due to CURL (there are currently too few tests
# here to care), and the weblib infrastructure like test_it()/print_result()
# is not used here yet anyway
TESTWEB_CURLFAIL=no

[ "x$INSTALLDIR" = "x" ] && INSTALLDIR=$CHECKOUTDIR/Installation
logmsg_info "Installation directory : $INSTALLDIR"

CFGDIR=""
for cfgd in "/etc/ups" "/etc/nut"; do
    if [ -d "$cfgd" ] ; then
        CFGDIR="$cfgd"
        break
    fi
done
if [ "$CFGDIR" = "" ] ; then
    die "NUT config dir not found"
fi

UPS1="UPS1-US477"
UPS2="UPS2-US477"
UPS3="UPS3-US477"

# Fill the .dev file for UPS parameters
# arg1 : UPS name (for .dev file)
# arg2 : Realpower value
create_ups_device(){
    DEVNAME="$1"
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
" > $CFGDIR/${DEVNAME}.dev
}

# Create a NUT config for 3 dummy UPS
create_nut_config() {
    logmsg_info "creating nut config"
    echo "MODE=standalone" > $CFGDIR/nut.conf 

    echo "[$UPS1]
driver=dummy-ups
port=$UPS1.dev
desc=\"dummy-ups 1 in dummy mode\" 

[$UPS2]
driver=dummy-ups
port=$UPS2.dev
desc=\"dummy-ups 2 in dummy mode\" 

[$UPS3]
driver=dummy-ups
port=$UPS3.dev
desc=\"dummy-ups 3 in dummy mode\" " > $CFGDIR/ups.conf

create_ups_device $UPS1 1500
create_ups_device $UPS2 700
create_ups_device $UPS3 1200

    chown nut:root $CFGDIR/*.dev
    logmsg_info "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    systemctl start nut-server
    logmsg_info "waiting for a while"
    sleep 15
}

# drop and fill the database
fill_database(){
    if [ -f "$CHECKOUTDIR/tools/$SQL_INIT" ] ; then
        loaddb_file "$CHECKOUTDIR/tools/$SQL_INIT"
    else
        die "$SQL_INIT not found"
    fi
    if [ -f "$CHECKOUTDIR/tools/$SQL_LOAD" ] ; then
        loaddb_file "$CHECKOUTDIR/tools/$SQL_LOAD"
    else
        die "$SQL_LOAD not found"
    fi
}

# start built daemons as a subprocess
start_bios_daemons(){
    # Kill existing process
    for d in db-ng agent-nut netmon driver-nmap ; do
        killall -KILL $d lt-$d || true
    done
    # start db-ng
    for d in db-ng agent-nut netmon driver-nmap ; do
    if [ -x $INSTALLDIR/usr/local/bin/$d ] ; then
        $INSTALLDIR/usr/local/bin/$d &
    else
        if [ -x ${BUILDSUBDIR}/$d ] ; then
            ${BUILDSUBDIR}/$d &
        else
            die "Can't find $d"
        fi
    fi
    done

    # wait a bit
    echo "Sleeping after daemon startup..."
    sleep 15
}

# start tntnet in order to make REST API request
start_tntnet(){
    # Kill existing process
    killall -KILL tntnet || true
    # Mod xml file and start tntnet
    if [ -f "$CHECKOUTDIR/src/web/$XML_TNTNET" ] ; then
        cp "$CHECKOUTDIR/src/web/$XML_TNTNET" "$SCRIPTDIR/$XML_TNTNET"
        sed -i '$ d' "$SCRIPTDIR/$XML_TNTNET"
        echo "<dir>$CHECKOUTDIR/src/web</dir>" >> "$SCRIPTDIR/$XML_TNTNET"
        echo "<compPath><entry>$BUILDSUBDIR/.libs</entry></compPath>" >> "$SCRIPTDIR/$XML_TNTNET"
        echo "</tntnet>" >> "$SCRIPTDIR/$XML_TNTNET"
        tntnet -c "$SCRIPTDIR/$XML_TNTNET" &
    else
        logmsg_error "$XML_TNTNET not found"
        stop_processes
        exit 1
    fi
}

# stop all processes launched in the script
stop_processes(){
    for d in db-ng agent-nut netmon driver-nmap ; do
        killall -KILL $d lt-$d || true
    done
    killall -KILL tntnet || true
}

for d in mysql saslauthd malamute ; do
    systemctl start $d
done

create_nut_config
fill_database
start_bios_daemons
start_tntnet

logmsg_info "starting the test"

# Get first rack total power
RACK_TOTAL_POWER1_CONTENT="`api_get_content '/metric/computed/rack_total?arg1=477002&arg2=total_power'`"
RACK_TOTAL_POWER1=$(echo "$RACK_TOTAL_POWER1_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)

RACK_TOTAL_POWER2_CONTENT="`api_get_content '/metric/computed/rack_total?arg1=477003&arg2=total_power'`"
RACK_TOTAL_POWER2=$(echo "$RACK_TOTAL_POWER2_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)

RACK_TOTAL_POWER3_CONTENT="`api_get_content '/metric/computed/rack_total?arg1=477004&arg2=total_power'`"
RACK_TOTAL_POWER3=$(echo "$RACK_TOTAL_POWER3_CONTENT" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)

DATACENTER_POWER_CONTENT="`api_get_content '/metric/computed/datacenter_indicators?arg1=477000&arg2=power'`"
DATACENTER_POWER=$(echo "$DATACENTER_POWER_CONTENT" | grep power | sed "s/: /%/" | cut -d'%' -f2)

RACKS_TOTAL_POWER=$(($RACK_TOTAL_POWER1+$RACK_TOTAL_POWER2+$RACK_TOTAL_POWER3))

# Print test data
echo "Rack1 total power :       $RACK_TOTAL_POWER1"
echo "Rack2 total power :       $RACK_TOTAL_POWER2"
echo "Rack3 total power :       $RACK_TOTAL_POWER3"
echo "Sum of rack total power : $RACKS_TOTAL_POWER"
echo "Datacenter power :        $DATACENTER_POWER"

stop_processes

if [ "$DATACENTER_POWER" == "" ] ; then
    logmsg_error "TEST FAILED - No Data"
    exit 1
elif [ "$DATACENTER_POWER" -eq "$RACKS_TOTAL_POWER" ] ; then
    logmsg_info "TEST PASSED"
    exit 0
else
    logmsg_error "TEST FAILED"
    exit 1
fi
