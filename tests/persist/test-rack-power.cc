#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "assetmsg.h"
#include "monitor.h"
#include "calc_power.h"


TEST_CASE("Rack power #0","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #0 ==================\n");
    auto t = select_power_topology_to (url.c_str(), 8003, 1, false);

    auto devices = t.first;

    REQUIRE (std::get<0>(*devices.begin()) == 8001 );

    auto b = extract_power_sources (url.c_str(), t, std::make_tuple (8003, "","", 5) );
    
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

TEST_CASE("Rack power #1","[db][power][rack][calc][rack_power.sql][wwww]")
{
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("=============== RACK POWER #1 ==================\n");
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3;
    m_msrmnt_sbtp_id_t  subtype_id = 1;

    std::set <m_dvc_id_t> ids;
    ids.insert(convert_asset_to_monitor(url.c_str(), 8003));
    ids.insert(convert_asset_to_monitor(url.c_str(), 8004));
    ids.insert(convert_asset_to_monitor(url.c_str(), 8005));
    ids.insert(convert_asset_to_monitor(url.c_str(), 8002));
    ids.insert(convert_asset_to_monitor(url.c_str(), 8001));
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300);
    
    
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8000);
    zmsg_destroy (&res);


    log_close();
    
}
TEST_CASE("Rack power #2","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
 //   log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #2 ==================\n");
    
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8006);
    zmsg_destroy (&res);


    log_close();
    
}
TEST_CASE("Rack power #3","[db][power][rack][calc][rack_power.sql][trp3]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #3 ==================\n");
    
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8013);
    zmsg_destroy (&res);


    log_close();
    
}
TEST_CASE("Rack power #4","[db][power][rack][calc][rack_power.sql][trp4]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #4 ==================\n");
    
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8023);
    zmsg_destroy (&res);


    log_close();
    
}

