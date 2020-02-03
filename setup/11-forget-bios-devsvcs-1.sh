#!/bin/sh

#
#   Copyright (c) 2019 Eaton
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
#! \file    11-forget-bios-devsvcs-1.sh
#  \brief   Make sure this deployment does not try to run obsoleted services
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#
# Note: as services get obsoleted over time between different end-user releases,
# more copies of this script can appear since each copy is only executed once.
#

# These units were development mocks intended only for tests,
# and/or implemented functionality that other components and
# their services took over, and were faked for dependencies
# and CI satisfaction. They are no longer delivered since PR
#    https://github.com/42ity/fty-core/pull/382
UNITS="bios-fake-th.service bios-agent-inventory.service"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

for U in $UNITS ; do
    /bin/systemctl stop "$U" || true
    /bin/systemctl disable "$U" || true
done
