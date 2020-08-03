#!/bin/sh

#
#   Copyright (c) 2020 Eaton
#
#   This file is part of the Eaton 42ity project.
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
#! \file    90-ntp-dhcp-conf.everytime.sh
#  \brief   Make sure the last known NTP settings from DHCP are honored
#           just after boot
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

# Sync settings with udhcp-hook.sh
NTPD_INIT="/usr/lib/ntp/ntp-systemd-wrapper"
NTP_DHCP_CONF_RUNTIME="/run/ntp.conf.dhcp"
NTP_DHCP_CONF="/var/lib/ntp/ntp.conf.dhcp"

[ -s "${NTPD_INIT}" ] && [ -x "${NTPD_INIT}" ] || skip "This scriptlet only applies if ${NTPD_INIT} is present"
grep "${NTP_DHCP_CONF_RUNTIME}" "${NTPD_INIT}" >/dev/null || skip "This scriptlet only applies if ${NTPD_INIT} references ${NTP_DHCP_CONF_RUNTIME}"
[ -s "${NTP_DHCP_CONF}" ] || skip "There is no saved last-known NTP configuration provided from DHCP options"
if [ -s "${NTP_DHCP_CONF_RUNTIME}" ]; then skip "The ${NTP_DHCP_CONF_RUNTIME} already exists" ; fi

echo "Symlinking the saved last-known NTP configuration provided from DHCP options, from ${NTP_DHCP_CONF} to ${NTP_DHCP_CONF_RUNTIME} as used by ${NTPD_INIT} on this distribution"
ln -fsr "${NTP_DHCP_CONF}" "${NTP_DHCP_CONF_RUNTIME}" || exit
