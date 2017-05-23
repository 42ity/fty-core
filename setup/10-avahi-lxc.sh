#!/bin/sh

#
#   Copyright (c) 2017 Eaton
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
#! \file    10-avahi-lxc.sh
#  \brief   Set up avahi in an LXC container so it is less restricted and can run
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

AVAHI_CFG="/etc/avahi/avahi-daemon.conf"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

[ -s "$AVAHI_CFG" ] || skip "This script does not apply on this OS: no avahi config file here ($AVAHI_CFG)"
[ -r "$AVAHI_CFG" -a -w "$AVAHI_CFG" ] || die "Can not manipulate $AVAHI_CFG"

case "`systemd-detect-virt -c`" in
    *lxc*)
        # remove 'rlimit-nproc = 3' from /etc/avahi/avahi-daemon.conf
        sed \
            -e 's|^\([ \t]*rlimit-nproc[ \t]*=.*\)|# \1|g' \
            -i "$AVAHI_CFG" || \
        die "Could not modify avahi config file"
        ;;
    *)  skip "This script does not apply on this OS: not a linux container"
        exit 0
        ;;
esac
