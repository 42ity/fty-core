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
#! \file    11-mount-boot-noauto.sh
#  \brief   Some releases defaulted to R/W mounts of /boot. Fix that.
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

# Try to fix the next boot-up's mounts, but do not fail if we could not
# patch this now (permissions, victim line not there).
sed -e 's|LABEL=BOOT /boot auto defaults|LABEL=BOOT /boot auto noauto,ro|' \
    -i /etc/fstab || true

if ( mount | grep '/boot' ) ; then
    df -k /boot || true
    umount /boot || \
    umount -f /boot || \
    umount -l /boot || \
    umount -fl /boot || \
    mount -o remount,ro /boot || \
    echo "WARNING: It seems /boot was mounted, and still is" >&2
fi
