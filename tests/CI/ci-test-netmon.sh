#!/bin/bash

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
# Author(s): Karol Hrdina <karolhrdina@eaton.com>
#
# Description: tests netmon module
#
# Requirements:
#   project is built
#   script is ran as root
#
# Usage $0 [-d]
#   -d  print extra debug output

# TODO (nice to have, if there is nothing to do):
# - grep second iface and do a couple add/del on it as well (lo is special)
# - install brctl and add own iface
# - vlan 
# - perform real parallel-proof locking; now it's kindergarden stuff :)

LOCKFILE=/tmp/ci-test-netmon.lock
if [ -f $LOCKFILE ]; then
    echo -e "Script already running. Stopping."
    exit 1 
fi

# Check for root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run with root permissions." >&2
    exit 1
fi
if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

export BUILDSUBDIR="$CHECKOUTDIR"
[ ! -x "$BUILDSUBDIR/config.status" ] && BUILDSUBDIR="$PWD"
if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "CI-ERROR: Cannot find $BUILDSUBDIR/config.status, did you run configure?"
    echo "CI-ERROR: Search path checked: $CHECKOUTDIR, $PWD"
    exit 1
fi

echo "CI-INFO: Using BUILDSUBDIR='$BUILDSUBDIR' to run the REST API webserver"

### Section: setting necessary variables
DEBUG=
if [ x"$1" = "x-d" ]; then
    DEBUG=1
fi

dsh_file=$(mktemp -p "$CHECKOUTDIR/tests/CI/")
if [ -n "$DEBUG" ]; then
    echo "DEBUG: dsh_file='$dsh_file'" >&2
fi

### Section: actual steps being performed
function cleanup {
    killall malamute
    killall dshell
    killall -9 netmon
    rm -f "$LOCKFILE" #"$dsh_file"
}

touch "$LOCKFILE"
trap cleanup EXIT SIGINT SIGQUIT SIGTERM

killall malamute

malamute "$CHECKOUTDIR/tools/malamute.cfg" &
if [[ $? -ne 0 ]]; then
    echo "malamute didn't start properly" >&2
    exit 1
fi
$CHECKOUTDIR/tools/dshell.sh networks ".*" > "$dsh_file" &
if [[ $? -ne 0 ]]; then
    echo "dshell didn't start properly" >&2
    exit 1
fi
$BUILDSUBDIR/netmon &
if [[ $? -ne 0 ]]; then
    echo "netmon didn't start properly" >&2
    exit 1
fi
sleep 2

# These actions have to be reflected in dsh_file for this test to succeed.
ip addr add 101.25.138.2 dev lo
ip addr add 103.15.3.0/24 dev lo
ip addr add 20.13.5.4/32 dev lo

# Reverting back
ip addr del 101.25.138.2 dev lo
ip addr del 103.15.3.0/24 dev lo
ip addr del 20.13.5.4/32 dev lo

file=$(<$dsh_file) # `cat file` for non-bash shell

# Actual dshell output format:
#sender=NETMON subject=add content=
#D: 15-01-21 08:37:04 NETDISC_MSG_AUTO_ADD:
#D: 15-01-21 08:37:04     name='lo'
#D: 15-01-21 08:37:04     ipver=0
#D: 15-01-21 08:37:04     ipaddr='101.25.138.2'
#D: 15-01-21 08:37:04     prefixlen=32
#D: 15-01-21 08:37:04     mac='00:00:00:00:00:00'


# TODO (later, if many regexps are done, we could use ${var/needle/replace} construct)
#re_tmpl=".*sender=NETMON subject=##SUBJECT## content=.*?##MSGTYPE##.*?name='##IFNAME##'.*?ipver=##IPVER##.*?ipaddr='##IPADDR##'.*?prefixlen=##PREFIXLEN##"

# 101.25.138.2 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi
re=".*sender=NETMON subject=del content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi

# 103.15.3.0/24 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi

# 20.13.5.4/32 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$file" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$file"
    exit 1
fi

exit 0
