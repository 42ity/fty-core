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
 * \file test-asset-crud.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Gerald Guillaume <GeraldGuillaume@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>

#include "dbpath.h"
#include "log.h"

#include "assetcrud.h"
#include "db/assets.h"
#include "db/asset_general.h"
#include "common_msg.h"

#define UGLY_ASSET_TAG "0123456"

#include "cleanup.h"

TEST_CASE("asset ext attribute INSERT/DELETE #1","[db][CRUD][insert][delete][asset_ext_attribute][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ASSET EXT ATTRIBUTE DELETE/INSERT #1 NULL ->true, true->true ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    a_elmnt_id_t  asset_element_id = 1;  // it is written in crud_test.sql file
    const char   *value            = "What is this life if, full of care, we have no time to stand and stare";
    const char   *value1            = "111";
    const char   *keytag           = "INSERTDELETEtest1";
    bool          read_only        = true; // -> second insert should be update

    // first insert
    auto reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );

    auto reply_select = persist::select_ext_attributes (conn, asset_element_id);
    REQUIRE (reply_select.status == 1);
    auto ext = reply_select.item;
    REQUIRE (ext.count(keytag) == 1);
    REQUIRE (ext[keytag].first == value);
    REQUIRE (ext[keytag].second == read_only);

    // true -> true 
    // must handle duplicate insert with the same value
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 0 );// the iserted row must not be updated
    REQUIRE ( rowid1 == rowid );

    // must handle duplicate insert with different value1
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 2 ); // as update statement was used
    REQUIRE ( rowid == rowid1 );    // the iserted row must be updated

    // true -> false
    // must handle duplicate insert with different value1
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 0 ); // 0 fail  , 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = persist::delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 ); // 0 fail  , 1 ok
    REQUIRE ( reply_delete.affected_rows == 1 );

    // check select
    reply_select = persist::select_ext_attributes (conn, asset_element_id);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 0 );

    // must handle second delete without crash
    reply_delete = persist::delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 ); // 0 fail  , 1 ok
    REQUIRE ( reply_delete.affected_rows == 0 );

    log_close();
}


TEST_CASE("asset ext attribute INSERT/DELETE #2","[db][CRUD][insert][delete][asset_ext_attribute][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ASSET EXT ATTRIBUTE DELETE/INSERT #2 NULL -> false, false->false ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
    
    a_elmnt_id_t  asset_element_id = 1;  // it is written in crud_test.sql file
    const char   *value            = "What is this life if, full of care, we have no time to stand and stare";
    const char   *value1            = "111";
    const char   *keytag           = "INSERTDELETEtest22";
    bool          read_only        = false; // -> second insert should be update

    // first insert
    auto reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );

    auto reply_select = persist::select_ext_attributes(conn, asset_element_id);
    REQUIRE (reply_select.status == 1);
    REQUIRE (reply_select.item.size() == 1);
    auto ext = reply_select.item;
    REQUIRE (ext.count(keytag) == 1);
    REQUIRE (ext[keytag].first == value);
    REQUIRE (ext[keytag].second == read_only);

    // -------------------------------  false -> false
    // must handle duplicate insert with the same value
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 0 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 ); // for pure insert 0 affected rows is unexpected value ->
                                                 // status = 0

    // must handle duplicate insert with different value1
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 0 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 );

    // ------------------------------- false -> true , same
    // must handle duplicate insert with the same value 
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 1 );        // 0 fail, 1 ok
    uint64_t rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 2 ); // insert_update should be uset, row should be updated
    REQUIRE ( rowid == rowid1 );

    // first delete
    auto reply_delete = persist::delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 );        // 0 fail, 1 ok
    REQUIRE ( reply_delete.affected_rows == 1 );

    reply_select = persist::select_ext_attributes (conn, asset_element_id);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 0 );

    // must handle second delete without crash
    reply_delete = persist::delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 ); // 0 fail  , 1 ok
    REQUIRE ( reply_delete.affected_rows == 0 );

    // ------------------------------- false ->true, different
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    rowid = reply_insert.rowid;

    // must handle duplicate insert with different value1
    reply_insert = persist::insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 1 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 2 );
    rowid1 = reply_insert.rowid;
    REQUIRE ( rowid == rowid1 );

    reply_delete = persist::delete_asset_ext_attribute (conn, keytag, asset_element_id);
    
    log_close();
}


