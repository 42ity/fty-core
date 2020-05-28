#!/bin/sh

#
#   Copyright (c) 2017 - 2020 Eaton
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
#! \file    10-bios-gpio.sh
#  \brief   Make sure "bios" user ends up in a "gpio" group
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#
#   Note: this seems to duplicate functionality in the preinstall script,
#   but 1) it does not seem to always work (group appears but stays empty
#   in OS image), and 2) it was not there when earlier releases were made.

BIOS_USER="bios"
GPIO_GROUP="gpio"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

getent passwd "${BIOS_USER}" >/dev/null \
|| die "The '${BIOS_USER}' user account is not defined in this system"

getent group "${GPIO_GROUP}" >/dev/null \
|| groupadd --system --force "${GPIO_GROUP}" || die "Failed to create the '${GPIO_GROUP}' system group"

getent group "${GPIO_GROUP}" >/dev/null \
|| die "The '${GPIO_GROUP}' group account is not defined in this system"

getent group "${GPIO_GROUP}" \
| sed -e 's,^.*:\([^:]*\)$,\1,' \
| egrep '(^'"${BIOS_USER}"'$|^'"${BIOS_USER}"',|,'"${BIOS_USER}"'$|,'"${BIOS_USER}"',)' \
&& skip "The '${BIOS_USER}' user account is already a member of '${GPIO_GROUP}' group"

usermod -G "${GPIO_GROUP}" -a "${BIOS_USER}"
