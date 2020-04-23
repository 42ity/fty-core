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
#! \file    shared.py
#  \brief   shared routines for CI tests written in python
#  \author  Michal Vyskocil <MichalVyskocil@Eaton.com>

import MySQLdb

def parse_ip_a_s(out):
    """
    parse output of command ip a(ddr) s(how)
    
    return - list of tuple (name, ipver, ipaddr, prefixlen: int, mac)
    """

    state = "start"
    header = list()
    ret = list()

    for line in out.split('\n'):

        if line and line[0] != ' ':
            state = "start"
            header = list()
            if not "state UP" in line:
                continue
            header = list()
            nic_name = line.split(':')[1].strip()
            header.append(nic_name)
            state = "read-mac"
            continue

        # skip entries for 'lo'
        if state == "start" and not header:
            continue

        if line.startswith("    link/"):
            try:
                mac = line.split()[1]
            except IndexError as e:
                print(line)
                print(e)
            header.append(mac)
        elif line.startswith("    inet"):
            name, mac = header
            ipver, ipaddr = line.split()[:2]
            ipaddr, prefixlen = ipaddr.split('/')

            ret.append((name, ipver, ipaddr, int(prefixlen), mac))
    return ret

def connect_to_db():
    return MySQLdb.connect(host="localhost", user="root", db="box_utf8")
