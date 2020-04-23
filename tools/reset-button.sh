#!/bin/sh
#
#   Copyright (c) 2014 - 2020 Eaton
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
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>

EVENT_IN="/dev/input/event0"

# The recognized buttons this service is currently interested in
RESET_DOWN="AQBUAAEAAAA="
RESET_UP="AQBUAAAAAAA="
BACK_DOWN="AQABAAEAAAA="
BACK_UP="AQABAAAAAAA="

# Seconds of consecutive keypress to cause factory-reset
[ -n "$LONG_PRESS_TIME" ] || LONG_PRESS_TIME=5

# Other buttons, not currently used by this daemon
BTNOK_DOWN="AQAcAAEAAAA="
BTNOK_UP="AQAcAAAAAAA="
ARROWUP_DOWN="AQBIAAEAAAA="
ARROWUP_UP="AQBIAAAAAAA="
ARROWDOWN_DOWN="AQBQAAEAAAA="
ARROWDOWN_UP="AQBQAAAAAAA="

log() {
    if [ "$1" = "-c" ]; then
        shift
#        [ -c /dev/kmsg ] && \
        echo "$1" > /dev/kmsg
    fi
    echo "$1" >&2
    logger -p daemon.crit "$1"
}

if [ ! -r "$EVENT_IN" ]; then
    log "Event input device $EVENT_IN is not available, aborting service!"
    ( which systemd-detect-virt >/dev/null 2>&1 ) && \
        log "Current node virtualization state: hypervizor=`systemd-detect-virt -v` / container=`systemd-detect-virt -c`"
    exit 127
fi

log "Begin monitoring Event input device $EVENT_IN for button presses"
while true; do
    key="`head -c 16 "$EVENT_IN" | tail -c 8 | base64`"
    NOW_S="`date -u +%s`"
    NOW="`date -d @${NOW_S}`"
    BTN=""
    for B in ARROWUP ARROWDOWN BTNOK BACK; do for S in DOWN UP; do
        eval BTNSTR='$'"${B}_${S}" 2>/dev/null || true
        if [ "$BTNSTR" = "$key" ]; then
            BTN="${B}_${S}"
            [ "$S" = "DOWN" ] && echo "[@$NOW_S] $NOW: User pressed $B" || echo "[@$NOW_S] $NOW: User released $B"
            break
        fi
    done; done

    case "$key" in
    "$RESET_DOWN")
        log -c "$NOW: User asked for a reboot or factory reset (if held pressed longer than $LONG_PRESS_TIME seconds)"
        RESET_ST="$NOW_S"
        ;;
    "$RESET_UP")
        echo "Checking time interval"
        RESET_END="$NOW_S"
        [ -n "$RESET_ST" ] || continue
        RESET_REAL_TIME="`expr $RESET_END - $RESET_ST`"
        if [ "$RESET_REAL_TIME" -gt "$LONG_PRESS_TIME" ]; then
            log -c "$NOW: Long reset button press - doing factory reset"
            touch /mnt/nand/factory_reset
        else
            log -c "$NOW: Short reset button press - doing normal reset"
        fi
        reboot
        RESET_ST=""
        ;;
    "$BACK_DOWN")
        log -c "$NOW: User asked for diagnostic (if held pressed longer than $LONG_PRESS_TIME seconds)"
        BACK_ST="$NOW_S"
        ;;
    "$BACK_UP")
        log -c "$NOW: Checking time interval"
        BACK_END="$NOW_S"
        [ -n "$BACK_ST" ] || continue
        BACK_REAL_TIME="`expr $BACK_END - $BACK_ST`" || continue
        if [ "$BACK_REAL_TIME" -gt "$LONG_PRESS_TIME" ]; then
            log -c "$NOW: Long back button press - collecting diagnostic information"
            DIAG_PROC_COUNT="`ps auxww | grep -v grep | grep diagnostic-information | wc -l`"
            if [ "$DIAG_PROC_COUNT" = "0" ]; then
                DI=`which diagnostic-information`
                if [ "$DI" = "" ] ; then
                    log -c "$NOW: diagnostic script not found"                    
                else
                    ($DI -u -y; RES=$?; log -c "`date -u`: diagnostic script finished ($RES)"; exit $RES) &
                fi
            else
                log -c "$NOW: another instance of diagnostic script is still running, button press ignored"
            fi
        fi
        BACK_ST=""
        ;;
    *)
        echo "Unhandled event $key $BTN"
        ;;
    esac
done

# Should not get here from our infinite loop
log -c "`date -u`: Apparently, the infinite loop in $0 got aborted somehow"
exit 1
