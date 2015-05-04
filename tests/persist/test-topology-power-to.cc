#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetcrud.h"

#include "cleanup.h"

TEST_CASE("Power topology to #1","[db][topology][power][to][power_topology.sql][t1]")
{
    log_open();

    log_info ("=============== POWER TO #1 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5019);
//    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
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


TEST_CASE("Power topology to #2","[db][topology][power][power_topology.sql][to][t2]")
{
    log_open();

    log_info ("=============== POWER TO #2 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4998);
//    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
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

TEST_CASE("Power topology to #3","[db][topology][power][power_topology.sql][to][t3]")
{
    log_open();

    log_info ("=============== POWER TO #3 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5045);
//    asset_msg_print (getmsg);

    // the expected devices
    std::set<std::tuple<int,std::string,std::string>> sdevices;
    sdevices.insert (std::make_tuple(5045, "UPSTO3", "ups")); // id,  device_name, device_type_name

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    
    // check the devices, should be one
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    _scoped_zmsg_t* pop = zmsg_popmsg (zmsg);
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

TEST_CASE("Power topology to #4","[db][topology][power][power_topology.sql][to][t4]")
{
    log_open();

    log_info ("=============== POWER TO #4 ==================");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5046);
//    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    
    // check the devices, should be one
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    _scoped_zmsg_t* pop = zmsg_popmsg (zmsg);
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

TEST_CASE("Power topology to #5","[db][topology][power][power_topology.sql][to][t5]")
{
    log_open();

    log_info ("=============== POWER TO #5 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5052:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5049"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5053:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5049"); 

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
//    print_frame_devices (asset_msg_devices (cretTopology));
    // check powers, should be two links
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    int n = zlist_size(powers);
    REQUIRE ( n == spowers.size() );
    for ( int i = 1; i <= n; i++ )
    {
        if ( i == 1 )
            a = (char*)zlist_first (powers);
        else
            a = (char*)zlist_next (powers);
        REQUIRE ( a != NULL );
        INFO (std::string (a) );
        auto it = spowers.find (std::string(a));

        REQUIRE ( it != spowers.end() );
        spowers.erase(it);
    }
    REQUIRE ( zlist_next (powers) == NULL );
    REQUIRE ( spowers.empty() );

    zlist_destroy (&powers);

    // check the devices, should be three
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
    
    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #6","[db][topology][power_topology.sql][power][to][t6]")
{   
    log_open();

    log_info ("=============== POWER TO #6 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5058:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5054"); 
    spowers.insert ("3:5057:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5058"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5057:4:5059"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5057:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5054"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5059:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5054"); 
    spowers.insert ("1:5055:2:5057"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5056:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5057"); 

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be seven links
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
    
    // check the devices, should be fsix devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}
 
TEST_CASE("Power topology to #7","[db][topology][power_topology.sql][power][to][t7]")
{
    log_open();

    log_info ("=============== POWER TO #7 ==================");
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

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be one link
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

    // check the devices, should be one device
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #8","[db][topology][power_topology.sql][power][to][t8]")
{
    log_open();

    log_info ("=============== POWER TO #8 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5064:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5062"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5063:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5064"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5062:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5063"); 
    
    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be three link
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

    // check the devices, should be three devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #9","[db][topology][power_topology.sql][power][to][t9]")
{
    log_open();

    log_info ("=============== POWER TO #9 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5066:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5070"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5067:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5070"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5068:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5071"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5069:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5071"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5070:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5072"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5071:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5073"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5072:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5065"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5073:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5065"); 

    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
//    asset_msg_print (cretTopology);

    // check the devices, should be nine device
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
    int n = sdevices.size();
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
    
    // check powers, should be eight links
    zlist_t* powers = asset_msg_get_powers (cretTopology);
    REQUIRE ( powers );

    char* a = NULL;
    n = zlist_size(powers);
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

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #10","[db][topology][power_topology.sql][power][to][t10]")
{
    log_open();

    log_info ("=============== POWER TO #10 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5075:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5074"); 
    spowers.insert ("5:5075:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5074"); 
    
    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    // check powers, should be one link
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

    // check the devices, should be two devices
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}

TEST_CASE("Power topology to #11","[db][topology][power][power_topology.sql][to][t11]")
{
    log_open();

    log_info ("=============== POWER TO #11 ==================");
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
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5077:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5076"); 
    spowers.insert (std::string(SRCOUT_DESTIN_IS_NULL) + ":5076:" + std::string(SRCOUT_DESTIN_IS_NULL) + ":5077"); 
    
    _scoped_zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
//    print_frame_devices (asset_msg_devices (cretTopology));
    
    // check powers, should be two link
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

    // check the devices, should be two
    zframe_t* frame = asset_msg_devices (cretTopology);
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );

    _scoped_zmsg_t* pop = NULL;
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
    zmsg_destroy (&zmsg);
    log_close();
}
