#!/bin/sh
#
#   Copyright (c) 1991-2012 iMatix Corporation <www.imatix.com>
#   Copyright (c) 2014 Eaton Corporation <www.eaton.com>
#   Copyright other contributors as noted in the AUTHORS file.
#
#   This file is part of the Eaton $BIOS project.
#
#   This is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This software is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   Description: Script to generate all required files from fresh git
#   checkout.
#   NOTE: It expects to be run in the root of the project directory
#   (probably the checkout directory, unless you use strange set-ups).
echo  "initializing db .."
mysql < /tmp/initdb.sql
mysql < /tmp/load_data.sql
mysql < /tmp/patch.sql

echo "Starting SASL auth .."
service saslauthd start

echo "Starting web srv .."
service tntnet start

echo "starting nut .."
service nut-server start

echo "starting simple .."
cd /usr/local/bin
./simple 

