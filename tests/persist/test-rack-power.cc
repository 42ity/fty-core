#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetmsg.h"
#include "calc_power.h"


TEST_CASE("Rack power #1","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #1 ==================\n");
    auto t = select_power_topology_to (url.c_str(), 8003, 1, false);

    auto devices = t.first;

    REQUIRE (std::get<0>(*devices.begin()) == 8001 );

    auto b = doA (url.c_str(), t, std::make_tuple (8003, "","", 5) );
    
    log_debug( "start epdu \n");
    for ( auto &adevice : std::get<0>(b))
    {
        printf ("%d \n",std::get<0>(adevice));
    }
    log_debug( "start ups \n");
    for ( auto &adevice : std::get<1>(b))
    {
        printf ("%d \n",std::get<0>(adevice));
    }

    log_debug( "start device \n");
    for ( auto &adevice : std::get<2>(b))
    {
        printf ("%d \n",std::get<0>(adevice));
    }
    log_close();
    
}

