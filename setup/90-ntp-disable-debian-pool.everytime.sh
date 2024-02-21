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
#! \file    90-ntp-disable-debian-pool.everytime.sh
#  \brief   Disable debian pool NTP configuration (Debian12, ntpsec)
#  \author  Eaton Developers <eatonproductfeedback@eaton.com>

ntpconf="/etc/ntpsec/ntp.conf"

if [ -f "$ntpconf" ]; then
    # comment (once) debian pool family
    #pool [n].debian.pool.ntp.org iburst

    sed -i -E '/^pool [0-9]+\.debian.pool.ntp.org/ s/^#*/#/' "$ntpconf"
else
    echo "WARNING: '$ntpconf' not found" >&2
fi

exit 0
