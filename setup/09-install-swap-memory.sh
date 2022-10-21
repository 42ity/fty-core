#!/bin/sh

#
#   Copyright (c) 2019 - 2020 Eaton
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
#! \file    09-install-swap-memory.sh
#  \brief   Create swap memory
#  \author  F.-R. Degott <FrancoisRegisDegott@Eaton.com>
#

# Create swap-memory (assume 8G RAM)

#set +x

#ext4
swapfile_path="/run/initramfs/mnt/root-rw/swapfile"
swapfile_size="8G"

echo "swapfile: $swapfile_path $swapfile_size"

# deactivate/delete existing
if [ -f "$swapfile_path" ]; then
    ls -al "$swapfile_path"
    /sbin/swapoff -v -a || exit 1
    rm -f "$swapfile_path"
fi

# create/update
fallocate -l "$swapfile_size" "$swapfile_path"

# activate
if [ -f "$swapfile_path" ]; then
    chmod 600 "$swapfile_path"
    /sbin/mkswap "$swapfile_path"
    /sbin/swapon -v "$swapfile_path"
fi

# display current active swap memory
free -m

exit 0
