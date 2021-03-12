#!/bin/sh

#
#   Copyright (c) 2021 Eaton
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
#! \file    02-migrate-users-bios-script.sh
#  \brief   Migrate _bios-script user to its new UID
#  \author  Jean-Baptiste Boric <JeanBaptisteBORIC@Eaton.com>
#

BIOS_SCRIPT_OLD_UID=$(id -u _bios-script)
BIOS_SCRIPT_NEW_UID=124

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

if [ "$(id -u _bios-script)" -ne "$BIOS_SCRIPT_NEW_UID" ]
then
    echo "Fixing user _bios-script ($BIOS_SCRIPT_OLD_UID) into id $BIOS_SCRIPT_NEW_UID..." >&2
    /usr/sbin/usermod -u "$BIOS_SCRIPT_NEW_UID" "_bios-script" \
    && for D in / /run /var /home /tmp /dev/shm ; do
        if [ -d "$D" ] ; then
            find "$D" -xdev -uid "$BIOS_SCRIPT_OLD_UID" -exec chown "_bios-script" '{}' \; || return
        fi
    done
fi
