#!/bin/bash
#
# Copyright (C) 2015-2019 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    udhcpc-override.sh
#  \brief   This script overrides "udhcpc"
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details This script overrides "udhcpc" as called by "ifup" and friends
#           which have the command line hardcoded (and to bad defaults).
#           Should be installed into "/usr/local/sbin/udhcpc" to take effect.

# Inspect the caller and do the evil magic below only if it is "ifup" et al.
# If none of the suspects are found in call stack, fulfill original request

if [ "`grep -lw debug /proc/cmdline`" ]; then
    (echo "[`awk '{print $1}' < /proc/uptime`] `date` [$$]: $0 $@"; set) >/dev/console 2>&1
fi

pstree -plsaA $$ | grep -v grep | egrep '(ifup.*|ifdown|ifquery|ifplug.*)' >/dev/null \
    || exec /sbin/udhcpc "$@"

# For "ifup" and friends, fix up the call
echo "WARN: $0 $@: will hack the command-line for proper DHCP support..." >&2

UDHCPC_ARGS=""
UDHCPC_IFACE=""
UDHCPC_OPTS=""
UDHCPC_OPTS_DEFAULT="-b -t 4 -T 6 -a"
if /bin/systemctl is-enabled ntpd ; then
    # The service can be "disabled" or "masked"
    # if users want a manually set clock - do not
    # ask the DHCP client to request and perhaps
    # override that manual setting in this case.
    # TODO: Differentiate somehow if the user also
    # wanted a specific NTP server vs. one from DHCP?
    UDHCPC_OPTS_DEFAULT="$UDHCPC_OPTS_DEFAULT -O ntpsrv"
fi

while [ $# -gt 0 ]; do
    case "$1" in
        -n) ;; # ignore quit-if-not-leased
        -p) UDHCPC_ARGS="$UDHCPC_ARGS $1 $2"; shift;;
        -i) UDHCPC_ARGS="$UDHCPC_ARGS $1 $2"
            UDHCPC_IFACE="$2"
            shift;;
        *) UDHCPC_ARGS="$UDHCPC_ARGS $1" ;;
    esac
    shift
done

# Skip search if we do not have a definite interface or that keyword is missing
if [ -n "$UDHCPC_IFACE" ] && \
   [ -n "`grep udhcpc_opts /etc/network/interfaces /etc/network/interfaces.d/*.conf 2>/dev/null`" ] \
; then
    # Run augtools once to speed up the process
    AUGOUT="`(echo 'match /files/etc/network/interfaces/iface[*]'; echo 'match /files/etc/network/interfaces/iface[*]/udhcpc_opts' ) | augtool -S -I/usr/share/fty/lenses`"
    if [ $? = 0 ] && [ -n "$AUGOUT" ]; then
        AUGOUT_IFACE="`echo "$AUGOUT" | grep " = $UDHCPC_IFACE" | sed 's, = .*$,,'`" && \
        [ -n "$AUGOUT_IFACE" ] && \
        UDHCPC_OPTS="`echo "$AUGOUT" | fgrep "$AUGOUT_IFACE/udhcpc_opts"`" && \
        UDHCPC_OPTS="`echo "$UDHCPC_OPTS" | sed 's,^.*/udhcpc_opts = ,,'`" && \
        echo "INFO: Detected UDHCPC_OPTS='$UDHCPC_OPTS' for interface '$UDHCPC_IFACE'" >&2 && \
        { [ -n "$UDHCPC_OPTS" ] || UDHCPC_OPTS=" " ; } || \
        UDHCPC_OPTS=""
        # The braces above ensure that if the admin configured an empty
        # line with udhcpc_opts in /etc/network/interfaces then we don't
        # fall back to script defaults; but upon an error we should...
    fi
fi

[ -z "$UDHCPC_OPTS" ] && \
    echo "INFO: No UDHCPC_OPTS were detected in /etc/network/interfaces, using script defaults" >&2 && \
    UDHCPC_OPTS="$UDHCPC_OPTS_DEFAULT"

case "$UDHCPC_OPTS" in
    *hostname*) ;; # Something provided already
    *)  # Give hint about my non-default name
        case x"`hostname`" in
            x|xeaton-rc3|xlocalhost*)
                echo "WARNING: Current local host name is '`hostname`', trying to find a better name" >&2
                # Try to generate and apply a MAC-based name, do not save it
                # yet - might do so through DHCP assignment processing though
                interface="$UDHCPC_IFACE" \
                    fty-hostname-setup "" "false"
            ;;
        esac

        case x"`hostname`" in
            x|xeaton-rc3|xlocalhost*)
                echo "WARNING: Current local host name is '`hostname`', so not pushing it to DHCP" >&2 ;;
            *) UDHCPC_OPTS="$UDHCPC_OPTS -x hostname:`hostname`" ;;
        esac
        ;;
esac

echo "INFO: udhcpc command-line was changed to: /sbin/udhcpc $UDHCPC_ARGS $UDHCPC_OPTS" >&2
if [ "`grep -lw debug /proc/cmdline`" ]; then
    echo "[`awk '{print $1}' < /proc/uptime`] `date` [$$]:" \
        "udhcpc command-line was changed to: /sbin/udhcpc $UDHCPC_ARGS $UDHCPC_OPTS" >/dev/console
fi
exec /sbin/udhcpc $UDHCPC_ARGS $UDHCPC_OPTS
