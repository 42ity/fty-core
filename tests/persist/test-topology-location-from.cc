#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetmsg.h"


TEST_CASE("Location topology from #1","[db][topology][location][location_topology.sql][from][lf1]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== LOCATION FROM #1 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    // t_bios_asset_element with id 5019 should not exist
    asset_msg_set_element_id (getmsg, 5019);
    asset_msg_set_type (getmsg, 1);
    asset_msg_set_filter_type (getmsg, 7);
    asset_msg_set_recursive (getmsg, true);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_common_msg (retTopology) );
    common_msg_t* cretTopology = common_msg_decode (&retTopology);
    assert ( cretTopology );
//    common_msg_print (cretTopology);
    REQUIRE ( common_msg_errtype (cretTopology) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (cretTopology) == DB_ERROR_NOTFOUND );

    asset_msg_destroy (&getmsg);
    common_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #3","[db][topology][location][location_topology.sql][from][lf3]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = 2;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 7;    // take all, 7 means without the filter
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #3 ==================\n");
    
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, start_id);
    asset_msg_set_type        (getmsg, start_type_id);
    asset_msg_set_filter_type (getmsg, start_filter_type_id);
    asset_msg_set_recursive   (getmsg, start_recursive);
//    asset_msg_print (getmsg);

    // expected childs in the tree
    // (child, parent)
    // id, id_type, name, device_type_name
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    edge_lf expected;
    
    expected.insert (std::make_tuple(7016,id_device,"main_LOC_1"  ,"main"  ,7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7017,id_device,"genset_LOC_1","genset",7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7018,id_device,"ups_LOC_1"   ,"ups"   ,7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7019,id_device,"srv_LOC_40"  ,"server",7000,id_dc,"DC_LOC_01",""));
    
    expected.insert (std::make_tuple(7013,id_rack,"RACK_LOC_30"   ,""      ,7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7007,id_row,"ROW_LOC_30"     ,""      ,7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7001,id_room,"ROOM_LOC_01"   ,""      ,7000,id_dc,"DC_LOC_01",""));
    expected.insert (std::make_tuple(7002,id_room,"ROOM_LOC_02"   ,""      ,7000,id_dc,"DC_LOC_01",""));

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );   
    REQUIRE ( is_asset_msg (retTopology) );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    // check if the root is ok
    REQUIRE ( compare_start_element (cretTopology, start_id, start_type_id, start_name, start_device_type_name) );
   
    // take edges from each group, and union step by step into r1
    auto r1 = print_frame_to_edges (asset_msg_dcs (cretTopology), start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    auto r2 = print_frame_to_edges (asset_msg_rooms (cretTopology), start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    r1.insert(r2.begin(), r2.end());
    r2.clear();

    r2 = print_frame_to_edges (asset_msg_rows    (cretTopology),start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    r1.insert(r2.begin(), r2.end());
    r2.clear();

    r2 = print_frame_to_edges (asset_msg_racks   (cretTopology), start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    r1.insert(r2.begin(), r2.end());
    r2.clear();

    r2 = print_frame_to_edges (asset_msg_devices (cretTopology),start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    r1.insert(r2.begin(), r2.end());
    r2.clear();

    r2 = print_frame_to_edges (asset_msg_grps    (cretTopology),start_id, start_type_id, std::string(start_name), std::string(start_device_type_name));
    r1.insert(r2.begin(), r2.end());
    r2.clear();

    // check if edges are ok
    for (auto  it = expected.begin(); it != expected.end(); ++it )
    {   
        auto itr = r1.find ( *it );
        INFO(std::get<0>(*it));
        REQUIRE ( itr != r1.end() );
        r1.erase (itr); 
    }
    REQUIRE ( (int)r1.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
/*
TEST_CASE("Power topology to #3","[db][topology][power][to][t3]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #3 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5045);
//    asset_msg_print (getmsg);

    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5045, "UPSTO3", "ups")); // id,  device_name, device_type_name

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    
    // check the devices, should be one
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = zmsg_popmsg (zmsg);
    // the first device
    REQUIRE ( pop != NULL );
    
    asset_msg_t* item = asset_msg_decode (&pop); // pop is freed
    assert ( item );
//    asset_msg_print (item);
    
    REQUIRE ( asset_msg_element_id (item) == 5045);
    REQUIRE ( !strcmp(asset_msg_name (item), "UPSTO3") );
    REQUIRE ( !strcmp(asset_msg_type_name (item), "ups") );
    asset_msg_destroy (&item);

    // there is no more devices
    pop = zmsg_popmsg (zmsg);
    REQUIRE ( pop == NULL );

    // check powers, should be empty
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );
    REQUIRE ( zlist_size (powers) == 0 );

    zlist_destroy (&powers);
 
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #4","[db][topology][power][to][t4]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #4 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5046);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    
    // check the devices, should be one
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = zmsg_popmsg (zmsg);
    // the first device
    REQUIRE ( pop != NULL );
    
    asset_msg_t* item = asset_msg_decode (&pop); // pop is freed
    assert ( item );
//    asset_msg_print (item);
    
    REQUIRE ( asset_msg_element_id (item) == 5046);
    REQUIRE ( !strcmp(asset_msg_name (item), "UPSTO4") );
    REQUIRE ( !strcmp(asset_msg_type_name (item), "ups") );
    asset_msg_destroy (&item);

    // there is no more devices
    pop = zmsg_popmsg (zmsg);
    REQUIRE ( pop == NULL );

    // check powers, should be empty
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );
    REQUIRE ( zlist_size (powers) == 0 );

    zlist_destroy (&powers);
 
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #5","[db][topology][power][to][t5]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #5 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5049);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5049, "UPSTO5", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5052, "SINK17", "sink"));
    sdevices.insert (std::make_tuple(5053, "SINK18", "sink"));
    
    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5052:0:5049"); 
    spowers.insert ("0:5053:0:5049"); 

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
//    print_frame_devices (asset_msg_devices (cretTopology));
    // check powers, should be two links
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 2; i++ )
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
    REQUIRE ( zlist_next (powers)   == NULL );

    zlist_destroy (&powers);

    // check the devices, should be three
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
    
    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 3 ; i ++ )
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #6","[db][topology][power][to][t6]")
{   
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #6 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5054);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5054, "UPSTO6", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5058, "SINK22", "sink"));
    sdevices.insert (std::make_tuple(5059, "SINK23", "sink"));
    sdevices.insert (std::make_tuple(5057, "SINK21", "sink"));
    sdevices.insert (std::make_tuple(5055, "SINK19", "sink"));
    sdevices.insert (std::make_tuple(5056, "SINK20", "sink"));

    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5058:0:5054"); 
    spowers.insert ("3:5057:0:5058"); 
    spowers.insert ("0:5057:4:5059"); 
    spowers.insert ("0:5057:0:5054"); 
    spowers.insert ("0:5059:0:5054"); 
    spowers.insert ("1:5055:2:5057"); 
    spowers.insert ("0:5056:0:5057"); 

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be seven links
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 7; i++ )
    {
        if ( i == 1 )
            a = (char*)zlist_first (powers);
        else
            a = (char*)zlist_next (powers);
        REQUIRE ( a != NULL );
        auto it = spowers.find (std::string(a));
        INFO(a);
        REQUIRE ( it != spowers.end() );
        spowers.erase(it);
    }
    REQUIRE ( zlist_next (powers)   == NULL );
    zlist_destroy (&powers);
    
    // check the devices, should be fsix devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 6 ; i ++ )
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}
 
TEST_CASE("Power topology to #7","[db][topology][power][to][t7]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #7 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5061);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5061, "UPSTO7", "ups")); // id,  device_name, device_type_name

    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("5:5061:6:5061"); 

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be one link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 1; i++ )
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
    REQUIRE ( zlist_next (powers)   == NULL );

    zlist_destroy (&powers);

    // check the devices, should be one device
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 1 ; i ++ )
    {   
        pop = zmsg_popmsg (zmsg);
        REQUIRE ( pop != NULL );
    
        asset_msg_t* item = asset_msg_decode (&pop); // pop is freed
        assert ( item );
//        asset_msg_print (item);
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #8","[db][topology][power][to][t8]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #8 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5062);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5062, "UPSTO8", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5064, "SINK25", "sink"));
    sdevices.insert (std::make_tuple(5063, "SINK24", "sink"));

    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5064:0:5062"); 
    spowers.insert ("0:5063:0:5064"); 
    spowers.insert ("0:5062:0:5063"); 
    
    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be three link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 3; i++ )
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
    REQUIRE ( zlist_next (powers)   == NULL );
    zlist_destroy (&powers);

    // check the devices, should be three devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 3 ; i++ )
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
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #9","[db][topology][power][to][t9]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #9 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5065);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5065, "UPSTO9", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5072, "SINK32", "sink"));
    sdevices.insert (std::make_tuple(5073, "SINK33", "sink"));
    sdevices.insert (std::make_tuple(5070, "SINK30", "sink"));
    sdevices.insert (std::make_tuple(5071, "SINK31", "sink"));
    sdevices.insert (std::make_tuple(5066, "SINK26", "sink"));
    sdevices.insert (std::make_tuple(5067, "SINK27", "sink"));
    sdevices.insert (std::make_tuple(5068, "SINK28", "sink"));
    sdevices.insert (std::make_tuple(5069, "SINK29", "sink"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5066:0:5070"); 
    spowers.insert ("0:5067:0:5070"); 
    spowers.insert ("0:5068:0:5071"); 
    spowers.insert ("0:5069:0:5071"); 
    spowers.insert ("0:5070:0:5072"); 
    spowers.insert ("0:5071:0:5073"); 
    spowers.insert ("0:5072:0:5065"); 
    spowers.insert ("0:5073:0:5065"); 

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
//    asset_msg_print (cretTopology);

    // check the devices, should be nine device
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 9 ; i ++ )
    {   
        pop = zmsg_popmsg (zmsg);
        REQUIRE ( pop != NULL );
    
        asset_msg_t* item = asset_msg_decode (&pop); // pop is freed
        assert ( item );
//    asset_msg_print (item);
        auto it = sdevices.find ( std::make_tuple ( asset_msg_element_id (item),
                                                    asset_msg_name (item),
                                                    asset_msg_type_name (item) ));
        INFO (std::string(asset_msg_name (item)));
        REQUIRE ( it != sdevices.end() );
        sdevices.erase (it); 
        asset_msg_destroy (&item);
    }
    
    // there is no more devices
    pop = zmsg_popmsg (zmsg);
    REQUIRE ( pop == NULL );
    
    // check powers, should be eight links
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 8; i++ )
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
    REQUIRE ( zlist_next (powers)   == NULL );
    zlist_destroy (&powers);

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #10","[db][topology][power][to][t10]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #10 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5074);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5074, "UPSTO10", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5075, "SINK34", "sink"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5075:0:5074"); 
    spowers.insert ("5:5075:0:5074"); 
    
    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    // check powers, should be one link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 2; i++ )
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

    zlist_destroy (&powers);

    // check the devices, should be two devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 2 ; i ++ )
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #11","[db][topology][power][to][t11]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO #11 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5076);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5076, "UPSTO11", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5077, "SINK35", "sink"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5077:0:5076"); 
    spowers.insert ("0:5076:0:5077"); 
    
    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be two link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    for ( int i = 1; i <= 2; i++ )
    {
        if ( i == 1 )
            a = (char*)zlist_first (powers);
        else
            a = (char*)zlist_next (powers);
        REQUIRE ( a != NULL );
        auto it = spowers.find (std::string(a));
        INFO(a);
        INFO(i);
        REQUIRE ( it != spowers.end() );
        spowers.erase(it);
    }
    REQUIRE ( zlist_next (powers) == NULL );

    zlist_destroy (&powers);

    // check the devices, should be two
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 2 ; i ++ )
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}*/
