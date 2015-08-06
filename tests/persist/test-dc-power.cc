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

