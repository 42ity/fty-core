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
#! \file    90-mylogin.everytime.sh
#  \brief   Make sure debian init script for mariadb maintenance can log into it
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#
#   Note: this file is also called, if present, by fty-db-init to
#   add the password reference as soon as it generates one. Do not
#   rename this scriptlet carelessly!

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

if [ -s /root/.my.cnf ] && [ -s /etc/mysql/debian.cnf ]; then
    grep "include /root/.my.cnf" /etc/mysql/debian.cnf >/dev/null \
    && skip "Already applied" \
    || ( echo '!include /root/.my.cnf' >> /etc/mysql/debian.cnf || die "Could not update /etc/mysql/debian.cnf" )
else
    skip "Not applicable to current system, maybe on another boot..."
fi
