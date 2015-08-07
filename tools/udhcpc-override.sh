#!/bin/bash

# Copyright (C) 2015 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description:
# This script overrides "udhcpc" as called by "ifup" and friends
# which have the command line hardcoded (and to bad defaults).
# Should be installed into "/usr/local/sbin/udhcpc" to take effect.

# TODO: inspect the caller and do the evil magic below only if it is
# among (ifup*|ifdown|ifquery|ifplug*)
UDHCPC_OPTS=""
while [ $# -gt 0 ]; do
        case "$1" in
                -n) ;; # ignore quit-if-not-leased
                -p|-i) UDHCPC_OPTS="$UDHCPC_OPTS $1 $2"; shift;;
                *) UDHCPC_OPTS="$UDHCPC_OPTS $1" ;;
        esac
        shift
done

# First shot for testing: hardcoded override
# TODO: inspect /etc/network/interfaces for udhcpc_opts with augtool
exec /sbin/udhcpc $UDHCPC_OPTS -b -t 4 -T 6