// general standart case
// TODO: alternaternative flow: too long name or other

TEST_CASE("asset element INSERT/DELETE #3","[db][CRUD][insert][delete][asset_element][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ASSET ELEMENT DELETE/INSERT #3 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
    
    a_elmnt_id_t     parent_id       = 1;  // it is written in crud_test.sql file
    a_elmnt_tp_id_t  element_type_id = asset_type::ROOM;
    const char      *element_name    = "Room_insert_test";
    const char *status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    a_dvc_tp_id_t subtype_id = 10;

    // first insert
    auto reply_insert = persist::insert_into_asset_element (conn, element_name, element_type_id,
        parent_id, status, priority, bc, subtype_id, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;
    CAPTURE (rowid);
    REQUIRE ( reply_insert.affected_rows == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == element_name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == asset_type::ROOM);
    REQUIRE (item.type_name == "room");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 2);     // in crud_test.sql
    REQUIRE (item.subtype_id == subtype_id);

    // must handle duplicate insert without insert
    reply_insert = persist::insert_into_asset_element (conn, element_name, element_type_id,
            parent_id, status, priority, bc, 10, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = persist::delete_asset_element (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);

    // must handle second delete without crash
    reply_delete = persist::delete_asset_element (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}


TEST_CASE("into asset group INSERT/DELETE #5","[db][CRUD][insert][delete][grp_element][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ASSET ELEMENT INTO GROUP INSERT/DELETE #5 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    a_elmnt_id_t asset_element_id = 2; // it is written in crud_test.sql file
    a_elmnt_id_t asset_group_id = 3; // it is written in crud_test.sql file

    // first insert
    auto reply_insert = persist::insert_asset_element_into_asset_group (conn, asset_group_id, asset_element_id);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    std::set <a_elmnt_id_t> reply_select;
    REQUIRE_NOTHROW ( reply_select = select_asset_group_elements (conn, asset_group_id) );
    REQUIRE ( reply_select.size() == 1 );
    REQUIRE ( reply_select.count(asset_element_id) == 1 );
       
    // must handle duplicate insert without insert
    reply_insert = persist::insert_asset_element_into_asset_group (conn, asset_group_id, asset_element_id);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_asset_element_from_asset_group (conn, asset_group_id, asset_element_id);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select.clear();
    REQUIRE_NOTHROW ( reply_select = select_asset_group_elements (conn, asset_group_id) );
    REQUIRE ( reply_select.size() == 0 );

    // must handle second delete without crash
    reply_delete = persist::delete_asset_element_from_asset_group (conn, asset_group_id, asset_element_id);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}


TEST_CASE("into asset link INSERT/DELETE #6","[db][CRUD][insert][delete][asset_link][crud_test.sql][66]")
{
    log_open ();

    log_info ("=============== ASSET LINK INSERT/DELETE #6 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    a_elmnt_id_t asset_element_id_src  = 4; // it is written in crud_test.sql file
    a_elmnt_id_t asset_element_id_dest = 5; // it is written in crud_test.sql file
    
    const a_lnk_src_out_t src_out = SRCOUT_DESTIN_IS_NULL;
    const a_lnk_dest_in_t dest_in = SRCOUT_DESTIN_IS_NULL;
    // first insert
    auto reply_insert = persist::insert_into_asset_link (conn, asset_element_id_src, asset_element_id_dest, INPUT_POWER_CHAIN,
                                       src_out, dest_in);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    _scoped_zlist_t *reply_select = NULL;
    REQUIRE_NOTHROW ( reply_select = select_asset_device_links_all (conn, asset_element_id_src, INPUT_POWER_CHAIN) );
    REQUIRE ( zlist_size (reply_select) == 1 );
    REQUIRE ( !strcmp ( (char*)zlist_first (reply_select),"999:4:999:5"));// ATTENTION: string depends on input parameters;
    zlist_purge (reply_select);
    
       
    // must handle duplicate insert without insert
    reply_insert = persist::insert_into_asset_link (conn, asset_element_id_src, asset_element_id_dest, INPUT_POWER_CHAIN,
                                       src_out, dest_in);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_asset_link (conn, asset_element_id_src, asset_element_id_dest);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    REQUIRE_NOTHROW ( reply_select = select_asset_device_links_all (conn, asset_element_id_src, INPUT_POWER_CHAIN));
    REQUIRE ( zlist_size (reply_select) == 0 );
    zlist_purge (reply_select);

    // must handle second delete without crash
    reply_delete = persist::delete_asset_link  (conn, asset_element_id_src, asset_element_id_dest);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );
    
    zlist_destroy (&reply_select);
    log_close();
}


TEST_CASE("dc unlockated INSERT/DELETE #7","[db][CRUD][insert][delete][dc][unlockated][crud_test.sql]")
{
    log_open ();

    log_info ("=============== dc INSERT/DELETE #7 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "DC_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::DATACENTER;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char *status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    std::set <a_elmnt_id_t>  groups;    // left empty, simple cass

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people"));
    expected_ext_attributes.insert (std::make_pair ("total_facility_load", "123456"));
    expected_ext_attributes.insert (std::make_pair ("company", "EATON"));
    expected_ext_attributes.insert (std::make_pair ("site_name", "www.eaton.com"));
    expected_ext_attributes.insert (std::make_pair ("region", "EMEA"));
    expected_ext_attributes.insert (std::make_pair ("country", "Czech Republic"));
    expected_ext_attributes.insert (std::make_pair ("address", "some nice place in Czech Republic"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));
    
    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    // first insert
    auto reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id,
                parent_id, ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "datacenter");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 10); // 10 magic->default from initdb.sql

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }

    // second insert
    reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("room unlockated INSERT/DELETE #8","[db][CRUD][insert][delete][unlockated][room][crud_test.sql]")
{
    log_open ();

    log_info ("=============== room INSERT/DELETE #8 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "ROOM_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::ROOM;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char* status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    std::set <a_elmnt_id_t>  groups;    // left empty, simple cass

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));
    
    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    // first insert
    auto reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id,
            parent_id, ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "room");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 10); //magic

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }
    // second insert
    reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("row unlockated INSERT/DELETE #9","[db][CRUD][insert][delete][unlockated][row][crud_test.sql]")
{
    log_open ();

    log_info ("=============== row INSERT/DELETE #9 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "ROW_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::ROW;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char* status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    std::set <a_elmnt_id_t>  groups;    // left empty, simple cass

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));

    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    // first insert
    auto reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG) ;
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "row");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 10); //magic

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }

    // second insert
    reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("rack unlockated INSERT/DELETE #10","[db][CRUD][insert][delete][unlockated][rack][crud_test.sql]")
{
    log_open ();

    log_info ("=============== rack INSERT/DELETE #10 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "RACK_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::RACK;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char* status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    std::set <a_elmnt_id_t>  groups;    // left empty, simple cass

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));
    expected_ext_attributes.insert (std::make_pair ("free_space", "123"));
    expected_ext_attributes.insert (std::make_pair ("manufacturer", "Be calm"));      // TODO should we check this parametr??
    expected_ext_attributes.insert (std::make_pair ("model", "2"));
    expected_ext_attributes.insert (std::make_pair ("u_size", "3"));
    expected_ext_attributes.insert (std::make_pair ("asset_tag", "This doesn't matter"));
    
    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    // first insert
    auto reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "rack");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 10); // magic

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }
    // second insert
    reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id,
            parent_id, ext_attributes, status, priority, bc, groups,
            UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("group unlockated INSERT/DELETE #11","[db][CRUD][insert][delete][unlockated][group][crud_test.sql]")
{
    log_open ();

    log_info ("=============== group INSERT/DELETE #11 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "GROUP_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::GROUP;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char* status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;
    std::set <a_elmnt_id_t>  groups;    // left empty, simple cass

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people, do we have any problems with % and %% characters"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));
    
    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    // first insert
    auto reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id,
            parent_id, ext_attributes, status, priority, bc, groups,
            UGLY_ASSET_TAG);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "group");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 10);  //magic

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }
    // second insert
    reply_insert = persist::insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id,
            ext_attributes, status, priority, bc, groups, UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}


