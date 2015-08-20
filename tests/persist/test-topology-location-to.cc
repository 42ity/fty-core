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
 * \file test-topology-location-to.cc
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
#include <queue>          // std::queue

#include "cleanup.h"

#define id_device 6
#define id_rack 5
#define id_row 4
#define id_room 3
#define id_dc 2
#define id_group 1

TEST_CASE("Location topology to #1","[db][topology][location][location_topology.sql][to][lt1]")
{
    log_open();

    log_info ("=============== LOCATION TO #1 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7014);
    asset_msg_set_type (getmsg, id_rack);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7014,id_rack,"RACK_LOC_1","N_A"));
    expected.push (std::make_tuple (7001,id_room,"ROOM_LOC_01","N_A"));
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();
    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real vlaues");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() ) )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));

//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        //asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #2","[db][topology][location][location_topology.sql][to][lt2]")
{
    log_open();

    log_info ("=============== LOCATION TO #2 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7022);
    asset_msg_set_type (getmsg, id_device);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7022,id_device,"ups_LOC_010","ups"));
    expected.push (std::make_tuple (7009,id_rack,"RACK_LOC_010","N_A"));
    expected.push (std::make_tuple (7004,id_row,"ROW_LOC_01","N_A"));
    expected.push (std::make_tuple (7001,id_room,"ROOM_LOC_01","N_A"));
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();

    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real vlaues");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() ) )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));

//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        //asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #3","[db][topology][location][location_topology.sql][to][lt3]")
{
    log_open();

    log_info ("=============== LOCATION TO #3 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7005);
    asset_msg_set_type (getmsg, id_row);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7005,id_row,"ROW_LOC_20","N_A"));
    expected.push (std::make_tuple (7002,id_room,"ROOM_LOC_02","N_A"));
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();

    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real vlaues");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() ) )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        //asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #4","[db][topology][location][location_topology.sql][to][lt4]")
{
    log_open();

    log_info ("=============== LOCATION TO #4 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7023);
    asset_msg_set_type (getmsg, id_device);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7023,id_device,"srv_LOC_50","server"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();

    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real vlaues");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() ) )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        //asset_msg_print (parent);
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #5","[db][topology][location][location_topology.sql][to][lt5]")
{
    log_open();

    log_info ("=============== LOCATION TO #5 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7002);
    asset_msg_set_type (getmsg, id_room);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7002,id_room,"ROOM_LOC_02","N_A"));
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();

        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() )  )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #6","[db][topology][location][location_topology.sql][to][lt6]")
{
    log_open();

    log_info ("=============== LOCATION TO #6 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7025);
    asset_msg_set_type (getmsg, id_group);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7025,id_group,"inputpowergroup DC_LOC_01","happynewyear"));
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();
    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real values");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() )  )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}

TEST_CASE("Location topology to #7","[db][topology][location][location_topology.sql][to][lt7]")
{
    log_open();

    log_info ("=============== LOCATION TO #7 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 7000);
    asset_msg_set_type (getmsg, id_dc);
//    asset_msg_print (getmsg);

    std::queue<std::tuple<int,int,std::string,std::string>> expected;
    expected.push (std::make_tuple (7000,id_dc,"DC_LOC_01","N_A"));

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
    assert ( retTopology );
    REQUIRE ( is_asset_msg (retTopology) );
    _scoped_asset_msg_t* cretTopology = asset_msg_decode (&retTopology);
    assert ( cretTopology );
//    asset_msg_print (cretTopology);
    
    auto el = expected.front();
    expected.pop();
    INFO("expected values");
    INFO(std::get<0>(el));
    INFO(std::get<1>(el));
    INFO(std::get<2>(el).c_str());
    INFO(std::get<3>(el).c_str());
    INFO("real values");
    INFO((int)asset_msg_element_id(cretTopology));
    INFO((int)asset_msg_type(cretTopology));
    INFO(asset_msg_name(cretTopology));
    INFO(asset_msg_type_name(cretTopology));
    REQUIRE ( compare_start_element (cretTopology, std::get<0>(el), std::get<1>(el), 
                    std::get<2>(el).c_str(), std::get<3>(el).c_str() ));

    _scoped_zmsg_t* submsg = asset_msg_get_msg (cretTopology);
    assert ( submsg );
    while ( ( zmsg_size (submsg) != 0 ) && ( !expected.empty() )  )
    {
        _scoped_asset_msg_t* parent = asset_msg_decode (&submsg);
        assert ( parent );
        el = expected.front();
        expected.pop();
        INFO("expected values");
        INFO(std::get<0>(el));
        INFO(std::get<1>(el));
        INFO(std::get<2>(el).c_str());
        INFO(std::get<3>(el).c_str());
        INFO("real vlaues");
        INFO((int)asset_msg_element_id(cretTopology));
        INFO((int)asset_msg_type(cretTopology));
        INFO(asset_msg_name(cretTopology));
        INFO(asset_msg_type_name(cretTopology));
//        asset_msg_print (parent);
        REQUIRE ( compare_start_element (parent, std::get<0>(el), std::get<1>(el), 
                        std::get<2>(el).c_str(), std::get<3>(el).c_str() ));
        submsg = asset_msg_get_msg (parent);
        asset_msg_destroy (&parent);
    }
    REQUIRE ( expected.empty() );
    REQUIRE ( zmsg_size (submsg) == 0 );

    asset_msg_destroy (&getmsg);
    asset_msg_destroy (&cretTopology);
    log_close();
}
TEST_CASE("Location topology to #8","[db][topology][location][location_topology.sql][to][lt8]")
{
    log_open();

    log_info ("=============== LOCATION TO #8 ==================");
    _scoped_asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_TO);
    assert ( getmsg );
    asset_msg_set_element_id (getmsg, 5019);
    asset_msg_set_type (getmsg, id_dc);
//    asset_msg_print (getmsg);

    _scoped_zmsg_t* retTopology = get_return_topology_to (url.c_str(), getmsg);
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

