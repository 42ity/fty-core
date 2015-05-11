#include <catch.hpp>

#include "dbpath.h"
#include "log.h"

#include "assetcrud.h"
#include "common_msg.h"

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
    auto reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    _scoped_zhash_t *reply_select = select_asset_element_attributes (conn, asset_element_id);
    REQUIRE ( zhash_size (reply_select) == 1 );
    char *value_s  = (char *) zhash_first  (reply_select);   // first value
    char *keytag_s = (char *) zhash_cursor (reply_select);   // key of this value
    REQUIRE ( strcmp(value_s, value) == 1 );
    REQUIRE ( !strcmp(keytag_s, keytag) );
    zhash_destroy (&reply_select);
   
    // true -> true 
    // must handle duplicate insert with the same value
    reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 0 );// the iserted row must not be updated
    REQUIRE ( rowid1 == rowid );

    // must handle duplicate insert with different value1
    reply_insert = insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 2 ); // as update statement was used
    REQUIRE ( rowid == rowid1 );    // the iserted row must be updated

    // true -> false
    // must handle duplicate insert with different value1
    reply_insert = insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 0 ); // 0 fail  , 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 ); // 0 fail  , 1 ok
    REQUIRE ( reply_delete.affected_rows == 1 );

    // check select
    reply_select = select_asset_element_attributes (conn, asset_element_id);
    REQUIRE ( zhash_size (reply_select) == 0 );

    // must handle second delete without crash
    reply_delete = delete_asset_ext_attribute (conn, keytag, asset_element_id);
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
    auto reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 1 ); // 0 fail  , 1 ok
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    _scoped_zhash_t *reply_select = select_asset_element_attributes (conn, asset_element_id);
    REQUIRE ( zhash_size (reply_select) == 1 );
    char *value_s  = (char *) zhash_first  (reply_select);   // first value
    char *keytag_s = (char *) zhash_cursor (reply_select);   // key of this value
    printf ("%s \n", value_s);
    REQUIRE ( strcmp(value_s, value) == 1 );
    REQUIRE ( !strcmp(keytag_s, keytag) );
    zhash_destroy (&reply_select);

    // -------------------------------  false -> false
    // must handle duplicate insert with the same value
    reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 0 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 ); // for pure insert 0 affected rows is unexpected value ->
                                                 // status = 0

    // must handle duplicate insert with different value1
    reply_insert = insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, read_only);
    REQUIRE ( reply_insert.status == 0 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 0 );

    // ------------------------------- false -> true , same
    // must handle duplicate insert with the same value 
    reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 1 );        // 0 fail, 1 ok
    uint64_t rowid1 = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 2 ); // insert_update should be uset, row should be updated
    REQUIRE ( rowid == rowid1 );

    // first delete
    auto reply_delete = delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 );        // 0 fail, 1 ok
    REQUIRE ( reply_delete.affected_rows == 1 );

    // check select
    reply_select = select_asset_element_attributes (conn, asset_element_id);
    REQUIRE ( zhash_size (reply_select) == 0 );

    // must handle second delete without crash
    reply_delete = delete_asset_ext_attribute (conn, keytag, asset_element_id);
    REQUIRE ( reply_delete.status == 1 ); // 0 fail  , 1 ok
    REQUIRE ( reply_delete.affected_rows == 0 );

    // ------------------------------- false ->true, different
    reply_insert = insert_into_asset_ext_attribute (conn, value, keytag, asset_element_id, read_only);
    rowid = reply_insert.rowid;
    
    // must handle duplicate insert with different value1
    reply_insert = insert_into_asset_ext_attribute (conn, value1, keytag, asset_element_id, !read_only);
    REQUIRE ( reply_insert.status == 1 );        // 0 fail, 1 ok
    REQUIRE ( reply_insert.affected_rows == 2 );
    rowid1 = reply_insert.rowid;
    REQUIRE ( rowid == rowid1 );

    reply_delete = delete_asset_ext_attribute (conn, keytag, asset_element_id);
    
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

    // first insert
    auto reply_insert = insert_into_asset_element (conn, element_name, element_type_id, parent_id, status, priority, bc);
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;
    CAPTURE (rowid);
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    _scoped_zmsg_t* reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), element_name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    REQUIRE ( zhash_size( asset_msg_ext (reply_select_decode) ) == 0 );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);
    
    // must handle duplicate insert without insert
    reply_insert = insert_into_asset_element (conn, element_name, element_type_id, parent_id, status, priority, bc);
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = delete_asset_element (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);

    // must handle second delete without crash
    reply_delete = delete_asset_element (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("asset device INSERT/DELETE #4","[db][CRUD][insert][delete][asset_device][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ASSET DEVICE DELETE/INSERT #4 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    a_elmnt_id_t   asset_element_id = 2; // it is written in crud_test.sql file
    a_dvc_tp_id_t  asset_device_type_id = 4; // TODO deal with map id to name
    const char*    asset_device_type = "pdu";

    // first insert
    auto reply_insert = insert_into_asset_device (conn, asset_element_id, asset_device_type_id);
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;

    // check select
    _scoped_zmsg_t *reply_select = select_asset_device (conn, asset_element_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_DEVICE);
    REQUIRE ( !strcmp(asset_msg_device_type (reply_select_decode), asset_device_type) );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);
    
    // must handle duplicate insert without insert
    reply_insert = insert_into_asset_device (conn, asset_element_id, asset_device_type_id);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_asset_device (conn, asset_element_id);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_device (conn, asset_element_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);

    // must handle second delete without crash
    reply_delete = delete_asset_device (conn, asset_element_id);
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
    auto reply_insert = insert_asset_element_into_asset_group (conn, asset_group_id, asset_element_id);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    std::set <a_elmnt_id_t> reply_select;
    REQUIRE_NOTHROW ( reply_select = select_asset_group_elements (conn, asset_group_id) );
    REQUIRE ( reply_select.size() == 1 );
    REQUIRE ( reply_select.count(asset_element_id) == 1 );
       
    // must handle duplicate insert without insert
    reply_insert = insert_asset_element_into_asset_group (conn, asset_group_id, asset_element_id);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_asset_element_from_asset_group (conn, asset_group_id, asset_element_id);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select.clear();
    REQUIRE_NOTHROW ( reply_select = select_asset_group_elements (conn, asset_group_id) );
    REQUIRE ( reply_select.size() == 0 );

    // must handle second delete without crash
    reply_delete = delete_asset_element_from_asset_group (conn, asset_group_id, asset_element_id);
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
    auto reply_insert = insert_into_asset_link (conn, asset_element_id_src, asset_element_id_dest, INPUT_POWER_CHAIN,
                                       src_out, dest_in);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zlist_t *reply_select = NULL;
    REQUIRE_NOTHROW ( reply_select = select_asset_device_links_all (conn, asset_element_id_src, INPUT_POWER_CHAIN) );
    REQUIRE ( zlist_size (reply_select) == 1 );
    REQUIRE ( !strcmp ( (char*)zlist_first (reply_select),"999:4:999:5"));// ATTENTION: string depends on input parameters;
    zlist_purge (reply_select);
    
       
    // must handle duplicate insert without insert
    reply_insert = insert_into_asset_link (conn, asset_element_id_src, asset_element_id_dest, INPUT_POWER_CHAIN,
                                       src_out, dest_in);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_asset_link (conn, asset_element_id_src, asset_element_id_dest);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    REQUIRE_NOTHROW ( reply_select = select_asset_device_links_all (conn, asset_element_id_src, INPUT_POWER_CHAIN));
    REQUIRE ( zlist_size (reply_select) == 0 );
    zlist_purge (reply_select);

    // must handle second delete without crash
    reply_delete = delete_asset_link  (conn, asset_element_id_src, asset_element_id_dest);
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
    auto reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key = (char*) zhash_cursor (reply_ext_attributes);
        // value = r:aaaaaa or value = w:aaaaa ; w,r means read_only or writable, need to remove these 2 characters, as they are not value
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value+2)));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_dc_room_row_rack (conn, rowid);
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
    auto reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key   = (char*) zhash_cursor (reply_ext_attributes);
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value + 2)));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_dc_room_row_rack (conn, rowid);
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
    auto reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key   = (char*) zhash_cursor (reply_ext_attributes);
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value+2)));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_dc_room_row_rack (conn, rowid);
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
    auto reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key   = (char*) zhash_cursor (reply_ext_attributes);
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value + 2)));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_dc_room_row_rack (conn, rowid);
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
    auto reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key   = (char*) zhash_cursor (reply_ext_attributes);
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value + 2)));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_dc_room_row_rack_group (conn, name, element_type_id, parent_id, ext_attributes, status, priority, bc, groups);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_dc_room_row_rack (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_dc_room_row_rack (conn, rowid);
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
    auto reply_insert = insert_device (conn, links, groups, name, parent_id,
                            ext_attributes, asset_device_type_id, status, priority, bc);
    uint64_t rowid = reply_insert.rowid;
    REQUIRE ( reply_insert.affected_rows == 1 );
    REQUIRE ( reply_insert.status == 1 );

    // check select
    //              element
    _scoped_zmsg_t *reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_asset_msg (reply_select) );
    _scoped_asset_msg_t *reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_ELEMENT);
    REQUIRE ( asset_msg_location (reply_select_decode) == parent_id );
    REQUIRE ( !strcmp(asset_msg_name (reply_select_decode), name) );
    REQUIRE ( asset_msg_type (reply_select_decode) == element_type_id );
    //              extattributes
    zhash_t *reply_ext_attributes = asset_msg_ext (reply_select_decode);
    REQUIRE ( zhash_size (reply_ext_attributes) == expected_ext_attributes.size() );
    std::set<std::pair<std::string, std::string>> real_ext_attributes;
    char *value = (char*) zhash_first  (reply_ext_attributes);
    while ( value != NULL)
    {
        char *key   = (char*) zhash_cursor (reply_ext_attributes);
        real_ext_attributes.insert (std::make_pair (std::string (key), std::string (value + 2 )));
        value = (char *) zhash_next (reply_ext_attributes);
    }
    REQUIRE ( real_ext_attributes == expected_ext_attributes );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);
    //              asset_device
    reply_select = select_asset_device (conn, rowid);
    REQUIRE ( is_asset_msg (reply_select) );
    reply_select_decode = asset_msg_decode (&reply_select);
    REQUIRE ( asset_msg_id (reply_select_decode) == ASSET_MSG_DEVICE);
    REQUIRE ( !strcmp(asset_msg_device_type (reply_select_decode), asset_device_type) );
    zmsg_destroy (&reply_select);
    asset_msg_destroy (&reply_select_decode);

    // second insert
    reply_insert = insert_device (conn, links, groups, name, parent_id, 
                            ext_attributes, asset_device_type_id, status, priority, bc);
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.status == 1 );

    // first delete
    auto reply_delete = delete_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_asset_element (conn, rowid, element_type_id);
    REQUIRE ( is_common_msg (reply_select) );
    _scoped_common_msg_t *creply_select_decode = common_msg_decode (&reply_select);
    REQUIRE ( common_msg_id (creply_select_decode) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errtype (creply_select_decode) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno (creply_select_decode) == DB_ERROR_NOTFOUND );
    // selects don't return count
    zmsg_destroy (&reply_select);
    common_msg_destroy (&creply_select_decode);
 
    // second delete
    reply_delete = delete_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );
    
    log_close();
}

