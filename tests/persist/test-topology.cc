/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file test-topology.cc
 * \author Alena Chernikava
 * \author Karol Hrdina
 * \author Michal Hrusecky
 * \brief Not yet documented file
 */
#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "common_msg.h"
#include "assetcrud.h"

#include "cleanup.h"

TEST_CASE("Power topology group","[db][topology][power][group]")
{
    log_open();

    log_info ("=============== POWER GROUP id = 4999 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_GROUP);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4999);
    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_group (url.c_str(), getmsg);
    assert ( retTopology );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
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

    log_info ("=============== POWER GROUP id = 4998 EMPTY ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_GROUP);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 4998);
    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_group (url.c_str(), getmsg);
    assert ( retTopology );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
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

    log_info ("=============== POWER DATACENTER id = 1 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 1);
    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
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

    log_info ("=============== POWER DATACENTER id = 10 NIC ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_POWER_DATACENTER);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 10);
    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_power_topology_datacenter (url.c_str(), getmsg);
    assert ( retTopology );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
    asset_msg_print (cretTopology);
    print_frame_devices (asset_msg_devices (cretTopology));
    
    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

*/
