#!/bin/bash

# Copyright (C) 2016 Eaton
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
#! \file   _bios-script.sh
#  \brief  Initialize password for _bios-script user
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>

PASSWD="/etc/default/_bios-script"
random_password() {
        # Generate a random ASCII string without "confusing" characters
        head -c 12 /dev/urandom | base64 | sed 's,[\+\=\/\ \t\n\r\%],_,g'
}

if [[ ! -f "${PASSWD}" ]]; then
    password=$(random_password)
    printf 'BIOS_USER="_bios-script"\nBIOS_PASSWD="%s"' "${password}" >"${PASSWD}" 
    chown root:bios-admin "${PASSWD}"
    chmod 0640 "${PASSWD}"

    passwd _bios-script << EOF
${password}
${password}
EOF
fi
