#!/bin/sh
#
# Copyright (c) 2014-2015 Eaton
#
# This file is part of the Eaton $BIOS project.
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
#! \file    start_bios.sh
#  \brief   Tool for initializing and starting BIOS processes 
#  \author  Gerald Guillaume <GeraldGuillaume@Eaton.com>

echo  "initializing db .."
mysql < /usr/local/share/bios/sql/mysql/initdb.sql
mysql < /usr/local/share/bios/examples/sql/mysql/load_data.sql
mysql < /usr/local/share/bios/examples/sql/mysql/load_ROZLAB.sql
mysql < /usr/local/share/bios/docker/resources/patch.sql

echo "Starting SASL auth .."
service saslauthd start

echo "Starting network dummy interface"
ip link add eth1 type dummy
ip link set eth1 up
ip link add eth2 type dummy
ip link set eth2 up

echo "Starting web srv .."
export LICENSE_DIR=/var/license
export DATADIR=/var/bios
tntnet -c /etc/tntnet/tntnet.xml &

echo "starting nut .."
service nut-server start

echo "starting Broker .."
malamute /etc/malamute/malamute.cfg &

echo "starting BIOS .."
/usr/local/libexec/bios/agent-dbstore &
/usr/local/libexec/bios/agent-inventory &
/usr/local/libexec/bios/agent-alert &
/usr/local/libexec/bios/agent-cm &
/usr/local/libexec/bios/agent-tpower &
# this is one shot program
/usr/local/libexec/bios/outage
/usr/local/libexec/bios/agent-nut 

 

