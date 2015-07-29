#!/bin/sh

# Simple button handling script to provide reset and factory reset features.

# Copyright (C) 2014-2015 by Eaton
# Author: Michal Hrusecky <MichalHrusecky@eaton.com>


RESET_DOWN="AQBUAAEAAAA="
RESET_UP="AQBUAAAAAAA="

RESET_TIME=5

while true; do
    key="`head -c 16 /dev/input/event0 | tail -c 8 | base64`"
    case $key in
    $RESET_DOWN)
        echo "Asked for factory reset"
        RESET_ST="`date +%s`"
        ;;
    $RESET_UP)
        echo "Checking time interval"
        RESET_END="`date +%s`"
        [ -n "$RESET_ST" ] || continue
        RESET_REAL_TIME="`expr $RESET_END - $RESET_ST`"
        if [ 0$RESET_REAL_TIME -gt $RESET_TIME ]; then
            echo "Doing factory reset"
            touch /mnt/nand/factory_reset
        else
            echo "Doing normal reset"
        fi
	reboot
        RESET_ST=""
        ;;
    *)
        echo "Unhandled command $key"
        ;;
    esac
done
