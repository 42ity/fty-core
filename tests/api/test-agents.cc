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
    \brief  Not yet documented file
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
}

TEST_CASE ("bios asset extended message encode/decode", "[agents][public_api][asset_extra]")
{
    log_open ();

    const char *name = "my_test_device";
    zhash_t *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");

    uint32_t type_id = 1;
    uint32_t subtype_id = 1;
    uint32_t parent_id = 1;
    const char *status = "active";
    uint8_t priority = 2;
    int8_t operation = 1;

    _scoped_ymsg_t *encoded_message = bios_asset_extra_encode (
            name,
            &ext_attributes,
            type_id,
            subtype_id,
            parent_id,
            status,
            priority,
            operation);
    REQUIRE (ext_attributes == NULL);


    _scoped_zhash_t *ext_attributes_new = NULL;
    char *name_new = NULL;
    uint32_t type_id_new = 0;
    uint32_t subtype_id_new = 0;
    uint32_t parent_id_new = 0;
    char *status_new = NULL;
    uint8_t priority_new = 0;
    int8_t operation_new = 0;
 
    int rv = bios_asset_extra_extract (encoded_message, &name_new, 
        &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
        &priority_new, &operation_new);
    REQUIRE ( rv == 0 );
    REQUIRE ( encoded_message != NULL );
    REQUIRE ( streq (name, name_new) == true );
    REQUIRE ( type_id == type_id_new );
    REQUIRE ( subtype_id == subtype_id_new );
    REQUIRE ( parent_id == parent_id_new );
    REQUIRE ( priority == priority_new );
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
    ymsg_destroy (&encoded_message);
    zhash_destroy(&ext_attributes_new);
    zhash_destroy(&ext_attributes);
}

/*
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
    uint32_t subtype_id   = 2;
    uint32_t parent_id = 2;
    const char *status = "active";
    uint8_t priority   = 3;
    int8_t operation   = 1;

    _scoped_zhash_t *ext_attributes_new = NULL;
    char *name_new         = NULL;
    uint32_t type_id_new   = 0;
    uint32_t subtype_id_new   = 0;
    uint32_t parent_id_new = 0;
    char *status_new       = NULL;
    uint8_t priority_new   = 0;
    int8_t operation_new   = 0;

    SECTION ("__type_id is missing") {

        app_t *app = app_new(APP_MODULE);
        REQUIRE ( app );
        app_set_name (app, "ASSET_EXTENDED");

        app_set_args        (app, &ext_attributes);
        app_args_set_uint32 (app, "__parent_id", parent_id);
        app_args_set_uint32 (app, "__subtype_id", subtype_id);
        app_args_set_uint8  (app, "__priority", priority);
        app_args_set_string (app, "__status", status);
        app_args_set_string (app, "__name", name);
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == 0 );
        REQUIRE ( subtype_id_new == subtype_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
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
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == 0 );
        REQUIRE ( priority_new == priority );
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
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == 0 );
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
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
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
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == -7 );
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

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
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
        app_args_set_int8   (app, "__operation", operation);

        ymsg_t *msg = ymsg_new(YMSG_SEND);
        REQUIRE ( msg );
        ymsg_request_set_app (msg, &app);

        int rv = bios_asset_extra_extract (msg, &name_new,
                &ext_attributes_new, &type_id_new, &subtype_id_new, &parent_id_new, &status_new,
                &priority_new, &operation_new);
        REQUIRE ( rv == 0 );
        REQUIRE ( msg != NULL );
        REQUIRE ( streq (name_new, name) == true );
        REQUIRE ( type_id_new == type_id );
        REQUIRE ( parent_id_new == parent_id );
        REQUIRE ( priority_new == priority );
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
}
*/
