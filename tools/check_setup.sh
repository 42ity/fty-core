#!/bin/bash

#
# Copyright (C) 2015 - 2020 Eaton
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


#! \file   check_setup.sh
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \brief  Not yet documented file

echo "Check requirements of environment for 42ity when cloning & building:"

# Check no. 1:
# We expect db to be in UTC time zone
OUT=$(mysql -u root -N -e "select FROM_UNIXTIME (1577890800)")
if ! grep -q "2020-01-01 15:00:00" <<< "${OUT}"; then
    echo "ISSUE: Fix database time zone!"
    exit 1
fi

OUT=$(mysql -u root -N -e "select FROM_UNIXTIME (1437137193)")
if ! grep -q "2015-07-17 12:46:33" <<< "${OUT}"; then
    echo "ISSUE: Fix database time zone!"
    exit 1
fi

echo "OK."