TEST_CASE("device unlockated INSERT/DELETE #12","[db][CRUD][insert][delete][unlockated][device][crud_test.sql]")
{
    log_open ();

    log_info ("=============== device INSERT/DELETE #12 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char      *name = "DEVICE_TEST";
    a_elmnt_tp_id_t  element_type_id = asset_type::DEVICE;
    a_elmnt_id_t     parent_id = 0;                     // unlockated
    _scoped_zhash_t         *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    const char* status = "active";
    a_elmnt_pr_t priority   = 4;
    a_elmnt_bc_t bc         = 0;

    std::set<std::pair<std::string, std::string>> expected_ext_attributes;
    expected_ext_attributes.insert (std::make_pair ("description", "Hello people, do we have any problems with % and %% characters"));
    expected_ext_attributes.insert (std::make_pair ("contact_name", "thisisanemailaddress@gmail.com"));
    
    for ( auto &ea : expected_ext_attributes )
        zhash_insert (ext_attributes, ea.first.c_str(), (void *)ea.second.c_str());

    
    std::vector <link_t>        links;      // left empty, simple case
    std::set <a_elmnt_id_t>  groups;     // left empty, simple case
    _scoped_zhash_t       *extattributes = NULL; // left empty, simple case, TODO with NEW
    a_dvc_tp_id_t  asset_device_type_id = 4;
    const char    *asset_device_type = "pdu";

    // first insert
    auto reply_insert = persist::insert_device (conn, links, groups, name, parent_id,
                            ext_attributes, asset_device_type_id,
                            asset_device_type, status, priority, bc,
                            UGLY_ASSET_TAG);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    auto reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 1);
    auto item = reply_select.item;
    REQUIRE (item.id == rowid);
    REQUIRE (item.name == name);
    REQUIRE (item.status == status);
    REQUIRE (item.priority == priority);
    REQUIRE (item.bc == bc);
    REQUIRE (item.type_id == element_type_id);
    REQUIRE (item.type_name == "device");
    REQUIRE (item.parent_id == parent_id);
    REQUIRE (item.parent_type_id == 0);     // in crud_test.sql
    REQUIRE (item.subtype_id == 4);

    auto reply_ext = persist::select_ext_attributes(conn, rowid);
    REQUIRE (reply_ext.status == 1);
    auto ext = reply_ext.item;
    for (const auto& p : expected_ext_attributes) {
        REQUIRE(ext[p.first].first == p.second);
        //TODO: check the read_only attribute
    }
    // second insert
    reply_insert = persist::insert_device (conn, links, groups, name, parent_id,
                            ext_attributes, asset_device_type_id,
                            asset_device_type, status, priority, bc,
                            UGLY_ASSET_TAG);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = persist::delete_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    reply_select = persist::select_asset_element_web_byId(conn, rowid);
    REQUIRE (reply_select.status == 0);
    REQUIRE (reply_select.errtype == BIOS_ERROR_DB);
    REQUIRE (reply_select.errsubtype == DB_ERROR_NOTFOUND);
    // second delete
    reply_delete = persist::delete_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );
    
    log_close();
}
