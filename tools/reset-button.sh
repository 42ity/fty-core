#!/bin/sh

# Simple button handling script to provide reset and factory reset features.

# Copyright (C) 2014-2015 by Eaton
# Author: Michal Hrusecky <MichalHrusecky@eaton.com>


RESET_DOWN="AQBUAAEAAAA="
RESET_UP="AQBUAAAAAAA="
EVENT_IN="/dev/input/event0"

log() {
    echo "$1" >&2
    logger -p daemon.notice "$1"
}

[ -n "$LONG_PRESS_TIME" ] || LONG_PRESS_TIME=5

if [ ! -r "$EVENT_IN" ]; then
    log "Event input device $EVENT_IN is not available, aborting service!"
    ( which systemd-detect-virt >/dev/null 2>&1 ) && \
        log "Current node virtualization state: hypervizor=`systemd-detect-virt -v` / container=`systemd-detect-virt -c`"
    exit 127
fi

while true; do
    key="`head -c 16 "$EVENT_IN" | tail -c 8 | base64`"
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
        if [ "$RESET_REAL_TIME" -gt "$LONG_PRESS_TIME" ]; then
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
