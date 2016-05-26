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
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>

#include <tntdb/connect.h>

#include "dbhelpers.h"
#include "persist_error.h"
#include "dbpath.h"

TEST_CASE("helper functions: convert_asset_to_monitor_old", "[db][convert_to_monitor]")
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
    REQUIRE_THROWS_AS(convert_asset_to_monitor_old (url.c_str(), id_asset), bios::NotFound );

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
       "where name = 'select_device'"
    );
    REQUIRE_NOTHROW (val = st.selectValue ());
    REQUIRE_NOTHROW (val.get (id_monitor));

    REQUIRE ( convert_asset_to_monitor_old (url.c_str(), id_asset) == id_monitor);
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
       "where name = 'select_device'"
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
