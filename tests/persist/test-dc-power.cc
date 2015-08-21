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
 * \file test-dc-power.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
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

TEST_CASE("DC power #1 v2","[db][power][dc][calc][v2][dc_power.sql]")
{
    log_open();
    log_info ("=============== DC POWER #1 v2 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached(url)); 
    
    db_reply <std::map<std::string, std::vector<std::string>>> ret =
        select_devices_total_power_dcs (conn);

    REQUIRE ( ret.status == 1 );

    std::map<std::string, std::set<std::string>> const EXP {
        { "DC1",  { "UPS1-DC1" } },
        { "DC2",  { "UPS3-DC2", "EPDU2-DC2" } }
    };

    REQUIRE (ret.item.size() == EXP.size());

    for ( auto & oneDc : ret.item )
    {
        log_debug ("dc: '%s'",oneDc.first.c_str());
        REQUIRE(EXP.count(oneDc.first) == 1);
        auto devices = oneDc.second;
        auto exp_devices = EXP.at(oneDc.first);

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

