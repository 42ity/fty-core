#!/bin/bash -ux

#
# requirements:
#   Must run as root (nut configuration)
#   CHECKOUTDIR should be set as core directory
#   INSTALLDIR should be set as the installation directory (make DESTDIR install)
#

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=$PWD
echo "Will try to run test in $CHECKOUTDIR"

[ "x$INSTALLDIR" = "x" ] && INSTALLDIR=$CHECKOUTDIR/Installation
echo "Installation directory : $INSTALLDIR"

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

UPS1="UPS1-US477"
UPS2="UPS2-US477"
UPS3="UPS3-US477"

# Fill the .dev file for UPS parameters
# arg1 : UPS name (for .dev file)
# arg2 : Realpower value
create_ups_device(){
    echo "battery.charge: 90
device.type: ups
output.current: 0.00
output.voltage: 230.0
ups.realpower: $2
ups.temperature: 25
outlet.realpower: 20
ups.load: 10
ups.mfr: MGE UPS SYSTEMS
ups.model: Pulsar Evolution 500
ups.serial: AV2G3300L
ups.status: OL
outlet.1.voltage: 220
outlet.2.voltage: 220
outlet.3.voltage: 220
" > $CFGDIR/$1.dev
}

# Create a NUT config for 3 dummy UPS
create_nut_config() {
    echo "creating nut config"
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
    echo "restart NUT server"
    systemctl stop nut-server
    systemctl stop nut-driver
    systemctl start nut-server
    echo "waiting for a while"
    sleep 15
}

# drop and fill the database
fill_database(){
    if [ -f $CHECKOUTDIR/tools/initdb.sql ] ; then
        mysql < $CHECKOUTDIR/tools/initdb.sql
    else
        echo "initdb.sql not found"
        exit 1
    fi
    if [ -f $CHECKOUTDIR/tests/CI/datacenter_power.sql ] ; then
        mysql < $CHECKOUTDIR/tests/CI/datacenter_power.sql
    else
        echo "datacenter_power.sql not found"
        exit 1
    fi
}

# start simple as a subprocess
start_simple(){
    # Kill existing process
    killall -9 simple netmon driver-nmap
    # start simple
    if [ -f $INSTALLDIR/usr/local/bin/simple ] ; then
        $INSTALLDIR/usr/local/bin/simple &
    else
        echo "Can't find simple"
        exit 1
    fi
    # wait a bit
    sleep 15
}

# start tntnet in order to make REST API request
start_tntnet(){
    # Kill existing process
    killall -9 tntnet
    # start tntnet
    if [ -f $CHECKOUTDIR/tests/CI/datacenter_power.xml ] ; then
        tntnet -c $CHECKOUTDIR/tests/CI/datacenter_power.xml &
    else
        echo "Can't find tntnet.xml.example"
        stop_processes
        exit 1
    fi
}

# stop all processes launched in the script
stop_processes(){
    killall -9 simple netmon driver-nmap tntnet
}

create_nut_config
fill_database
start_simple
start_tntnet

echo "starting the test"

# Get first rack total power
RACK_TOTAL_POWER1=$(curl "http://localhost:8000/api/v1/metric/computed/rack_total?arg1=477002&arg2=total_power" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
RACK_TOTAL_POWER2=$(curl "http://localhost:8000/api/v1/metric/computed/rack_total?arg1=477003&arg2=total_power" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)
RACK_TOTAL_POWER3=$(curl "http://localhost:8000/api/v1/metric/computed/rack_total?arg1=477004&arg2=total_power" | grep total_power | sed "s/: /%/" | cut -d'%' -f2)

DATACENTER_POWER=$(curl "http://localhost:8000/api/v1/metric/computed/datacenter_indicators?arg1=477000&arg2=power" | grep power | sed "s/: /%/" | cut -d'%' -f2)

RACKS_TOTAL_POWER=$(($RACK_TOTAL_POWER1+$RACK_TOTAL_POWER2+$RACK_TOTAL_POWER3))

# Print test data
echo "Rack1 total power :       $RACK_TOTAL_POWER1"
echo "Rack2 total power :       $RACK_TOTAL_POWER2"
echo "Rack3 total power :       $RACK_TOTAL_POWER3"
echo "Sum of rack total power : $RACKS_TOTAL_POWER"
echo "Datacenter power :        $DATACENTER_POWER"

if [ $DATACENTER_POWER -eq $RACKS_TOTAL_POWER ] ; then
    echo "TEST PASSED"
    stop_processes
    exit 0
else
    echo "TEST FAILED"
    stop_processes
    exit 1
fi
