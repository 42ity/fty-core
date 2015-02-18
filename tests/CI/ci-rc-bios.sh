#!/bin/sh

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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: starts or stops the simple daemon installed in $HOME

DAEMONS="db-ng driver-nut driver-nmap netmon"

restart_malamute(){
    systemctl stop malamute || true
    cat >/etc/malamute/malamute.cfg <<[eof]

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
    systemctl start malamute
}

start_daemon(){
    if which ${1} >/dev/null 2>&1; then
        prefix=`which $1`
        prefix=`dirname $prefix`
    fi
    [ \! -x ${PWD}/${1} ]       || prefix="${PWD}"
    [ \! -x ~/bin/$1 ]          || prefix="`cd ~/bin; pwd`"
    [ \! -x ~/lib/$1 ]          || prefix="`cd ~/lib; pwd`"
    [ \! -x ~/libexec/$1 ]      || prefix="`cd ~/libexec; pwd`"
    [ \! -x ~/lib/bios/$1 ]     || prefix="`cd ~/lib/bios; pwd`"
    [ \! -x ~/libexec/bios/$1 ] || prefix="`cd ~/libexec/bios; pwd`"
    if [ -x ${prefix}/$1 ] ; then
        /bin/rm -rf ~/$1.log
        nohup ${prefix}/$1 >~/$1.log 2>&1 &
        sleep 5
        echo -n "${prefix}/$1 "
        pidof $1
        RESULT=$?
        if [ "$RESULT" != "0" ] ; then
            echo "failed to start $1"
            exit 1
        fi
    else
        echo "ERROR: $1 is missing"
        exit 1
    fi
}

stop() {
    for d in $DAEMONS ; do
       pidof $d >/dev/null 2>&1 && killall $d
    done
    sleep 1
    for d in $DAEMONS ; do
       pidof $d >/dev/null 2>&1 && killall -9 $d
    done
}

status() {
    RESULT=0
    for d in $DAEMONS ; do
       echo -n "$d "
       if pidof $d >/dev/null 2>&1 ; then
           echo "running"
       else
           echo "stopped"
           RESULT=1
       fi
    done
    exit $RESULT
}

start() {
    for d in $DAEMONS ; do
        start_daemon $d
    done
}

usage(){
    echo "Usage: $(basename $0) [options...]
options:
    --stop       stop BIOS processes
    --start      start BIOS processes
    --restart    restart BIOS processes
    --status     check wether all processes are running
    --help|-h    print this help"
}

OPERATION=help

while [ $# -gt 0 ] ; do
    case "$1" in
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
        restart_malamute
        start
        ;;
    stop)
        stop
        ;;
    status)
        status
        ;;
    help)
        usage
        exit 1
        ;;
esac
exit 0
