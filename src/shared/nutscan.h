/*
Copyright (C) 2015 Eaton
 
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

/*! \file nutscan.h
    \brief Wrapper for libnutscan/nut-scanner
    \author Michal Vyskocil <michalvyskocil@eaton.com>

Example:
    #include "nutscan.h"

    std::string nut_config;
    auto ret = shared::nut_scan_snmp(
            "TEST",
            shared::CIDRAddress("127.0.0.1"),
            NULL,
            nut_config);
    std::cout << ret << std::endl;
*/

#ifndef SRC_SHARED_NUTSCAN_H
#define SRC_SHARED_NUTSCAN_H

#include<string>
#include<vector>
#include<functional>
#include<nut-scan.h>

#include "cidr.h"
#include "config.h"

namespace shared {

/**
 * \brief call nut scan over SNMP
 *
 * \param[in] name asset name of device
 * \param[in] ip_address ip address of device
 * \param[out] out resulted string with NUT config snippets
 * \return 0 if success, -1 otherwise
 */
int
nut_scan_snmp(
        const std::string& name,
        const CIDRAddress& ip_address,
        std::vector<std::string>& out);

/**
 * \brief call nut scan over XML HTTP
 *
 * \param[in] name asset name of device
 * \param[in] ip_address ip address of device
 * \param[out] out resulted string with NUT config snippets
 * \return 0 if success, -1 otherwise
 */
int
nut_scan_xml_http(
        const std::string& name,
        const CIDRAddress& ip_address,
        std::vector<std::string>& out);

} //namespace shared

#endif //SRC_SHARED_NUTSCAN_H
