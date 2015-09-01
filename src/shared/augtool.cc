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
 * \file augtool.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <vector>
#include <string>
#include <functional>
#include <cxxtools/split.h>

#include "augtool.h"

// Helper function to parse output of augtool
std::string augtool_out(const std::string in, bool key_value, std::string sep, std::function<bool(const std::string)> filter) {
    std::vector<std::string> spl;
    bool not_first = false;
    std::string out;
    cxxtools::split("\n", in, std::back_inserter(spl));
    if(spl.size() >= 3) {
        spl.erase(spl.begin());
        spl.pop_back();
    } else {
        return out;
    }
    for(auto i : spl) {
        auto pos = i.find_first_of("=");
        if(pos == std::string::npos) {
            if(key_value)
                continue;
            if(not_first)
                out += sep;
            if(filter(i))
                continue;
            out += i;
        } else {
            if(not_first)
                out += sep;
            if(filter(i.substr(pos+2)))
                continue;
            out += i.substr(pos+2);
        }
        not_first = true;
    }
    return out;
}

