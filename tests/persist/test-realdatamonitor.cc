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
 * \file test-realdatamonitor.cc
 * \author Alena Chernikava
 * \author Michal Vyskocil
 * \author Karol Hrdina
 * \author    unknown
 * \brief Not yet documented file
 */
#include <catch.hpp>

#include <set>
#include <iostream>
#include <czmq.h>

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>

#include "common_msg.h"
#include "dbhelpers.h"
#include "persist_error.h"
#include "monitor.h"
#include "defs.h"
#include "dbpath.h"
#include <time.h>
#include "cleanup.h"
#include "measurements.h"
#include "measurement.h"

/* tests for real monitoring assumed, that DB has the following state
 * In case, the initial data should be changed, the following tests should be changed too

MariaDB [box_utf8]> select * from v_bios_client_info_measurements;
+----+-----------+----------------------+---------------------+--------+-----------+-------+
| id | id_client | id_discovered_device | timestamp           | id_key | id_subkey | value |
+----+-----------+----------------------+---------------------+--------+-----------+-------+
|  1 |         4 |                    2 | 2014-11-12 09:45:59 |      1 |         1 |    32 |
|  2 |         4 |                    2 | 2014-11-12 09:46:59 |      1 |         1 |     3 |
|  3 |         4 |                    2 | 2014-11-12 09:47:59 |      2 |         1 |    31 |
|  4 |         4 |                    2 | 2014-11-12 09:48:59 |      2 |         2 |    12 |
|  5 |         4 |                    2 | 2014-11-12 09:49:59 |      1 |         2 |     1 |
|  6 |         4 |                    2 | 2014-11-12 09:59:59 |      3 |         1 |     1 |
|  7 |         4 |                    2 | 2014-11-12 09:59:59 |      7 |         1 |     2 |
|  8 |         4 |                    2 | 2014-11-12 09:59:59 |      4 |         1 |    56 |
|  9 |         4 |                    2 | 2014-11-12 09:59:59 |      5 |         1 |    17 |
| 10 |         4 |                    2 | 2014-11-12 09:59:59 |      6 |         1 |   931 |
| 11 |         4 |                    3 | 2014-11-12 09:59:59 |      3 |         5 |  2405 |
| 12 |         4 |                    3 | 2014-11-12 09:59:59 |      3 |         6 |  2405 |
| 13 |         4 |                    3 | 2014-11-12 09:59:59 |      3 |         7 |   500 |
+----+-----------+----------------------+---------------------+--------+-----------+-------+

MariaDB [box_utf8]> select * from v_bios_client_info_measurements_last;
+----+----------------------+--------+-----------+-------+---------------------+-------+-----------------------+
| id | id_discovered_device | id_key | id_subkey | value | timestamp           | scale | name                  |
+----+----------------------+--------+-----------+-------+---------------------+-------+-----------------------+
|  2 |                    2 |      1 |         1 |     3 | 2014-11-12 09:46:59 |    -1 | select_device         |
|  3 |                    2 |      2 |         1 |    31 | 2014-11-12 09:47:59 |    -1 | select_device         |
|  4 |                    2 |      2 |         2 |    12 | 2014-11-12 09:48:59 |    -1 | select_device         |
|  5 |                    2 |      1 |         2 |     1 | 2014-11-12 09:49:59 |    -1 | select_device         |
|  6 |                    2 |      3 |         1 |     1 | 2014-11-12 09:59:59 |    -1 | select_device         |
|  7 |                    2 |      7 |         1 |     2 | 2014-11-12 09:59:59 |     0 | select_device         |
|  8 |                    2 |      4 |         1 |    56 | 2014-11-12 09:59:59 |    -1 | select_device         |
|  9 |                    2 |      5 |         1 |    17 | 2014-11-12 09:59:59 |    -1 | select_device         |
| 10 |                    2 |      6 |         1 |   931 | 2014-11-12 09:59:59 |    -1 | select_device         |
| 11 |                    3 |      3 |         5 |  2405 | 2014-11-12 09:59:59 |    -1 | monitor_asset_measure |
| 12 |                    3 |      3 |         6 |  2405 | 2014-11-12 09:59:59 |    -1 | monitor_asset_measure |
| 13 |                    3 |      3 |         7 |   500 | 2014-11-12 09:59:59 |    -1 | monitor_asset_measure |
+----+----------------------+--------+-----------+-------+---------------------+-------+-----------------------+

"keytag_id:subkeytag_id:value:scale"

*/

/*
MariaDB [box_utf8]> select * from v_bios_discovered_device;
+----+-----------------------+----------------+
| id | name                  | id_device_type |
+----+-----------------------+----------------+
|  1 | DUMMY_DEVICE          |              1 |
|  2 | select_device         |              1 |
|  3 | monitor_asset_measure |              1 |
|  4 | measures              |              1 |
+----+-----------------------+----------------+
*/

//DUMMY_DEVICE_ID defined in defs.h
#define SELECT_DEVICE_ID 2
#define MONITOR_ASSET_MEASURE_ID 3
#define MEASURES_DEVICE_ID 4
#define NO_DEVICE_ID 42

static const std::set<std::string> EXP{"6:1:931:-1", "5:1:17:-1", "4:1:56:-1", "7:1:2:0", "3:1:1:-1", "1:2:1:-1", "2:2:12:-1", "2:1:31:-1", "1:1:3:-1"};

