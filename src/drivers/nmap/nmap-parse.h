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

/*! \file nmap-parse.h
    \brief Parsing code for nmap XML output
    \author Michal Vyskocil <michalvyskocil@eaton.com>
 */

#ifndef _SRC_DRIVERS_NMAP_PARSE_H_
#define _SRC_DRIVERS_NMAP_PARSE_H_

#include <iostream>
#include <sstream>
#include <list>
#include <string>

//! \brief parse results of nmap list scan -sL and return a list of ip addresses
//
// \param inp - input stream
void parse_list_scan(std::istream& inp, zsock_t *socket);

//! \brief parse results of nmap list scan -sL and return a list of ip addresses
//
// \param inp - input string
void parse_list_scan(const std::string& inp, zsock_t *socket);

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

#endif //_SRC_DRIVERS_NMAP_PARSE_H_

