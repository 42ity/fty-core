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
