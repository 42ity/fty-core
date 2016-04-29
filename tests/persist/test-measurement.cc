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
 * \file test-measurement.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <tntdb/connection.h>
#include <tntdb/value.h>

#include <vector>

#include "measurements.h"
#include "dbhelpers.h"
#include "dbpath.h"
#include "log.h"
#include "persistencelogic.h"
#include "cleanup.h"

using namespace persist;

TEST_CASE("measurement INSERT/SELECT/DELETE #1", "[db][CRUD][insert][delete][select][t_bios_measurement][t_bios_measurement_topic]")
{

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // it is supposed that topic has a predefined structure
    // in  transport layer it can be longer than in database,
    // because in database topic for measurements is supposed to be
    // name_of_the_measurement@device_name
    const char* topic1 = "this.is.complex.topic@DUMMY_DEVICE";
    const char* topic2 = "that.is.more.complex.topic@DUMMY_DEVICE";
    const char* unit1 = "unit1";
    const char* unit2 = "unit2";
    const char* query1 = "%this%";
    const char* device_name = "DUMMY_DEVICE";
    //1.) assert empty database
    auto ret = select_from_measurement_by_topic(conn, "%DUMMY_DEVICE%");
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 0);

    //2.) insert a few entries
    auto ret1 = insert_into_measurement(conn,
            topic1, 42, 1, 0, unit1, device_name);
    REQUIRE(ret1.status == 1);
    REQUIRE(ret1.affected_rows == 1);
    REQUIRE(ret1.rowid != 0);
    
    
    auto ret2 = insert_into_measurement(conn,
            topic1, 142, 1, 0, unit2, device_name);
    REQUIRE(ret2.status == 1);
    REQUIRE(ret2.affected_rows == 1);
    REQUIRE(ret2.rowid == ret1.rowid + 1);
    
    auto ret3 = insert_into_measurement(conn,
            topic2, 999, -1, 0, unit2,device_name);
    REQUIRE(ret3.status == 1);
    REQUIRE(ret3.affected_rows == 1);
    REQUIRE(ret3.rowid == ret2.rowid + 1);

    // 3.) SELECT
    ret = select_from_measurement_by_topic(conn, query1);
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 2);
    auto i = 1u;
    for (const auto it : ret.item) {
        bool condition_to_check = (it.id == ret1.rowid)  || (it.id == ret2.rowid);
        REQUIRE(condition_to_check);
        REQUIRE(it.topic == topic1);
        i++;
    }

    // 4.) SELECT all
    ret = select_from_measurement_by_topic(conn, "%DUMMY_DEVICE%");
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 3);

    // 5.) DELETE NON EXISTING
    // First find highest id
    uint64_t highest_measurements_id = 0;
    conn.selectValue("select id from t_bios_measurement ORDER BY id DESC LIMIT 1").get (highest_measurements_id); 
    CHECK (highest_measurements_id != 0 );  
        
    ret2 = delete_from_measurement_by_id(conn, highest_measurements_id + 1);
    REQUIRE(ret2.status == 1);
    REQUIRE(ret2.affected_rows == 0);

    // 6.) DELETE EXISTING
    for (i = ret1.rowid; i <=ret3.rowid; i++) {
        ret2 = delete_from_measurement_by_id(conn, i);
        REQUIRE(ret2.status == 1);
        REQUIRE(ret2.affected_rows == 1);
    }

}