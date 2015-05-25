#!/bin/bash

# Copyright (C) 2015 Eaton
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
    # ***   BIOS processes (malamut, agent-dbstore, nut, netmon) are running on SUT ***

# TODO (nice to have, if there is nothing to do):
# - grep second iface and do a couple add/del on it as well (lo is special)
# - install brctl and add own iface
# - vlan 
# - perform real parallel-proof locking; now it's kindergarden stuff :)

# ***** INIT *****
TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Start time is "$TIME.

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)

    # *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        --port-web|--sut-port-web|-wp|-o|--port)
            SUT_WEB_PORT="$2"
            shift 2
            ;;
        --host|--machine|-s|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        --sut-user|-su)
            SUT_USER="$2"
            shift 2
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift 2
            ;;
        -p|--passwd|--bios-passwd)
            BIOS_PASSWD="$2"
            shift 2
            ;;
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            break
            ;;
    esac
done

# default values:
[ -z "$SUT_USER" ] && SUT_USER="root"
[ -z "$SUT_HOST" ] && SUT_HOST="debian.roz.lab.etn.com"
# port used for ssh requests:
[ -z "$SUT_SSH_PORT" ] && SUT_SSH_PORT="2206"
# port used for REST API requests:
if [ -z "$SUT_WEB_PORT" ]; then
    if [ -n "$BIOS_PORT" ]; then
        SUT_WEB_PORT="$BIOS_PORT"
    else
        SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
        [ "$SUT_SSH_PORT" -ge 2200 ] && \
            SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
    fi
fi
# unconditionally calculated values
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

    # *** if used set BIOS_USER and BIOS_PASSWD for tests where it is used:
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="@PASSWORD@"

ERRORS=0
SUCCESSES=0

    # *** is system running? ***
LOCKFILE="`echo "/tmp/ci-test-netmon-vte__${SUT_USER}@${SUT_HOST}:${SUT_SSH_PORT}:${SUT_WEB_PORT}.lock" | sed 's, ,__,g'`"


# ***** FUNCTIONS *****
    # *** stop  dshell process and delete LOCKFILE ***
function cleanup {
    set +e
    sut_run "killall dshell lt-dshell"
    sut_run "rm -f '$DSH_FILE_REMOTE'"
    ### TODO: Should systemd-managed BIOS and other services be stopped?
    #sut_run "killall -9 netmon lt-netmon"
    rm -f "$LOCKFILE" #"$DSH_FILE"
}

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"


# ***** LOCK THE RUNNING SCRIPT, SET trap FOR EXIT SIGNALS *****
if [ -f $LOCKFILE ]; then
    ls -la "$LOCKFILE" >&2
    die "Script already running. Stopping."
fi

    # *** lock the script with creating $LOCKFILE ***
echo $$ > "$LOCKFILE"
    # *** call cleanup() when some of te signal appears *** 
trap cleanup EXIT SIGHUP SIGINT SIGQUIT SIGTERM

logmsg_info "Will use BASE_URL = '$BASE_URL'"

    # *** temporary dsh file ***
DSH_FILE_REMOTE="/tmp/temp-ci-netmon-vte-dshell-$$.log"

mkdir -p "$BUILDSUBDIR/tests/CI" || die "Can't create '$BUILDSUBDIR/tests/CI/'"
DSH_FILE=$(mktemp -p "$BUILDSUBDIR/tests/CI/")
if [ -n "$DEBUG" ]; then
    echo "DEBUG: local DSH_FILE='$DSH_FILE'" >&2
fi

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-driver-netmon bios-agent-nut bios-agent-inventory ; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent bios-driver-netmon bios-agent-nut bios-agent-inventory ; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

    # ***  start dshell on SUT ***
logmsg_info "Starting dshell on the VTE..."
sut_run "/usr/bin/dshell ipc://@/malamute 1000 mshell networks '.*' > '$DSH_FILE_REMOTE'" &
# start was successfull?
if [[ $? -ne 0 ]]; then
    echo "ERROR: dshell didn't start properly" >&2
    echo "TEST FAILED."
    exit 1
fi
sleep 2

# ***** CREATE SOME CHANGES IN THE NETWORK TOPOLOGY *****
    # *** These actions have to be reflected in DSH_FILE_REMOTE for this test to succeed." ***
logmsg_info "Starting IP addressing changes..."

# Clean up if old test was aborted mid-way
sut_run "sudo ip addr del 101.25.138.2 dev lo" 2>/dev/null || true
sut_run "sudo ip addr del 103.15.3.0/24 dev lo" 2>/dev/null || true
sut_run "sudo ip addr del 20.13.5.4/32 dev lo" 2>/dev/null || true

# add addresses using different "ip" command syntaxes
sut_run "sudo ip addr add 101.25.138.2 dev lo 2>/dev/null"
sut_run "sudo ip addr add 103.15.3.0/24 dev lo"
sut_run "sudo ip addr add 20.13.5.4/32 dev lo"

# Reverting back
sut_run "sudo ip addr del 101.25.138.2 dev lo 2>/dev/null"
sut_run "sudo ip addr del 103.15.3.0/24 dev lo"
sut_run "sudo ip addr del 20.13.5.4/32 dev lo"

logmsg_info "Done with IP addressing changes..."
sleep 7

# ***** GET THE DATA SNIFFERED WITH dshell FROM SUT TO MS. *****
    # *** read the data to variable FILE_DATA ***
sut_run "cat $DSH_FILE_REMOTE" > "$DSH_FILE"
FILE_DATA=$(cat "$DSH_FILE")

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

logmsg_info "Beginning regexp tests on remotely collected FILE_DATA contents:
$FILE_DATA
"

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
