/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file bios-nutscan.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \brief Not yet documented file
 */

#include "log.h"
#include "subprocess.h"
#include "nutscan.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {

    if (argc != 4) {
        log_error("Usage: ./bios-nutscan [snmp|xml] name ip");
        return -1;
    }

    std::string proto{argv[1]};

    if (proto != "snmp" && proto != "xml") {
        std::cerr << "first argument must be snmp or xml, got " << argv[1] << std::endl;
        return -1;
    }

    nutscan_init();
    for( int N = 1 ; N < 3 ; ++N ) {
        std::cout << "pass: " << N << std::endl;
        std::vector<std::string> nut_config;

        int ret = -1;
        if (proto == "snmp") {
            ret = shared::nut_scan_snmp(
                argv[2],
                shared::CIDRAddress(argv[3]),
                nut_config);
        }
        else {
            ret = shared::nut_scan_xml_http(
                argv[2],
                shared::CIDRAddress(argv[3]),
                nut_config);
        }

        if (ret != 0) {
            std::cerr << "fail!" << std::endl;
            return 1;
        }

        size_t i = 0;
        for (const auto& s: nut_config) {
            std::cout << i << ":" << std::endl;
            i++;
            std::cout << s << std::endl;
        }
        nutscan_free();
    }
    return 0;
}