TEST_CASE("real_measurements: select_last_measurements", "[db][select][lastmeasurements]")
{
    //success
    uint32_t id = SELECT_DEVICE_ID;
    std::string device_name_exp = "select_device";

    tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached(url.c_str()));

    // insert measurement we want to select
    std::vector<int> ids{};
    
    std::set <std::string> list_EXP{};
    int scale = -1;
    for ( int i = 0 ; i <= 20 ; i++ )
    {
        std::string topic = "my_topic" + std::to_string(i);
        int64_t time = ::time(NULL);
        int value = 100 + i;
        std::string list_item = std::to_string(value) + ":" + std::to_string(scale) + ":" + topic;
        list_EXP.insert (list_item);
        db_reply_t ret = persist::insert_into_measurement(
                conn,
                topic.c_str(),
                value,
                scale,
                time,
                "W",
                device_name_exp.c_str());
        REQUIRE ( ret.status == 1 );
        ids.push_back(ret.rowid);
    }

    std::string device_name = "select_device";
    _scoped_zlist_t *measurements = select_last_measurements (conn, id, device_name);
    REQUIRE ( measurements );
    CHECK ( device_name == device_name_exp );

    REQUIRE (zlist_size (measurements) == list_EXP.size());
    std::set<std::string> results;
    for (char* s = (char*) zlist_first(measurements); s != NULL; s = (char*) zlist_next(measurements)) {
        results.insert(s);
    }
    REQUIRE (results == list_EXP);
    zlist_destroy (&measurements);

    for ( auto id : ids )
    {
        auto ret = persist::delete_from_measurement_by_id(conn, id);
        REQUIRE ( ret.status == 1 );
    }
 
    //FAIL
    id = NO_DEVICE_ID;
    device_name = "";
    measurements = select_last_measurements (conn, id, device_name);
    CHECK ( device_name == "" );
    REQUIRE ( measurements == NULL );


}

TEST_CASE("helper functions: convert_asset_to_monitor", "[db][convert_to_monitor]")
{
    tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached (url));
    tntdb::Value val;
    uint32_t id_asset = 0;
    uint32_t id_monitor = 0;

    tntdb::Statement st = conn.prepareCached (
       "select id_asset_element from t_bios_asset_element where name = 'DC1'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_asset));
    REQUIRE_THROWS_AS(convert_asset_to_monitor (url.c_str(), id_asset), bios::NotFound );

    st = conn.prepareCached(
       "select id_asset_element from t_bios_asset_element where name = 'ROW1'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_asset));

    st = conn.prepareCached(
       "select id_asset_element from t_bios_asset_element where name = 'ups'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_asset));

    st = conn.prepareCached (
       "select id_discovered_device from t_bios_discovered_device "
       "where name = 'select_device' AND id_device_type = "
       "(select id_device_type from t_bios_device_type where name = 'not_classified')"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_monitor));

    REQUIRE ( convert_asset_to_monitor (url.c_str(), id_asset) == id_monitor);
}

TEST_CASE("helper functions: convert_monitor_to_asset", "[db][convert_to_asset]")
{
    tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached(url));
    tntdb::Value val;
    uint32_t id_asset = 0;
    uint32_t id_monitor = 0;

    tntdb::Statement st = conn.prepareCached (
       "select id_discovered_device from t_bios_discovered_device "
       "where name = 'select_device' AND id_device_type = "
       "(select id_device_type from t_bios_device_type where name = 'not_classified')"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_monitor));

    st = conn.prepareCached (
       "select id_asset_element from t_bios_asset_element where name = 'ups'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_asset));
    REQUIRE ( convert_monitor_to_asset (url.c_str(), id_monitor) == id_asset );

    id_monitor = 65530;
    REQUIRE_THROWS_AS ( convert_monitor_to_asset (url.c_str(), id_monitor),  bios::NotFound );
}


TEST_CASE("get_last_measurements", "[db][get][lastmeasurements]")
{
    //TODO
    //SUCCESS
 /*   tntdb::Connection conn;
    REQUIRE_NOTHROW (conn = tntdb::connectCached(url));
    tntdb::Value val;
    uint32_t id = 0;

    tntdb::Statement st = conn.prepareCached (
       "select id_asset_element from t_bios_asset_element where name = 'ups'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id));

    _scoped_zmsg_t* getlastmeasurements = common_msg_encode_get_last_measurements (id);
    _scoped_common_msg_t* glm = common_msg_decode (&getlastmeasurements);
    common_msg_print (glm);
    _scoped_zmsg_t* returnmeasurements = _get_last_measurements (url.c_str(), glm);
    common_msg_print (glm);
    REQUIRE ( returnmeasurements );

    REQUIRE ( is_common_msg (returnmeasurements) );
    common_msg_destroy (&glm);

    glm = common_msg_decode (&returnmeasurements);
    REQUIRE ( common_msg_id (glm) == COMMON_MSG_RETURN_LAST_MEASUREMENTS );
    REQUIRE ( common_msg_device_id (glm) == id );
    _scoped_zlist_t* measurements = common_msg_get_measurements (glm);
    
    REQUIRE (zlist_size(measurements) == EXP.size());
    std::set<std::string> results;
    for (char* s = (char*) zlist_first(measurements); s != NULL; s = (char*) zlist_next(measurements)) {
        results.insert(s);
    }
    REQUIRE (results == EXP);
    
    zlist_destroy (&measurements);
    common_msg_destroy (&glm);

    //FAIL
    id = 65531;
    getlastmeasurements = common_msg_encode_get_last_measurements (id);
    glm = common_msg_decode (&getlastmeasurements);
    returnmeasurements = _get_last_measurements (url.c_str(), glm);
    REQUIRE ( returnmeasurements );
    REQUIRE ( is_common_msg (returnmeasurements) );
    common_msg_destroy (&glm);

    glm = common_msg_decode (&returnmeasurements);
    REQUIRE ( common_msg_id (glm) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (glm) == DB_ERROR_NOTFOUND );
    common_msg_destroy (&glm);
*/
}
