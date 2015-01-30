#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetcrud.h"

TEST_CASE("Power topology group","[db][topology][power][group]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== POWER GROUP id = 4999 ==================");
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

    log_info ("=============== POWER GROUP id = 4998 EMPTY ==================");
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

    log_info ("=============== POWER DATACENTER id = 1 ==================");
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

    log_info ("=============== POWER DATACENTER id = 10 NIC ==================");
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
