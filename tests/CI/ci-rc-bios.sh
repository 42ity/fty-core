#!/bin/sh

# Copyright (C) 2014-2015 Eaton
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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: starts or stops the $BIOS daemons installed in $HOME

DAEMONS="db-ng driver-nut driver-nmap netmon"

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

[ -z "$BUILDSUBDIR" -o ! -d "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
[ ! -x "$BUILDSUBDIR/config.status" ] && BUILDSUBDIR="$PWD"
if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "Cannot find $BUILDSUBDIR/config.status, did you run configure?"
    echo "Search path: $CHECKOUTDIR, $PWD"
fi

# Simple check for whether sudo is needed to restart saslauthd
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 -o "$CURID" = root ] || RUNAS="sudo"

stop_malamute(){
    # NOTE: This likely needs execution via sudo if user is not root
    $RUNAS systemctl stop malamute || true
    pidof malamute >/dev/null 2>&1 && return 1
    return 0
}

start_malamute(){
    # NOTE1: This likely needs execution via sudo if user is not root
    # NOTE2: This restarts malamute if it is running and config changed;
    # but the service would remain running if config is the same.
    # The two-step routine below is needed for privilege elevation
    # to change the file in /etc/malamute
    rm -f "/tmp/.malamute.$$.cfg" 2>/dev/null || true

    cat > /tmp/.malamute.$$.cfg <<[eof]
# Note: This file was regenerated `date` by $0
#   Apply to the whole broker
server
    timeout = 10000     #   Client connection timeout, msec
    background = 0      #   Run as background process
    workdir = /tmp      #   Working directory for daemon
    verbose = 0         #   Do verbose logging of activity?

#   Apply to the Malamute service
mlm_server
    echo = binding Malamute service to 'ipc://@/malamute'
    bind
        endpoint = ipc://@/malamute
[eof]
    RESULT=$?

    if [ "$RESULT" = 0 -a -d /etc/malamute ]; then
        echo -n 'systemd service unit: malamute '
        RESTART=n
        if [ -s /etc/malamute/malamute.cfg -a -s /tmp/.malamute.$$.cfg ]; then
            if diff -bu /etc/malamute/malamute.cfg /tmp/.malamute.$$.cfg | \
               egrep -v '^(\-\-\-|\+\+\+|[ @]|[\+\-]\#.*regenerated)' \
            ; then
                # "diff" found some lines other than timestamp
                RESTART=y
            fi
        else
            RESTART=y
        fi >/dev/null

        if [ "$RESTART" = n ]; then
            # Config is the same
            echo -n "(config unchanged) "
            pidof malamute >/dev/null 2>&1 || $RUNAS systemctl start malamute
            RESULT=$?
        else
            stop_malamute
            $RUNAS cp /tmp/.malamute.$$.cfg /etc/malamute/malamute.cfg && \
            $RUNAS systemctl start malamute
            RESULT=$?
        fi
        pidof malamute || RESULT=$?
        # copy, start or pidof could fail by this point;
        # otherwise we have RESULT==0 from diff-clause or cp-execution
    fi

    if [ "$RESULT" != "0" ] ; then
        echo "ERROR: failed to (re)start malamute" >&2
    fi

    rm -f /tmp/.malamute.$$.cfg 2>/dev/null
    return $RESULT
}

start_daemon(){
    prefixw=""
    if which ${1} >/dev/null 2>&1; then
        prefixw="`which ${1}`"
        prefixw="`dirname "$prefixw"`"
    fi

    # Apparently from pre-existing logic, an older "prefix" may remain
    for prefix in "$prefix" "${PWD}" "$BUILDSUBDIR" "$CHECKOUTDIR" \
        ~/bin ~/lib ~/libexec ~/lib/bios ~/libexec/bios $prefixw \
    ; do
        if [ -n "$prefix" ] && [ -d "$prefix" -a -x "$prefix/${1}" ]; then
            break
        fi
    done

    if [ -x "${prefix}/${1}" ] ; then
        /bin/rm -rf ~/${1}.log
        nohup "${prefix}/${1}" >~/${1}.log 2>&1 &
        sleep 5
        echo -n "${prefix}/$1 "
        pidof ${1}
        RESULT=$?
        if [ "$RESULT" != "0" ] ; then
            echo "ERROR: failed to start $1" >&2
            exit 1
        fi
    else
        echo "ERROR: $1 is missing" >&2
        exit 1
    fi
}

stop() {
    for d in $DAEMONS ; do
       ( pidof $d >/dev/null 2>&1 && killall $d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d >/dev/null 2>&1 && killall -9 $d 2>/dev/null ) || true
    done
    sleep 1
    # Test successful kills
    for d in $DAEMONS ; do
        pidof $d >/dev/null 2>&1 && \
            echo "ERROR: $d still running (`pidof $d`)" && return 1
    done
    return 0
}

status() {
    RESULT=0
    for d in malamute $DAEMONS ; do
       echo -n "$d "
       if pidof $d >/dev/null 2>&1 ; then
           echo "running (`pidof $d`)"
       else
           echo "stopped"
           RESULT=1
       fi
    done
    return $RESULT
}

start() {
    for d in $DAEMONS ; do
        start_daemon $d
    done
}

usage(){
    echo "Usage: $(basename $0) [options...]
Options:
    --stop       stop BIOS processes
    --start      start BIOS processes (does restart if BIOS is running)
    --status     check whether all processes are running
    --help|-h    print this help"
}

OPERATION=help

while [ $# -gt 0 ] ; do
    case "${1}" in
        -h|--help)
            OPERATION=help
            ;;
        --start)
            OPERATION=start
            ;;
        --stop)
            OPERATION=stop
            ;;
        --status)
            OPERATION=status
            ;;
        *)
            echo "Invalid option $1" 1>&2
            usage
            exit 1
            ;;
    esac
    shift
done

case "$OPERATION" in
    start)
        stop
        start_malamute && \
        start
        exit
        ;;
    stop)
        RESULT=0
        stop || RESULT=$?
        stop_malamute || RESULT=$?
        exit $RESULT
        ;;
    status)
        status
        exit
        ;;
    help)
        usage
        exit 1
        ;;
esac

# Unknown operation
exit 1
