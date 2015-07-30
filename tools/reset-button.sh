#!/bin/sh

# Simple button handling script to provide reset and factory reset features.

# Copyright (C) 2014-2015 by Eaton
# Author: Michal Hrusecky <MichalHrusecky@eaton.com>


RESET_DOWN="AQBUAAEAAAA="
RESET_UP="AQBUAAAAAAA="

log() {
    echo "$1"
    logger -p daemon.notice "$1"
}

[ -n "$LONG_PRESS_TIME" ] || LONG_PRESS_TIME=5

while true; do
    key="`head -c 16 /dev/input/event0 | tail -c 8 | base64`"
    case "$key" in
    "$RESET_DOWN")
        echo "Asked for factory reset"
        RESET_ST="`date -u +%s`"
        ;;
    "$RESET_UP")
        echo "Checking time interval"
        RESET_END="`date -u +%s`"
        [ -n "$RESET_ST" ] || continue
        RESET_REAL_TIME="`expr $RESET_END - $RESET_ST`"
        if [ $RESET_REAL_TIME -gt $LONG_PRESS_TIME ]; then
            log "Long reset button press - doing factory reset"
            touch /mnt/nand/factory_reset
        else
            log "Short reset button press - doing normal reset"
        fi
        reboot
        RESET_ST=""
        ;;
    *)
        echo "Unhandled command $key"
        ;;
    esac
done
