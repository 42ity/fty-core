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

start(){
   set -x
   cd ~/bin
   /bin/rm -rf ~/simple.log
   nohup ./simple >~/simple.log 2>&1 &
   sleep 5
   pidof simple && pidof netmon
}

stop(){
   killall simple 2>/dev/null || true
   sleep 1
   killall netmon 2>/dev/null || true
   sleep 1
   ( pidof simple &>/dev/null && killall -9 simple ) || true
   ( pidof netmon &>/dev/null && killall -9 netmon ) || true
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
