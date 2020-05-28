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
#! \file    20-ipc-lcd-services.sh
#  \brief   Toggle IPC LCD services that appeared after initial MVP release
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

if [ "`uname -m`" = x86_64 ]; then
    /bin/systemctl disable lcd-boot-display || true
    /bin/systemctl disable lcd-net-display || true
    /bin/systemctl disable lcd-shutdown-display || true
    /bin/systemctl disable lcd-shutdown-inverse-display || true
    /bin/systemctl disable lcd-reboot-display || true
    /bin/systemctl disable lcd-poweroff-display || true
    /bin/systemctl mask lcd-boot-display || true
    /bin/systemctl mask lcd-net-display || true
    /bin/systemctl mask lcd-shutdown-display || true
    /bin/systemctl mask lcd-shutdown-inverse-display || true
    /bin/systemctl mask lcd-reboot-display || true
    /bin/systemctl mask lcd-poweroff-display || true
    /bin/systemctl disable bios-reset-button || true
    /bin/systemctl mask bios-reset-button || true
else
    /bin/systemctl enable lcd-boot-display
    /bin/systemctl enable lcd-net-display
    /bin/systemctl enable lcd-shutdown-display || true
    /bin/systemctl enable lcd-shutdown-inverse-display || true
    /bin/systemctl enable lcd-reboot-display || true
    /bin/systemctl enable lcd-poweroff-display || true
fi

/bin/systemctl daemon-reload
