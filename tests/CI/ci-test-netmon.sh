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

# TODO
#   check privilige || exit
# TODO (nice to have, if there is nothing to do):
# try to grep second iface and do a couple add/del on it as well (lo is special)
# later install brctl and add own iface
# vlan 

### Section: setting necessary variables
DEBUG=
if [ -n "$1" ] && [ "$1" = "-d" ]; then
    DEBUG=1
fi

DIRNAME=$(dirname $0)
topsrc_dir=$(realpath -e "$DIRNAME")
# For those of us with weird operating systems :)
if [ -z "$topsrc_dir" ]; then
    topsrc_dir=$(cd "$DIRNAME" && pwd -P)
    if [ -z "$topsrc_dir" ]; then
        exit 1
    fi
fi
topsrc_dir=${topsrc_dir%/tests/CI}
if [ -n "$DEBUG" ]; then
    echo "DEBUG: topsrc_dir='$topsrc_dir'" >&2
fi

dsh_file=$(mktemp -p "$topsrc_dir/tests/CI/")
if [ -n "$DEBUG" ]; then
    echo "DEBUG: dsh_file='$dsh_file'" >&2
fi

LOCKFILE=/tmp/ci-test-netmon.lock
if [ -f $LOCKFILE ]; then
    echo -e "Script already running!\nWhy don't you wait for the other one to finish to find out the real output of this test. Eh?"
    exit 1 
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

# Note:
# When obs builds the image, binaries (like netmon for e.g.) are naturally put
# into /usr/bin. These are different from what we actually build from latest git snapshot.
# Therefore if we want to invoke these binaries, their location has to be in the front
# of PATH environment variable.
export PATH="$topsrc_dir:$topsrc_dir/tools:$PATH"
if [ -n "$DEBUG" ]; then
    echo "DEBUG: PATH='$PATH'" >&2
fi

killall malamute

malamute "$topsrc_dir/tools/malamute.cfg" &
if [[ $? -ne 0 ]]; then
    echo "malamute didn't start properly" >&2
    exit 1
fi
dshell.sh networks ".*" > "$dsh_file" &
if [[ $? -ne 0 ]]; then
    echo "dshell didn't start properly" >&2
    exit 1
fi
netmon &
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
