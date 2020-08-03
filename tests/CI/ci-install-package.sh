#!/bin/bash
#
# Copyright (C) 2014 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   ci-install-package.sh
#  \brief  Installs packages on virtual machine
#  \author Tomas Halman <TomasHalman@Eaton.com>

install_packages() {
    # if debian
    apt-get update -q || { echo "Wipe metadata and retry"; rm -rf /var/lib/apt/lists/*; apt-get update -q; }
    dpkg --configure -a
    apt-get -f -y --force-yes --fix-missing \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install
    apt-get -f -y --force-yes \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install "$@"
}

if [ "$1" != "" ] ; then
   export DEBIAN_FRONTEND=noninteractive
   install_packages "$@"
else
   echo "Usage: $(basename $0) package [package ...]"
fi
