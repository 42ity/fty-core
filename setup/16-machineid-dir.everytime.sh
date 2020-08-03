#!/bin/sh

#
#   Copyright (c) 2018 - 2020 Eaton
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
#! \file    16-machineid-dir.everytime.sh
#  \brief   Make sure a persistent directory for unique machine info exists
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

# On IPC appliance, the /mnt/nand is a partition that stores root squashfs.
# On OVA appliance it is similar in role, but a symlink to another miniroot
# location. On LXC containers the directory might not exist initially.
# Ownership to be by the same user account that runs the licensing agent.
MACHINEID_DIR="/mnt/nand/license"
mkdir -p "${MACHINEID_DIR}" && \
chown bios:0 "${MACHINEID_DIR}" && \
chmod 755 "${MACHINEID_DIR}" && \
test -d "$MACHINEID_DIR" && test -w "$MACHINEID_DIR" && test -x "$MACHINEID_DIR" && \
echo "`date -u`: Ensured usable persistent directory for unique machine info"
