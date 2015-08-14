#!/bin/sh
#
#   Copyright (c) 2014-2015 Eaton
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   reset-button.sh
#  \brief  Simple button handling script to provide reset and factory reset features.
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>

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
