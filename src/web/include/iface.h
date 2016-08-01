/*
Copyright (C) 2015-2016 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*!
 * \file iface.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Function for accessing network interfaces on Linux
 */

#include <set>
#include <vector>
#include <string>

struct iface {
   std::vector<std::string> ip;
   std::vector<std::string> netmask;
   std::string state;
   std::string gateway;
   std::string cable;
   std::string mac;
};

std::set<std::string> get_ifaces();
iface get_iface(std::string iface);
