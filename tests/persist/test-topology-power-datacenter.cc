#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetcrud.h"


TEST_CASE("Power topology datacenter #1","[db][topology][power][datacenter][power_topology.sql][pd1]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER DATACENTER #1 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5078);
//    asset_msg_print (getmsg);

    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5087, "MAIN-05", "main")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5081, "UPS1-05", "ups"));
    sdevices.insert (std::make_tuple(5082, "UPS2-05", "ups"));
    sdevices.insert (std::make_tuple(5083, "ePDU1-05", "epdu"));
    sdevices.insert (std::make_tuple(5084, "ePDU2-05", "epdu"));
    sdevices.insert (std::make_tuple(5085, "SRV1-05", "server"));
    sdevices.insert (std::make_tuple(5086, "SRV2-05", "server"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5087:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5081"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5087:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5082"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5081:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5083"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5082:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5084"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5083:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5085"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5083:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5086"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5084:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5085"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5084:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5086"); 
    
    zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    int n = zlist_size(powers);
    for ( int i = 1; i <= n; i++ )
    {
        if ( i == 1 )
            a = (char*)zlist_first (powers);
        else
            a = (char*)zlist_next (powers);
        REQUIRE ( a != NULL );
        auto it = spowers.find (std::string(a));
        REQUIRE ( it != spowers.end() );
        spowers.erase(it);
    }
    REQUIRE ( zlist_next (powers) == NULL );
    REQUIRE ( spowers.empty() );
    zlist_destroy (&powers);

    // check the devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    n = sdevices.size();
    for (int i = 1 ; i <= n ; i ++ )
    {   
        pop = zmsg_popmsg (zmsg);
        REQUIRE ( pop != NULL );
    
        asset_msg_t* item = asset_msg_decode (&pop); // pop is freed
        assert ( item );
//    asset_msg_print (item);
        auto it = sdevices.find ( std::make_tuple ( asset_msg_element_id (item),
                                                    asset_msg_name (item),
                                                    asset_msg_type_name (item) ));
        REQUIRE ( it != sdevices.end() );
        sdevices.erase (it); 
        asset_msg_destroy (&item);
    }
    
    // there is no more devices
    pop = zmsg_popmsg (zmsg);
    REQUIRE ( pop == NULL );
    REQUIRE ( sdevices.empty() );
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Power topology datacenter #2","[db][topology][power][datacenter][power_topology.sql][pd2]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER DATACENTER #2 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5019);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );

    // check powers
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( zlist_size(powers) == 0 );
    
    // check the devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = zmsg_popmsg (zmsg);
    REQUIRE ( pop == NULL );
 
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
