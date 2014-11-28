#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"

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
    
    print_frame (asset_msg_dcs     (cretTopology) );
    print_frame (asset_msg_rooms   (cretTopology) );
    print_frame (asset_msg_rows    (cretTopology) );
    print_frame (asset_msg_racks   (cretTopology) );
    print_frame (asset_msg_devices (cretTopology) );
    print_frame (asset_msg_grps    (cretTopology) );

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
    
    print_frame (asset_msg_dcs     (cretTopology) );
    print_frame (asset_msg_rooms   (cretTopology) );
    print_frame (asset_msg_rows    (cretTopology) );
    print_frame (asset_msg_racks   (cretTopology) );
    print_frame (asset_msg_devices (cretTopology) );
    print_frame (asset_msg_grps    (cretTopology) );

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
    
    print_frame (asset_msg_dcs     (cretTopology) );
    print_frame (asset_msg_rooms   (cretTopology) );
    print_frame (asset_msg_rows    (cretTopology) );
    print_frame (asset_msg_racks   (cretTopology) );
    print_frame (asset_msg_devices (cretTopology) );
    print_frame (asset_msg_grps    (cretTopology) );

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
    
    print_frame (asset_msg_dcs     (cretTopology) );
    print_frame (asset_msg_rooms   (cretTopology) );
    print_frame (asset_msg_rows    (cretTopology) );
    print_frame (asset_msg_racks   (cretTopology) );
    print_frame (asset_msg_devices (cretTopology) );
    print_frame (asset_msg_grps    (cretTopology) );

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
    
    print_frame (asset_msg_dcs     (cretTopology) );
    print_frame (asset_msg_rooms   (cretTopology) );
    print_frame (asset_msg_rows    (cretTopology) );
    print_frame (asset_msg_racks   (cretTopology) );
    print_frame (asset_msg_devices (cretTopology) );
    print_frame (asset_msg_grps    (cretTopology) );

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

TEST_CASE("Power topology from","[db][topology][power][from]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER FROM ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_FROM);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 8);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_from (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices(cretTopology));

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
 
TEST_CASE("Power topology to","[db][topology][power][to]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER TO serv2 ==================\n");
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5000);
    asset_msg_print (getmsg);

    zmsg_t* retTopology = get_return_power_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
