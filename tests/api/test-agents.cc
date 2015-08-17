/*
Copyright (C) 2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   test-agents.cc
    \brief  TODO
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
    \author Michal Hrusecky <MichalHrusecky@Eaton.com>
    \author Karol Hrdina <KarolHrdina@Eaton.com>
    \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include <catch.hpp>
#include <stdio.h>

#include "dbpath.h"
#include "log.h"
#include "agents.h"
#include "utils.h"
#include "utils_app.h"
#include "utils_ymsg.h"
#include "cleanup.h"

TEST_CASE(" inventory message encode/decode","[db][ENCODE][DECODE][bios_inventory]")
{
    log_open ();

    const char *device_name = "my_test_device";
    _scoped_zhash_t    *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");
    const char *module_name = "inventory";

    _scoped_ymsg_t * ymsg_encoded = bios_inventory_encode (device_name, &ext_attributes, module_name);
    REQUIRE ( ymsg_encoded != NULL );
    REQUIRE ( ext_attributes == NULL );
//    ymsg_print (ymsg_encoded);

    _scoped_char    *device_name_new = NULL;
    _scoped_zhash_t *ext_attributes_new = NULL;
    _scoped_char    *module_name_new = NULL;

    int rv = bios_inventory_decode (&ymsg_encoded, &device_name_new, &ext_attributes_new, &module_name_new);

    REQUIRE ( rv == 0 );
    REQUIRE ( ymsg_encoded == NULL );
    REQUIRE ( strcmp (device_name, device_name_new) == 0 );
    REQUIRE ( strcmp (module_name, module_name_new) == 0 );
    REQUIRE ( zhash_size (ext_attributes_new) == 3 );
    
    const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
    REQUIRE ( strcmp (value1, "value1") == 0 );

    const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
    REQUIRE ( strcmp (value2, "value2") == 0 );

    const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
    REQUIRE ( strcmp (value3, "value3") == 0 );

    FREE0 (device_name_new)
    FREE0 (module_name_new)
    zhash_destroy(&ext_attributes_new);

    log_close ();
}

TEST_CASE ("Functions fail for bad input arguments", "[agents][public_api]") {

    SECTION ("bios_web_average_request_encode") {
        CHECK ( bios_web_average_request_encode (0, 0, NULL, "",   0, "") == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, "",   NULL, 0, "") == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, "",   "",   0, NULL) == NULL );

        CHECK ( bios_web_average_request_encode (0, 0, NULL, NULL, 0, "") == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, NULL, "",   0, NULL) == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, "",   NULL, 0, NULL) == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, NULL, NULL, 0, NULL) == NULL );
    }

    SECTION ("bios_web_average_request_extract") {
        int64_t start_ts = -1, end_ts = -1;
        char *type = NULL, *step = NULL, *source = NULL;
        uint64_t element_id = 0;
        _scoped_ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);
        
        CHECK ( bios_web_average_request_extract (NULL, &start_ts, &end_ts, &type, &step, &element_id, &source) == -1 );
        CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, NULL,      &end_ts, &type, &step, &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, &start_ts, NULL,    &type, &step, &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, &start_ts, &end_ts, NULL,  &step, &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, &start_ts, &end_ts, &type, NULL,  &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, &start_ts, &end_ts, &type, &step, NULL,        &source) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_web_average_request_extract (msg, &start_ts, &end_ts, &type, &step, &element_id, NULL) == -1 );
        CHECK ( msg ); CHECK ( type == NULL); CHECK ( step == NULL); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        ymsg_destroy (&msg);
    }

    SECTION ("bios_web_average_reply_encode") {
        CHECK ( bios_web_average_reply_encode (NULL) == NULL );
    }

    SECTION ("bios_web_average_reply_extract") {
        _scoped_char *json = NULL;
        _scoped_ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);
        CHECK ( bios_web_average_reply_extract (NULL, &json) == -1 );
        CHECK ( json == NULL );

        CHECK ( bios_web_average_reply_extract (msg, NULL) == -1 );
        CHECK ( msg );
        ymsg_destroy (&msg);
    }

    SECTION ("bios_db_measurements_read_request_encode") {
        _scoped_char *subject = NULL;
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, NULL, &subject) == NULL );
        CHECK ( subject == NULL );
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, "", NULL) == NULL );
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, NULL, NULL) == NULL );
    }

    SECTION ("bios_db_measurements_read_request_extract") {
        int64_t start_ts = -1, end_ts = -1;
        _scoped_char *source = NULL;
        uint64_t element_id = 0;
        _scoped_ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);
       
        _scoped_ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        REQUIRE (msg_reply);

        CHECK ( bios_db_measurements_read_request_extract (NULL, &start_ts, &end_ts, &element_id, &source) == -1 );
        CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_db_measurements_read_request_extract (msg_reply, &start_ts, &end_ts, &element_id, &source) == -1 );
        CHECK ( msg_reply ); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_db_measurements_read_request_extract (msg, NULL, &end_ts, &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( source == NULL); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        CHECK ( bios_db_measurements_read_request_extract (msg, &start_ts, NULL, &element_id, &source) == -1 );
        CHECK ( msg ); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( element_id == 0 );

        CHECK ( bios_db_measurements_read_request_extract (msg, &start_ts, &end_ts, NULL, &source) == -1 );
        CHECK ( msg ); CHECK ( source == NULL); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1);

        CHECK ( bios_db_measurements_read_request_extract (msg, &start_ts, &end_ts, &element_id, NULL) == -1 );
        CHECK ( msg ); CHECK ( start_ts == -1 ); CHECK ( end_ts == -1); CHECK ( element_id == 0 );

        ymsg_destroy (&msg);
        ymsg_destroy (&msg_reply);
    }

    // TODO: finish
    // _scoped_ymsg_t * bios_db_measurements_read_reply_encode (const char *);
    // SECTION ("bios_db_measurements_read_reply_encode") {
    // }

    // TODO: finish
    // int bios_db_measurements_read_reply_extract (_scoped_ymsg_t **self_p, char **json);
    // SECTION ("bios_db_measurements_read_reply_extract") {
    // }


}

TEST_CASE ("bios web average request encoded & decoded", "[agents][public_api]") {
    int64_t start_ts = 1428928778;
    int64_t end_ts = 1428821234;
    const char *type = "arithmetic_mean";
    const char *step = "8h";
    uint64_t element_id = 412;
    const char *source = "temperature.default";

    _scoped_ymsg_t *msg = bios_web_average_request_encode (start_ts, end_ts, type, step, element_id, source);
    REQUIRE (msg);

    int64_t start_ts_r;
    int64_t end_ts_r;
    _scoped_char *type_r = NULL, *step_r = NULL, *source_r = NULL;
    uint64_t element_id_r = 0;
   
    int rv = bios_web_average_request_extract (msg, &start_ts_r, &end_ts_r, &type_r, &step_r, &element_id_r, &source_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK ( rv == 0 );

    CHECK ( type_r );
    CHECK ( step_r );
    CHECK ( source_r );

    CHECK ( start_ts == start_ts_r );
    CHECK ( end_ts == end_ts_r );
    CHECK ( str_eq (type, type_r) );
    CHECK ( str_eq (step, step_r) );
    CHECK ( element_id == element_id_r );
    CHECK ( str_eq (source, source_r) );


    FREE0 (type_r)
    FREE0 (step_r)
    FREE0 (source_r)
}

TEST_CASE ("bios web average reply encoded & decoded", "[agents][public_api]") {
    const char *json = "abrakadabra";
    _scoped_ymsg_t *msg = bios_web_average_reply_encode (json);
    REQUIRE ( msg );

    _scoped_char *json_r = NULL;
    int rv = bios_web_average_reply_extract (msg, &json_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK ( rv == 0 );

    CHECK ( json_r );
    CHECK ( str_eq (json, json_r) );
    FREE0 (json_r)
}

TEST_CASE ("bios db measurement read request encoded & decoded", "[agents][public_api]") {
    int64_t start_ts = 1428928778;
    int64_t end_ts = 14287322211;
    uint64_t element_id = 412;
    const char *source = "temperature.thermal_zone0";
    _scoped_char *subject = NULL;
   
    _scoped_ymsg_t *msg = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &subject);
    CHECK ( msg );
    CHECK ( subject );
    CHECK ( str_eq (subject, "get_measurements") );
    FREE0 (subject)

    int64_t start_ts_r = -1;
    int64_t end_ts_r = -1;
    uint64_t element_id_r = 0;
    _scoped_char *source_r = NULL;

    int rv = bios_db_measurements_read_request_extract (msg, &start_ts_r, &end_ts_r, &element_id_r, &source_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK (rv == 0);
    
    CHECK ( source_r );

    CHECK ( start_ts == start_ts_r );
    CHECK ( end_ts == end_ts_r );
    CHECK ( element_id == element_id_r );
    CHECK ( str_eq (source, source_r) );
    FREE0 (source_r)
}

TEST_CASE ("bios db measurement read reply encoded & decoded", "[agents][public_api]") {
    const char *json = "{ \"key\" : \"value\", \"key2\" : [1, 2, 3, 4]}";
    _scoped_ymsg_t *msg = bios_db_measurements_read_reply_encode (json);
    CHECK ( msg );

    _scoped_char *json_r = NULL;
    int rv = bios_db_measurements_read_reply_extract (msg, &json_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK ( rv == 0 );
    CHECK ( json_r );
    CHECK ( str_eq (json, json_r) );
    FREE0 (json_r)
}

TEST_CASE ("bios alert message encoded & decoded", "[agents][public_api]") {

    _scoped_ymsg_t *msg = bios_alert_encode(
        "testrule",
        ALERT_PRIORITY_P2,
        ALERT_STATE_ONGOING_ALERT,
        "myDev",
        "some text",
        42);
    REQUIRE ( msg );

    _scoped_char *rule = NULL, *devices = NULL, *description = NULL;
    uint8_t priority = ALERT_PRIORITY_UNKNOWN;
    int8_t state = ALERT_STATE_UNKNOWN;
    time_t time = 0;

    bios_alert_extract( msg, NULL, &priority, &state, &devices, &description, &time);
    CHECK( msg );
    
    int x = bios_alert_extract( msg, &rule, &priority, &state, &devices, &description, &time);
    REQUIRE ( x == 0 );
    CHECK ( msg != NULL );
    CHECK ( strcmp (rule, "testrule") == 0 );
    CHECK ( strcmp (devices, "myDev") == 0 );
    CHECK ( strcmp (description, "some text") == 0 );
    CHECK ( priority == ALERT_PRIORITY_P2 );
    CHECK ( state == ALERT_STATE_ONGOING_ALERT );
    CHECK ( time == 42 );
    FREE0 (rule)
    FREE0 (devices)
    FREE0 (description)
    ymsg_destroy( &msg );
}

TEST_CASE ("bios alsset message encoded & decoded", "[agents][public_api]") {

    ymsg_t *msg = bios_asset_encode(
        "device1",
        1,
	3,
        2,
        "ok",
        1,
        1);

    REQUIRE ( msg );

    char *device = NULL, *status = NULL;
    uint32_t type_id = 0;
    uint32_t subtype_id = 0;
    uint32_t parent_id = 0;
    uint8_t priority = 0;
    int8_t action_type = 0;

    int x = bios_asset_extract( msg, &device, &type_id, &subtype_id, &parent_id, &status, &priority, &action_type);
    REQUIRE( msg );
    REQUIRE ( x == 0 );
    CHECK ( str_eq( device, "device1") );
    CHECK ( type_id == 1 );
    CHECK ( subtype_id == 3 );
    CHECK ( parent_id == 2 );
    CHECK ( str_eq( status, "ok" ) );
    CHECK ( priority == 1 );
    CHECK ( action_type == 1 );
    ymsg_destroy( &msg );
    FREE0( device );
    FREE0( status );
}


TEST_CASE ("bios asset extended message encode & decode", "[agents][public_api][asset_extra]") 
{
    log_open ();

    const char *name = "my_test_device";
    _scoped_zhash_t *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");
    uint32_t type_id = 1;
    uint32_t parent_id = 1;
    const char *status = "active";
    uint8_t priority = 2;
    uint8_t bc = 1;
    int8_t operation = 1;
    _scoped_ymsg_t * ymsg_encoded = bios_asset_extra_encode
        (name, &ext_attributes, type_id, parent_id, status, priority, bc, operation);
    REQUIRE ( ymsg_encoded != NULL );
    REQUIRE ( ext_attributes == NULL );

    _scoped_zhash_t *ext_attributes_new = NULL;

    char *name_new = NULL;
    uint32_t type_id_new = 0;
    uint32_t parent_id_new = 0;
    char *status_new = NULL;
    uint8_t priority_new = 0;
    uint8_t bc_new = 0;
    int8_t operation_new = 0;
 
    int rv = bios_asset_extra_extract (ymsg_encoded, &name_new, 
        &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
        &priority_new, &bc_new, &operation_new);
    REQUIRE ( rv == 0 );
    REQUIRE ( ymsg_encoded != NULL );
    REQUIRE ( streq (name, name_new) == true );
    REQUIRE ( type_id == type_id_new );
    REQUIRE ( parent_id == parent_id_new );
    REQUIRE ( priority == priority_new );
    REQUIRE ( bc == bc_new );
    REQUIRE ( operation == operation_new );
    REQUIRE ( streq (status, status_new) == true );
    REQUIRE ( zhash_size (ext_attributes_new) == 3 );
    
    const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
    REQUIRE ( streq (value1, "value1") == true );

    const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
    REQUIRE ( streq (value2, "value2") == true );

    const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
    REQUIRE ( streq (value3, "value3") == true );

    FREE0 (status_new)
    FREE0 (name_new)
    zhash_destroy(&ext_attributes_new);
    zhash_destroy(&ext_attributes);

    log_close ();
}


TEST_CASE ("bios asset extended message decode", "[agents][public_api][asset_extra]") 
{
    log_open ();

    const char *name = "my_test_device";
    zhash_t *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");
    
    uint32_t type_id = 1;
    uint32_t parent_id = 1;
    const char *status = "active";
    uint8_t priority = 2;
    uint8_t bc = 1;
    int8_t operation = 1;

    app_t *app = app_new(APP_MODULE);
    REQUIRE ( app );
    app_set_name (app, "ASSET_EXTENDED");
        
    app_set_args  (app, &ext_attributes);
    app_args_set_uint32 (app, "__type_id", type_id);
    app_args_set_uint32 (app, "__parent_id", parent_id);
    app_args_set_uint8  (app, "__priority", priority);
    app_args_set_string (app, "__status", status);
    app_args_set_string (app, "__name", name);
    app_args_set_uint8  (app, "__bc", bc);
    app_args_set_int8   (app, "__operation", operation);
    
    ymsg_t *msg = ymsg_new(YMSG_SEND);
    REQUIRE ( msg );
    ymsg_request_set_app (msg, &app);

    _scoped_zhash_t *ext_attributes_new = NULL;
    char *name_new = NULL;
    uint32_t type_id_new = 0;
    uint32_t parent_id_new = 0;
    char *status_new = NULL;
    uint8_t priority_new = 0;
    uint8_t bc_new = 0;
    int8_t operation_new = 0;
 
    int rv = bios_asset_extra_extract (msg, &name_new, 
        &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
        &priority_new, &bc_new, &operation_new);
    REQUIRE ( rv == 0 );
    REQUIRE ( msg != NULL );
    REQUIRE ( streq (name, name_new) == true );
    REQUIRE ( type_id == type_id_new );
    REQUIRE ( parent_id == parent_id_new );
    REQUIRE ( priority == priority_new );
    REQUIRE ( bc == bc_new );
    REQUIRE ( operation == operation_new );
    REQUIRE ( streq (status, status_new) == true );
    REQUIRE ( zhash_size (ext_attributes_new) == 3 );
    
    const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
    REQUIRE ( streq (value1, "value1") == true );

    const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
    REQUIRE ( streq (value2, "value2") == true );

    const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
    REQUIRE ( streq (value3, "value3") == true );

    FREE0 (status_new)
    FREE0 (name_new)
    zhash_destroy(&ext_attributes_new);
    zhash_destroy(&ext_attributes);

    log_close ();
}


TEST_CASE ("bios asset extended message decode, missing keys", "[agents][public_api][asset_extra][1]")
{
    log_open ();

    _scoped_zhash_t *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");
    
    const char *name   = "my_test_device";
    uint32_t type_id   = 1;
    uint32_t parent_id = 2;
    const char *status = "active";
    uint8_t priority   = 3;
    uint8_t bc         = 1;
    int8_t operation   = 1;

    _scoped_zhash_t *ext_attributes_new = NULL;
    char *name_new         = NULL;
    uint32_t type_id_new   = 0;
    uint32_t parent_id_new = 0;
    char *status_new       = NULL;
    uint8_t priority_new   = 0;
    uint8_t bc_new         = 0;
    int8_t operation_new   = 0;

    SECTION ("__type_id is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == 0 );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == operation );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }

    SECTION ("__parent_id is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == 0 );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == operation );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__priority is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == 0 );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == operation );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__status is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == operation );
        REQUIRE ( status_new == NULL );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__name is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == -7 );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__bc is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == 0 );
        REQUIRE ( operation_new == operation );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__operation is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == 0 );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 3 );

        const char *value1 = (char *) zhash_lookup (ext_attributes_new, "key1");
        REQUIRE ( streq (value1, "value1") == true );

        const char *value2 = (char*) zhash_lookup (ext_attributes_new, "key2");
        REQUIRE ( streq (value2, "value2") == true );

        const char *value3 = (char *) zhash_lookup (ext_attributes_new, "key3");
        REQUIRE ( streq (value3, "value3") == true );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }
    SECTION ("__ext_attributes is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_args_set_uint32 (app, "__type_id", type_id);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_uint8  (app, "__bc", bc);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new, 
                &ext_attributes_new, &type_id_new, &parent_id_new, &status_new,
                &priority_new, &bc_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
        REQUIRE ( bc_new == bc );
        REQUIRE ( operation_new == operation );
        REQUIRE ( streq (status_new, status) == true );
        REQUIRE ( zhash_size (ext_attributes_new) == 0 );
        ymsg_destroy (&msg);
        app_destroy (&app);
    }

    FREE0 (status_new)
    FREE0 (name_new)
    zhash_destroy(&ext_attributes_new);
    zhash_destroy(&ext_attributes);

    log_close ();
}
