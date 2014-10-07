/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file nmap.h
    \brief Common definitions for nmap driver
    \author Michal Vyskocil <michalvyskocil@eaton.com>
 */

#ifndef _SRC_DRIVERS_NMAP_H_
#define _SRC_DRIVERS_NMAP_H_

#include <iostream>
#include <sstream>
#include <list>
#include <string>

// TODO autoconf friendly way
#define NMAP_BIN "/usr/bin/nmap"
#define SUDO_BIN "/ust/bin/sudo"


enum class NmapMethod {
    DefaultListScan,
    DefaultDeviceScan
};

//! \brief parse results of nmap list scan -sL and return a list of ip addresses
//
// \param inp - input stream
void parse_list_scan(std::istream& inp);

//! \brief parse results of nmap list scan -sL and return a list of ip addresses
//
// \param inp - input string
void parse_list_scan(const std::string& inp);

//! \brief parse results of nmap device scan (-R -sT -sU) and print the found information
//
// \param inp - input stream
// \return nothing \todo return structure to be defined
void parse_device_scan(std::istream& inp);

//! \brief parse results of nmap device scan (-R -sT -sU) and print the found information
//
// \param inp - input string
// \return nothing \todo return structure to be defined
void parse_device_scan(const std::string& inp);

#endif //_SRC_DRIVERS_NMAP_H_
