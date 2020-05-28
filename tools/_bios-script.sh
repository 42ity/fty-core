#!/bin/bash

# Copyright (C) 2016 - 2020 Eaton
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
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

PASSWD="/etc/default/_bios-script"
random_password() {
        # Generate a random ASCII string without "confusing" characters
        # yet strong enough for the checks (so add some varied chars)
        head -c 12 /dev/urandom | base64 | sed -e 's,[\+\=\/\ \t\n\r\%],_,g' -e 's|^\(....\)\(...\)\(.*\)|0_9z\1@\2A,\3!|'
}

if [[ ! -s "${PASSWD}" ]]; then
    password="$(random_password)"
    cat /dev/null > "${PASSWD}"
    chown root:bios-admin "${PASSWD}"
    chmod 0640 "${PASSWD}"
    cat > "${PASSWD}" << EOF
BIOS_USER="_bios-script"
BIOS_PASSWD="${password}"
EOF

    passwd _bios-script << EOF
${password}
${password}
EOF
fi
