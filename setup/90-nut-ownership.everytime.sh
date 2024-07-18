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
#! \file    90-nut-ownership.everytime.sh
#  \brief   Make sure NUT configuration and data locations are owned
#           by proper account names even if name-to-number mapping has
#           changed in the user database
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>

NUT_USER="nut"
NUT_GROUP="nut"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

getent passwd "${NUT_USER}" >/dev/null \
|| die "The '${NUT_USER}' user account is not defined in this system"

getent group "${NUT_GROUP}" >/dev/null \
|| die "The '${NUT_GROUP}' group account is not defined in this system"

RES=0
for D in /run/nut /var/run/nut /var/state/nut /var/state/ups ; do
    if [ -d "$D" ] ; then
        chown -R nut:nut "$D" || true
        chown root "$D" || true
        chmod 770 "$D" || true
    fi
done

chgrp -R nut /var/lib/nut || RES=$?
chown root /var/lib/nut || RES=$?
# Change card server and client certificates to be owned by the NUT user
chown nut /var/lib/nut/*.* || RES=$?
chmod 770 /var/lib/nut || RES=$?

chgrp -R nut /etc/nut || RES=$?
chown -R root /etc/nut || RES=$?
chmod -R 640 /etc/nut || RES=$?
chmod 755 /etc/nut || RES=$?

exit $RES
