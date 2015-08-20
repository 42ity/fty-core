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
 * \file test-rack-power.cc
 * \author Alena Chernikava
 * \author Michal Vyskocil
 * \author Karol Hrdina
 * \brief Not yet documented file
 */
#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include <map>
#include "log.h"

#include "assettopology.h"
#include "dbhelpers.h"
#include "monitor.h"
#include "dbtypes.h"
#include "calc_power.h"

#include "cleanup.h"

TEST_CASE("Rack power v2","[db][power][rack][calc][v2][rack_power.sql]")
{
    log_open();
    log_info ("=============== RACK POWER v2 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached(url)); 
    
    db_reply <std::map<std::string, std::vector<std::string>>> ret =
        select_devices_total_power_racks (conn);

    REQUIRE ( ret.status == 1 );

    std::map<std::string, std::set<std::string>> const EXP {
        { "RACK1",   { "ups1_1" } },
        { "RACK101", { "epdu101_1", "epdu101_2" } },
        { "RACK102", { "ups102_1" } },
        { "RACK103", { "ups103_1", "ups103_2" } },
        { "RACK104", { } },
        { "RACK105", { "epdu105_1" } },
        { "RACK106", { "ups106_1" } },
        { "RACK107", { "epdu107_1", "epdu107_2" } },
        { "RACK108", { "epdu108_1", "epdu108_2" } },
        { "RACK2",   { "epdu2_1", "ups2_1" } },
        { "RACK3",   { "epdu3_1", "ups3_1", "ups3_3"}},
        { "RACK4",   { "epdu4_1", "ups4_1" } }
    };

    REQUIRE (ret.item.size() == EXP.size());

    for ( auto & oneRack : ret.item )
    {
        log_debug ("rack: '%s'",oneRack.first.c_str());
        REQUIRE(EXP.count(oneRack.first) == 1);
        auto devices = oneRack.second;
        auto exp_devices = EXP.at(oneRack.first);

        REQUIRE(devices.size() == exp_devices.size());

        log_debug ("power_sources:");
        for ( auto &oneDevice : devices )
        {
            log_debug ("   : '%s'",oneDevice.c_str());
            REQUIRE(exp_devices.count(oneDevice) == 1);
        }
    }
    log_close();
}
