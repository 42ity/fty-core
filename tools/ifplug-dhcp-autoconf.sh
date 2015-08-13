#!/bin/sh
#
# Copyright (C) 2015 Eaton
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
#! \file    ifplug-dhcp-autoconf.sh
#  \brief   discover DHCP configured interfaces and put into ifplugd
#  \author  Jim Klimov <EvgenyKlimov@eaton.com>
#  \details This script uses augeas-tools to discover interfaces
#           currently configured as DHCP clients, and puts them into ifplugd
#           configuration so they can reconfigure upon link up/down events.

[ -n "$SCRIPTDIR" -a -d "$SCRIPTDIR" ] || \
        SCRIPTDIR="$(cd "`dirname "$0"`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname "$0"`" || \
        SCRIPTDIR="`dirname "$0"`"

# Include our standard routines for CI scripts
[ -s "$SCRIPTDIR/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/scriptlib.sh || \
{ [ -s "$SCRIPTDIR/../tests/CI/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/../tests/CI/scriptlib.sh ; } || \
{ echo "FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_CHECKOUTDIR=no NEED_BUILDSUBDIR=no determineDirs_default || true
LOGMSG_PREFIX="BIOS-IFPLUG-DHCP-"

( which augtool >/dev/null ) || die $?

ifplugd_on() {
    logmsg_info "(Re-)starting ifplugd..."
    systemctl enable ifplugd.service && \
    systemctl restart ifplugd.service && \
    logmsg_info "Enabled and (re-)started the systemd ifplugd.service"
}

ifplugd_off() {
    logmsg_info "Stopping ifplugd..."
    systemctl disable ifplugd.service
    systemctl stop ifplugd.service
    logmsg_info "Disabled and perhaps stopped systemd ifplugd.service"
}

# Run augtools once to speed up the process
AUGOUT="`(echo 'match /files/etc/network/interfaces/iface[*]'; echo 'match /files/etc/network/interfaces/iface[*]/method' ; echo 'match /files/etc/default/ifplugd/INTERFACES' ) | augtool`" || die $?
logmsg_debug "AUGOUT = " "$AUGOUT"

# Note: This may be revised or option-switched to find not DHCP interfaces,
# but something like "non-off, non-loopback" to monitor all active ones.
INTLIST=""
for INTNUM in `echo "$AUGOUT" | egrep '/method *= *(dhcp|static|manual)$' | sed 's,/method .*$,,'` ; do
    INTNAME="`echo "$AUGOUT" | fgrep "$INTNUM = " | sed 's,^[^=]* = *,,'`" \
        || continue
    logmsg_debug "Found configured interface $INTNAME"
    [ -z "$INTLIST" ] && INTLIST="$INTNAME" || INTLIST="$INTLIST $INTNAME"
done

# We should first disable ifplugd with its old list of tracked interfaces,
# then change it, then maybe start it back up if the list is not empty
if [ -z "$INTLIST" ]; then
    logmsg_info "No interfaces were currently detected as DHCP clients, static or manual (will clear the ifplugd list)"
else
    logmsg_info "The following interfaces were currently detected as DHCP clients, static or manual: $INTLIST"
fi

OLDINTLIST="`echo "$AUGOUT" | grep ifplugd/INTERFACES | sed 's,^[^=]* = *,,' | sed 's,^\"\(.*\)\"$,\1,'`"

if [ "$OLDINTLIST" = "$INTLIST" ]; then
    logmsg_info "Existing configuration of ifplugd already matches this list, got nothing to change!"
else
    [ -n "$OLDINTLIST" ] && \
        logmsg_info "Existing configuration of ifplugd tracked $OLDINTLIST;" \
            "I will disable the service before reconfiguring it" && \
        ifplugd_off

    logmsg_info "Applying changes to ifplugd configuration..."
    if ! /bin/echo -E set /files/etc/default/ifplugd/INTERFACES "\"\\\"$INTLIST\\\"\"" | augtool -e -s ; then
        [ -n "$OLDINTLIST" ] && ifplugd_on
        die "Could not apply changes to ifplugd config"
    fi
fi

if [ -n "$INTLIST" ]; then
    ifplugd_on || die
fi

logmsg_info "Started $0 $@"
