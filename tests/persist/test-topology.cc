#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetmsg.h"

TEST_CASE("location topology from recursive","[db][topology][location][from][recursive][nonfiltered]")
{
    log_open ();
    log_set_level (LOG_DEBUG);
    log_info ("================= LOCATION TOPOLOGY RECURSIVE NONFILTERED=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 1);
    asset_msg_set_type        (getmsg, 2);
    asset_msg_set_recursive   (getmsg, true);
    asset_msg_set_filter_type (getmsg, 7);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology), 1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("location topology nonrecursive","[db][topology][location][from][nonrecursive][nonfiltered]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY NONRECURSIVE NONFILTERED (ALL) =================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 1);
    asset_msg_set_type        (getmsg, 2);
    asset_msg_set_recursive   (getmsg, false);
    asset_msg_set_filter_type (getmsg, 7);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("location topology recursive filtered by 3","[db][topology][location][from][recursive][filtered]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY RECURSIVE FILTERED (by type = 3)=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 1);
    asset_msg_set_type        (getmsg, 2);
    asset_msg_set_recursive   (getmsg, true);
    asset_msg_set_filter_type (getmsg, 3);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("location topology recursive filtered by 4","[db][topology][location][from][recursive][filtered]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY RECURSIVE FILTERED (by type = 4)=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 1);
    asset_msg_set_type        (getmsg, 2);
    asset_msg_set_recursive   (getmsg, true);
    asset_msg_set_filter_type (getmsg, 4);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("location topology recursive filtered by 6","[db][topology][location][from][recursive][filtered]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY RECURSIVE FILTERED (by type = 6)=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 1);
    asset_msg_set_type        (getmsg, 2);
    asset_msg_set_recursive   (getmsg, true);
    asset_msg_set_filter_type (getmsg, 6);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology) ,1);
    print_frame (asset_msg_grps    (cretTopology) ,1);

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("unlocated location topology filtered by 3","[db][topology][location][from][nonrecursive][filtered][unlocated]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY UNLOCATED FILTERED (by type = 3)=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 0);
    asset_msg_set_filter_type (getmsg, 3);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("unlocated location topology nonfiltered","[db][topology][location][from][nonrecursive][nonfiltered][unlocated]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("================= LOCATION TOPOLOGY UNLOCATED NONFILTERED=================== \n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    assert ( getmsg );
    asset_msg_set_element_id  (getmsg, 0);
    asset_msg_set_filter_type (getmsg, 7);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_from (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    print_frame (asset_msg_dcs     (cretTopology),1 );
    print_frame (asset_msg_rooms   (cretTopology),1 );
    print_frame (asset_msg_rows    (cretTopology),1 );
    print_frame (asset_msg_racks   (cretTopology),1 );
    print_frame (asset_msg_devices (cretTopology),1 );
    print_frame (asset_msg_grps    (cretTopology),1 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}


TEST_CASE("location topology to2","[db][topology][to][location]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("===============LOCATION TOPOLOGY TO 2==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 1);
    asset_msg_set_type       (getmsg, 2);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
  
    zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( zmsg_size (submsg) != 0 )
    {
        asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("location topology to6","[db][location][topology][to]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("===============LOCATION TOPOLOGY TO 6==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5);
    asset_msg_set_type       (getmsg, 6);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );

    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    
    zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( zmsg_size (submsg) != 0 )
    {
        asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
TEST_CASE("Power topology from #1","[db][topology][power][from][n1]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #1 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5019);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
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

TEST_CASE("Power topology from #2","[db][topology][power][from][n2]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #2 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5020);
//    asset_msg_print (getmsg);
    // the expected devices

    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5020, "UPSFROM2", "ups")); // id,  device_name, device_type_name

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
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
    
    REQUIRE ( asset_msg_element_id (item) == 5020);
    REQUIRE ( !strcmp(asset_msg_name (item), "UPSFROM2") );
    REQUIRE ( !strcmp(asset_msg_type_name (item), "ups") );
    asset_msg_destroy (&item);

    pop = zmsg_popmsg (zmsg);
    // there is no more devices
    REQUIRE ( pop == NULL );

    // check powers, should be empty
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );
    REQUIRE ( zlist_size (powers) == 0 );

    zlist_destroy (&powers);
 
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    // TODO need to do smth with buffer ??
    log_close();
}

TEST_CASE("Power topology from #3","[db][topology][power][from][n3]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #3 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5021);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
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
    
    REQUIRE ( asset_msg_element_id (item) == 5021);
    REQUIRE ( !strcmp(asset_msg_name (item), "UPSFROM3") );
    REQUIRE ( !strcmp(asset_msg_type_name (item), "ups") );
    asset_msg_destroy (&item);

    pop = zmsg_popmsg (zmsg);
    // there is no more devices
    REQUIRE ( pop == NULL );

    // check powers, should be empty
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );
    REQUIRE ( zlist_size (powers) == 0 );

    zlist_destroy (&powers);
 
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    // TODO need to do smth with buffer ??
    log_close();
}

TEST_CASE("Power topology from #4","[db][topology][power][from][n4]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #4 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5024);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5024, "UPSFROM4", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5027, "SINK5", "sink"));

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be one link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char first1[15]  = "1:5024:2:5027";//src_socket:src_id:dst_socket:dst_id
    char* first  = (char*) zlist_first  (powers);
    REQUIRE ( first  != NULL );
    REQUIRE ( zlist_next (powers)   == NULL );
    REQUIRE ( strstr(first, first1) == first );

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
}

TEST_CASE("Power topology from #5","[db][topology][power][from][n5]")
{   
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #5 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5028);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5028, "UPSFROM5", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5031, "SINK8", "sink"));
    sdevices.insert (std::make_tuple(5032, "SINK9", "sink"));
    sdevices.insert (std::make_tuple(5033, "SINK10", "sink"));

    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5028:3:5031"); 
    spowers.insert ("0:5028:0:5032"); 
    spowers.insert ("4:5028:0:5033"); 

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be three links
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
    
    // check the devices, should be four devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 4 ; i ++ )
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
 
TEST_CASE("Power topology from #6","[db][topology][power][from][n6]")
{
    log_open();
 //   log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #6 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5034);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5034, "UPSFROM6", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5035, "SINK11", "sink"));
    sdevices.insert (std::make_tuple(5036, "SINK12", "sink"));
    sdevices.insert (std::make_tuple(5037, "SINK13", "sink"));

    //the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5034:0:5035"); 
    spowers.insert ("0:5034:0:5036"); 
    spowers.insert ("0:5034:0:5037"); 

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be three links
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

    // check the devices, should be four devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    zmsg_t* pop = NULL;
    for (int i = 1 ; i <= 4 ; i ++ )
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

TEST_CASE("Power topology from #7","[db][topology][power][from][n7]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #7 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5038);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5038, "UPSFROM7", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5039, "SINK14", "sink"));

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be one link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char first1[15]  = "0:5038:0:5039";//src_socket:src_id:dst_socket:dst_id
    char* first  = (char*) zlist_first  (powers);
    REQUIRE ( first  != NULL );
    REQUIRE ( zlist_next (powers)   == NULL );
    REQUIRE ( strstr(first, first1) == first );

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

TEST_CASE("Power topology from #8","[db][topology][power][from][n8]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #8 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4998);
//    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    
    REQUIRE ( is_common_msg (retTopology) );
    common_msg_t* cretTopology = common_msg_decode (&retTopology);
    assert ( cretTopology );
//    common_msg_print (cretTopology);
    REQUIRE ( common_msg_errtype (cretTopology) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (cretTopology) == DB_ERROR_BADINPUT );

    asset_msg_destroy (&getmsg);
    common_msg_destroy (&cretTopology);
    log_close();
}


TEST_CASE("Power topology from #9","[db][topology][power][from][n9]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #9 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5040);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5040, "UPSFROM9", "ups")); // id,  device_name, device_type_name
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("5:5040:6:5040"); 

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
//    asset_msg_print (cretTopology);

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
    
    // check powers, should be one link
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    REQUIRE ( spowers.count(std::string ((const char*)zlist_first  (powers))) == 1 );
    REQUIRE ( zlist_next (powers)   == NULL );

    zlist_destroy (&powers);

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology from #10","[db][topology][power][from][n10]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #10 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5041);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5041, "UPSFROM10", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5042, "SINK15", "sink"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5041:0:5042"); 
    spowers.insert ("5:5041:0:5042"); 
    
    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
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

TEST_CASE("Power topology from #11","[db][topology][power][from][n11]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM #11 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5043);
//    asset_msg_print (getmsg);
    
    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5043, "UPSFROM11", "ups")); // id,  device_name, device_type_name
    sdevices.insert (std::make_tuple(5044, "SINK16", "sink"));
    
    // the expected links
    std::set<std::string> spowers;
    spowers.insert ("0:5043:0:5044"); 
    
    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
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
}

TEST_CASE("Power topology group","[db][topology][power][group]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER GROUP id = 4999 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_GROUP);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4999);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_group (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}


TEST_CASE("Power topology group empty","[db][topology][power][group]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER GROUP id = 4998 EMPTY ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_GROUP);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4998);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_group (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Power topology datacenter1","[db][topology][power][datacenter]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER DATACENTER id = 1 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 1);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

/* TODO
 * DROPS the connection to the database, so other tests cannot continue, temporary disabled
TEST_CASE("Power topology datacenter10","[db][topology][power][datacenter]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER DATACENTER id = 10 NIC ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 10);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

*/
