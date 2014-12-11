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

    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;
    uint8_t id_group  = 1;

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
    edge_lf expected;
    
    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1"  , "main"  , start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1", "genset", start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1"   , "ups"   , start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7019, id_device, "srv_LOC_40"  , "server", start_id, id_dc, start_name, ""));
    
    expected.insert (std::make_tuple(7013, id_rack  , "RACK_LOC_30" , ""      , start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7007, id_row   , "ROW_LOC_30"  , ""      , start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc, start_name, ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc, start_name, ""));
    
    // a group
    expected.insert (std::make_tuple(7025, id_group, "inputpowergroup DC_LOC_01","happynewyear", start_id, id_dc, start_name    , ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #4","[db][topology][location][location_topology.sql][from][lf4]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 7;    // take all, 7 means without the filter
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #4 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1"  , "main"  , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1", "genset", start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1"   , "ups"   , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7019, id_device, "srv_LOC_40"  , "server", start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7013, id_rack  , "RACK_LOC_30" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7007, id_row   , "ROW_LOC_30"  , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc  , start_name    , ""));
    
    expected.insert (std::make_tuple(7004, id_row   , "ROW_LOC_01"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    expected.insert (std::make_tuple(7014, id_rack  , "RACK_LOC_1"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    
    expected.insert (std::make_tuple(7005, id_row   , "ROW_LOC_20"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));
    expected.insert (std::make_tuple(7006, id_row   , "ROW_LOC_21"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));

    expected.insert (std::make_tuple(7009, id_rack  , "RACK_LOC_010", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7010, id_rack  , "RACK_LOC_011", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));

    expected.insert (std::make_tuple(7020, id_device, "srv_LOC_10"  , "server", 7009    , id_rack, "RACK_LOC_010", ""));
    expected.insert (std::make_tuple(7022, id_device, "ups_LOC_010"  , "ups"  , 7009    , id_rack, "RACK_LOC_010", ""));

    expected.insert (std::make_tuple(7021, id_device, "srv_LOC_11"  , "server", 7010    , id_rack, "RACK_LOC_011", ""));

    expected.insert (std::make_tuple(7011, id_rack  , "RACK_LOC_20" , ""      , 7005    , id_row , "ROW_LOC_20"  , ""));
    
    expected.insert (std::make_tuple(7012, id_rack  , "RACK_LOC_21" , ""      , 7006    , id_row , "ROW_LOC_21"  , ""));

    // a group
    expected.insert (std::make_tuple(7025, id_group, "inputpowergroup DC_LOC_01","happynewyear", start_id, id_dc, start_name    , ""));

    // as a recursive, than tfor every group should be written one layer of the elements

    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1"  , "main"  , 7025, id_group, "inputpowergroup DC_LOC_01"  , "happynewyear"));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1", "genset", 7025, id_group, "inputpowergroup DC_LOC_01"  , "happynewyear"));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1"   , "ups"   , 7025, id_group, "inputpowergroup DC_LOC_01"  , "happynewyear"));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #5","[db][topology][location][location_topology.sql][from][lf5]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_room;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #5 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc  , start_name    , ""));
    
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
    log_info ("r1.size before grp = %d \n",(int)r1.size());
    r1.insert(r2.begin(), r2.end());
    r2.clear();
    log_info ("r1.size after grp = %d\n",(int)r1.size());

    log_info ("start print group elements\n");
    print_frame (asset_msg_grps    (cretTopology),7000 );
    log_info ("end print group elements\n");


    // check if edges are ok
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #5.1","[db][topology][location][location_topology.sql][from][lf5.1]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_room;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #5.1 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc  , start_name    , ""));
    
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #6","[db][topology][location][location_topology.sql][from][lf6]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7004;
    uint8_t     start_type_id           = id_row;
    const char* start_name              = "ROW_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_device;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #6 ==================\n");
    
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
    edge_lf expected;
    
    expected.insert (std::make_tuple(7009, id_rack  , "RACK_LOC_010", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7010, id_rack  , "RACK_LOC_011", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7020, id_device, "srv_LOC_10"  , "server", 7009    , id_rack, "RACK_LOC_010", ""));
    expected.insert (std::make_tuple(7022, id_device, "ups_LOC_010" , "ups"   , 7009    , id_rack, "RACK_LOC_010", ""));

    expected.insert (std::make_tuple(7021, id_device, "srv_LOC_11"  , "server", 7010    , id_rack, "RACK_LOC_011", ""));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #6.1","[db][topology][location][location_topology.sql][from][lf6.1]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7004;
    uint8_t     start_type_id           = id_row;
    const char* start_name              = "ROW_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_device;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #6.1 ==================\n");
    
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
    edge_lf expected;
    
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

    REQUIRE ( (int)r1.size() == 0 );    
    REQUIRE ( (int)expected.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #7","[db][topology][location][location_topology.sql][from][lf7]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #7 ==================\n");
    
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
    edge_lf expected;
    
    expected.insert (std::make_tuple(7007, id_row   , "ROW_LOC_30"  , ""      , start_id, id_dc  , start_name    , ""));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #8","[db][topology][location][location_topology.sql][from][lf8]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7002;
    uint8_t     start_type_id           = id_room;
    const char* start_name              = "ROOM_LOC_02";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #8 ==================\n");
    
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
    edge_lf expected;
  
    expected.insert (std::make_tuple(7005, id_row   , "ROW_LOC_20"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));
    expected.insert (std::make_tuple(7006, id_row   , "ROW_LOC_21"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #9","[db][topology][location][location_topology.sql][from][lf9]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_rack;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #9 ==================\n");
    
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
    edge_lf expected;
   
    expected.insert (std::make_tuple(7013, id_rack  , "RACK_LOC_30" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc  , start_name    , ""));
    
    expected.insert (std::make_tuple(7004, id_row   , "ROW_LOC_01"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    expected.insert (std::make_tuple(7014, id_rack  , "RACK_LOC_1"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    
    expected.insert (std::make_tuple(7005, id_row   , "ROW_LOC_20"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));
    expected.insert (std::make_tuple(7006, id_row   , "ROW_LOC_21"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));

    expected.insert (std::make_tuple(7009, id_rack  , "RACK_LOC_010", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7010, id_rack  , "RACK_LOC_011", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));

    expected.insert (std::make_tuple(7011, id_rack  , "RACK_LOC_20" , ""      , 7005    , id_row , "ROW_LOC_20"  , ""));
    
    expected.insert (std::make_tuple(7012, id_rack  , "RACK_LOC_21" , ""      , 7006    , id_row , "ROW_LOC_21"  , ""));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #10","[db][topology][location][location_topology.sql][from][lf10]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;
    uint8_t id_group  = 1;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_group;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #10 ==================\n");
    
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
    edge_lf expected;
   
    expected.insert (std::make_tuple(7025, id_group, "inputpowergroup DC_LOC_01","happynewyear", start_id, id_dc, start_name    , ""));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #11","[db][topology][location][location_topology.sql][from][lf11]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_device;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #11 ==================\n");
    
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
    edge_lf expected;
   
    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1"  , "main"  , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1", "genset", start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1"   , "ups"   , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7019, id_device, "srv_LOC_40"  , "server", start_id, id_dc  , start_name    , ""));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #12","[db][topology][location][location_topology.sql][from][lf12]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_device;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #12 ==================\n");
    
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
    edge_lf expected;
   
    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1"  , "main"  , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1", "genset", start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1"   , "ups"   , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7019, id_device, "srv_LOC_40"  , "server", start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7004, id_row   , "ROW_LOC_01"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    expected.insert (std::make_tuple(7009, id_rack  , "RACK_LOC_010", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7010, id_rack  , "RACK_LOC_011", ""      , 7004    , id_row , "ROW_LOC_01"  , ""));
    expected.insert (std::make_tuple(7020, id_device, "srv_LOC_10"  , "server", 7009    , id_rack, "RACK_LOC_010", ""));
    expected.insert (std::make_tuple(7022, id_device, "ups_LOC_010"  , "ups"  , 7009    , id_rack, "RACK_LOC_010", ""));
    expected.insert (std::make_tuple(7021, id_device, "srv_LOC_11"  , "server", 7010    , id_rack, "RACK_LOC_011", ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #13","[db][topology][location][location_topology.sql][from][lf13]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 0;
    uint8_t     start_type_id           = 0;
    const char* start_name              = "";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 7;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #13 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7023, id_device, "srv_LOC_50"  , "server", 0, 0,"", ""));
    expected.insert (std::make_tuple(7024, id_device, "srv_LOC_51"  , "server", 0, 0,"", ""));

    expected.insert (std::make_tuple(7015, id_rack, "RACK_LOC_50"  , "", 0, 0,"", ""));
    expected.insert (std::make_tuple(7008, id_row, "ROW_LOC_50"   , "", 0, 0,"", ""));
    expected.insert (std::make_tuple(7003, id_room, "ROOM_LOC_50"  , "", 0, 0,"", ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #14","[db][topology][location][location_topology.sql][from][lf14]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 0;
    uint8_t     start_type_id           = 0;
    const char* start_name              = "";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 6;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #14 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7023, id_device, "srv_LOC_50"  , "server", 0, 0,"", ""));
    expected.insert (std::make_tuple(7024, id_device, "srv_LOC_51"  , "server", 0, 0,"", ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #15","[db][topology][location][location_topology.sql][from][lf15]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7032;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_04";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 7;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #15 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
TEST_CASE("Location topology from #16","[db][topology][location][location_topology.sql][from][lf16]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7032;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_04";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 7;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #16 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #17","[db][topology][location][location_topology.sql][from][lf17]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7032;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_04";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #17 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #18","[db][topology][location][location_topology.sql][from][lf18]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7032;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_04";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #18 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #19","[db][topology][location][location_topology.sql][from][lf19]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7026;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_02";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #19 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}


TEST_CASE("Location topology from #20","[db][topology][location][location_topology.sql][from][lf20]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7026;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_02";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_room;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #20 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #21","[db][topology][location][location_topology.sql][from][lf21]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7029;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_03";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_rack;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #21 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #22","[db][topology][location][location_topology.sql][from][lf22]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_group  = 1;
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7029;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_03";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_device;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #22 ==================\n");
    
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
    edge_lf expected;
 
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

    // should be empty
    REQUIRE ( r1.size() == 0 );

    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #23","[db][topology][location][location_topology.sql][from][lf23]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_rack;    // take all, 7 means without the filter
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #23 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7013, id_rack  , "RACK_LOC_30" , ""      , start_id, id_dc  , start_name    , ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #24","[db][topology][location][location_topology.sql][from][lf24]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7001;
    uint8_t     start_type_id           = id_room;
    const char* start_name              = "ROOM_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = 6;
    bool        start_recursive         = false;

    log_info ("=============== LOCATION FROM #24 ==================\n");
    
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
    edge_lf expected;
    
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

    REQUIRE ( (int)r1.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology from #25","[db][topology][location][location_topology.sql][from][lf25]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_row;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #25 ==================\n");
    
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
    edge_lf expected;
 
    expected.insert (std::make_tuple(7007, id_row   , "ROW_LOC_30"  , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7001, id_room  , "ROOM_LOC_01" , ""      , start_id, id_dc  , start_name    , ""));
    expected.insert (std::make_tuple(7002, id_room  , "ROOM_LOC_02" , ""      , start_id, id_dc  , start_name    , ""));
    
    expected.insert (std::make_tuple(7004, id_row   , "ROW_LOC_01"  , ""      , 7001    , id_room, "ROOM_LOC_01" , ""));
    
    expected.insert (std::make_tuple(7005, id_row   , "ROW_LOC_20"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));
    expected.insert (std::make_tuple(7006, id_row   , "ROW_LOC_21"  , ""      , 7002    , id_room, "ROOM_LOC_02" , ""));

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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}


TEST_CASE("Location topology from #26","[db][topology][location][location_topology.sql][from][lf26]")
{
    log_open();
//    log_set_level(LOG_DEBUG);
 
    // TODO hardcoded constants
    uint8_t id_dc     = 2;
    uint8_t id_room   = 3;
    uint8_t id_row    = 4;
    uint8_t id_rack   = 5;
    uint8_t id_device = 6;
    uint8_t id_group  = 1;

    uint32_t    start_id                = 7000;
    uint8_t     start_type_id           = id_dc;
    const char* start_name              = "DC_LOC_01";
    const char* start_device_type_name  = "";   // it is not a device, so it should be empty string
    uint8_t     start_filter_type_id    = id_group;
    bool        start_recursive         = true;

    log_info ("=============== LOCATION FROM #26 ==================\n");
    
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
    edge_lf expected;
   
    expected.insert (std::make_tuple(7025, id_group, "inputpowergroup DC_LOC_01","happynewyear", start_id, id_dc, start_name    , ""));
    expected.insert (std::make_tuple(7016, id_device, "main_LOC_1","main",7025, id_group, "inputpowergroup DC_LOC_01","happynewyear"));
    expected.insert (std::make_tuple(7017, id_device, "genset_LOC_1","genset",7025, id_group, "inputpowergroup DC_LOC_01","happynewyear"));
    expected.insert (std::make_tuple(7018, id_device, "ups_LOC_1","ups",7025, id_group, "inputpowergroup DC_LOC_01","happynewyear"));
 
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
    for (auto  it = r1.begin(); it != r1.end(); ++it )
    {   
        auto itr = expected.find ( *it );
        INFO(std::get<0>(*it));
        INFO(std::get<1>(*it));
        INFO(std::get<2>(*it));
        INFO(std::get<3>(*it));
        INFO(std::get<4>(*it));
        INFO(std::get<5>(*it));
        INFO(std::get<6>(*it));
        INFO(std::get<7>(*it));
        REQUIRE ( itr != expected.end() );
        expected.erase (itr); 
    }

    REQUIRE ( (int)expected.size() == 0 );
    r1.clear();
    expected.clear();

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

