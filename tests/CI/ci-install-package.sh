#!/bin/sh

# Copyright (C) 2014 Eaton
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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: Installs packages on virtual machine


install_packages() {
    # if debian
    apt-get update
    apt-get -f -y --force-yes --fix-missing install
    apt-get -f -y --force-yes install "$@"
}

if [ "$1" != "" ] ; then
   install_packages "$@"
else
   echo "usage: $(basename $0) package [package ...]"
fi
