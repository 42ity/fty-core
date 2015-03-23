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
# Author(s): 
# Karol Hrdina <karolhrdina@eaton.com>
# Radomir Vrajik <radomirvrajik@eaton.com>
#
# Description: tests netmon module
# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***
# ***** PREREQUISITES *****
    # ***   project is built ***
    # ***   script is ran as root ***
    # ***   BIOS processes (malamut, db-ng, nut, netmon) are running on SUT ***

# TODO (nice to have, if there is nothing to do):
# - grep second iface and do a couple add/del on it as well (lo is special)
# - install brctl and add own iface
# - vlan 
# - perform real parallel-proof locking; now it's kindergarden stuff :)

# ***** INIT *****
TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Start time is "$TIME.
    # *** is system running? ***
LOCKFILE=/tmp/ci-test-netmon.lock
if [ -f $LOCKFILE ]; then
    echo -e "Script already running. Stopping."
    exit 1 
fi

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)
    # *** required SUT port and SUT name ***
SUT_PORT="2206"
SUT_NAME="root@debian.roz.lab.etn.com"
    # *** temporary dsh file ***
DSH_FILE=/tmp/temp
ERRORS=0
SUCCESSES=0
# ***** FUNCTIONS *****
    # *** stop  dshell process and delete LOCKFILE ***
function cleanup {
    ssh -p $SUT_PORT $SUT_NAME "killall dshell"
    rm -f "$LOCKFILE" #"$DSH_FILE"
}

# ***** LOCK THE RUNNING SCRIPT, SET trap FOR EXIT SIGNALS *****
    # *** lock the script with creating $LOCKFILE ***
touch "$LOCKFILE"
    # *** call cleanup() when some of te signal appears *** 
trap cleanup EXIT SIGINT SIGQUIT SIGTERM

    # ***  start dshell on SUT ***
ssh -p $SUT_PORT $SUT_NAME "/usr/bin/dshell ipc://@/malamute 1000 mshell networks .* > /tmp/temp &"
# start was successfull?
if [[ $? -ne 0 ]]; then
    echo "ERROR: dshell didn't start properly" >&2
    echo "TEST FAILED."
    exit 1
fi
sleep 2

# ***** CREATE SOME CHANGES IN THE NETWORK TOPOLOGY *****
    # *** These actions have to be reflected in DSH_FILE for this test to succeed." ***
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr add 101.25.138.2 dev lo" 2>/dev/null
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr add 103.15.3.0/24 dev lo"
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr add 20.13.5.4/32 dev lo"

# Reverting back
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr del 101.25.138.2 dev lo" 2>/dev/null
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr del 103.15.3.0/24 dev lo"
ssh -p $SUT_PORT $SUT_NAME "sudo ip addr del 20.13.5.4/32 dev lo"

# ***** GET THE DATA SNIFFERED WITH dshell FROM SUT TO MS. *****
    # *** read the data to variable FILE_DATA ***
FILE_DATA=`ssh -p $SUT_PORT root@debian.roz.lab.etn.com "cat /tmp/temp"`

# See at the dshell output format:
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

# ***** TESTCASES *****
# +++++++++++++++++++++++++++++++++++
    # *** Test 1 ***
# +++++++++++++++++++++++++++++++++++

re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 1 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 1 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# +++++++++++++++++++++++++++++++++++
    # *** Test 2 ***
# +++++++++++++++++++++++++++++++++++

re=".*sender=NETMON subject=del content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='101.25.138.2'.*?prefixlen=32"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 2 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 2 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# +++++++++++++++++++++++++++++++++++
    # *** Test 3 ***
# +++++++++++++++++++++++++++++++++++

# 103.15.3.0/24 dev lo
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 3 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 3 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# +++++++++++++++++++++++++++++++++++
    # *** Test 4 ***
# +++++++++++++++++++++++++++++++++++

re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='103.15.3.0'.*?prefixlen=24"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 4 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 4 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# +++++++++++++++++++++++++++++++++++
    # *** Test 5 ***
# +++++++++++++++++++++++++++++++++++

re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_ADD.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 5 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 5 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# +++++++++++++++++++++++++++++++++++
    # *** Test 6 ***
# +++++++++++++++++++++++++++++++++++
re=".*sender=NETMON subject=add content=.*?NETDISC_MSG_AUTO_DEL.*?name='lo'.*?ipver=0.*?ipaddr='20.13.5.4'.*?prefixlen=32"
if [[ ! "$FILE_DATA" =~ $re ]]; then
    [ -n "$DEBUG" ] && echo -e "Following regexp:\n$re\n did NOT match in the following text:\n$FILE_DATA"
    echo "Test 6 FAILED."
    ERRORS=$(expr $ERRORS + 1)
    exit 1
fi
echo "Test 6 PASSED."
SUCCESSES=$(expr $SUCCESSES + 1)

# ***** SUMMARY *****
echo "****************************************************"
echo TEST PASSED.
echo "****************************************************"
echo "Passed: $SUCCESSES / Failed: $ERRORS"

TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Finish time is "$TIME.
TIME_END=$(date +%s)
TEST_LAST=$(expr $TIME_END - $TIME_START)
echo "Test lasts "$TEST_LAST" second."

exit 0
