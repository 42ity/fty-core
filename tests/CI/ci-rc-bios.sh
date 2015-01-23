#!/bin/sh -e

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
    if [ -x ~/bin/$1 ] ; then
        /bin/rm -rf ~/$1.log
        nohup ~/bin/$1 >~/$1.log 2>&1 &
        sleep 5
        pidof $1
    else
        echo "ERROR: $1 is missing"
        exit 1
    fi
}

stop() {
    for d in $DAEMONS ; do
       killall $d 2>/dev/null || true
    done
}

start() {
    for d in $DAEMONS ; do
        start_daemon $d
    done
}

usage(){
    echo "Usage: $(basename $0) [options...]
options:
    --stop       stop simple and netmon daemons
    --start      start simple (and it starts netmon)
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
    help)
        usage
        exit 1
        ;;
esac
