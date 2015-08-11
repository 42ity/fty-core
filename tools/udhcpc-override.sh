#!/bin/bash

# Copyright (C) 2015 Eaton
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
# Author(s): Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description:
# This script overrides "udhcpc" as called by "ifup" and friends
# which have the command line hardcoded (and to bad defaults).
# Should be installed into "/usr/local/sbin/udhcpc" to take effect.

# Inspect the caller and do the evil magic below only if it is "ifup" et al.
# If none of the suspects are found in call stack, fulfill original request
pstree -plsaA $$ | grep -v grep | egrep '(ifup.*|ifdown|ifquery|ifplug.*)' >/dev/null \
    || exec /sbin/udhcpc "$@"

# For "ifup" and friends, fix up the call
echo "WARN: $0 $@: will hack the command-line for proper DHCP support..." >&2

UDHCPC_ARGS=""
UDHCPC_IFACE=""
UDHCPC_OPTS=""
UDHCPC_OPTS_DEFAULT="-b -t 4 -T 6 -O ntpsrv"

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

if [ -n "$UDHCPC_IFACE" ]; then
    # Run augtools once to speed up the process
    AUGOUT="`(echo 'match /files/etc/network/interfaces/iface[*]'; echo 'match /files/etc/network/interfaces/iface[*]/udhcpc_opts' ) | augtool`"
    if [ $? = 0 ] && [ -n "$AUGOUT" ]; then
        AUGOUT_IFACE="`echo "$AUGOUT" | grep " = $UDHCPC_IFACE" | sed 's, = .*$,,'`" && \
        [ -n "$AUGOUT_IFACE" ] && \
        UDHCPC_OPTS="`echo "$AUGOUT" | fgrep "$AUGOUT_IFACE/udhcpc_opts"`" && \
        UDHCPC_OPTS="`echo "$UDHCPC_OPTS" | sed 's,^.*/udhcpc_opts = ,,'`" && \
        echo "INFO: Detected UDHCPC_OPTS='$UDHCPC_OPTS' for interface '$UDHCPC_IFACE'" >&2 && \
        [ -z "$UDHCPC_OPTS" ] && UDHCPC_OPTS=" " || \
        UDHCPC_OPTS=""
    fi
fi

[ -z "$UDHCPC_OPTS" ] && \
    echo "INFO: No UDHCPC_OPTS were detected in /etc/network/interfaces, using script defaults" >&2 && \
    UDHCPC_OPTS="$UDHCPC_OPTS_DEFAULT"

case "$UDHCPC_OPTS" in
    *hostname*) ;;
    *)  # Give hint about my non-default name
        [ x"`hostname`" != xeaton-rc3 ] && \
        UDHCPC_OPTS="$UDHCPC_OPTS -x hostname:`hostname`"
        ;;
esac

echo "INFO: udhcpc command-line was changed to: /sbin/udhcpc $UDHCPC_ARGS $UDHCPC_OPTS" >&2
exec /sbin/udhcpc $UDHCPC_ARGS $UDHCPC_OPTS

