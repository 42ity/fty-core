#include <catch.hpp>
#include <stdio.h>

#include "dbpath.h"
#include "log.h"
#include "agents.h"
#include "utils.h"

TEST_CASE(" inventory message encode/decode","[db][ENCODE][DECODE][bios_inventory]")
{
    log_open ();

    const char *device_name = "my_test_device";
    zhash_t    *ext_attributes = zhash_new();
    zhash_autofree (ext_attributes);
    zhash_insert (ext_attributes, "key1", (char*)"value1");
    zhash_insert (ext_attributes, "key2", (char*)"value2");
    zhash_insert (ext_attributes, "key3", (char*)"value3");
    const char *module_name = "inventory";

    ymsg_t * ymsg_encoded = bios_inventory_encode (device_name, &ext_attributes, module_name);
    REQUIRE ( ymsg_encoded != NULL );
    REQUIRE ( ext_attributes == NULL );
    ymsg_print (ymsg_encoded);

    char    *device_name_new = NULL;
    zhash_t *ext_attributes_new = NULL;
    char    *module_name_new = NULL;

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

    free(device_name_new);
    free(module_name_new);
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
        ymsg_t *msg = ymsg_new (YMSG_SEND);
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
        char *json = NULL;
        ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);
        CHECK ( bios_web_average_reply_extract (NULL, &json) == -1 );
        CHECK ( json == NULL );

        CHECK ( bios_web_average_reply_extract (msg, NULL) == -1 );
        CHECK ( msg );
        ymsg_destroy (&msg);
    }

    SECTION ("bios_db_measurements_read_request_encode") {
        char *subject = NULL;
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, NULL, &subject) == NULL );
        CHECK ( subject == NULL );
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, "", NULL) == NULL );
        CHECK ( bios_db_measurements_read_request_encode (0, 0, 0, NULL, NULL) == NULL );
    }

    SECTION ("bios_db_measurements_read_request_extract") {
        int64_t start_ts = -1, end_ts = -1;
        char *source = NULL;
        uint64_t element_id = 0;
        ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);
       
        ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
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
    // ymsg_t * bios_db_measurements_read_reply_encode (const char *);
    // SECTION ("bios_db_measurements_read_reply_encode") {
    // }

    // TODO: finish
    // int bios_db_measurements_read_reply_extract (ymsg_t **self_p, char **json);
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

    ymsg_t *msg = bios_web_average_request_encode (start_ts, end_ts, type, step, element_id, source);
    REQUIRE (msg);

    int64_t start_ts_r;
    int64_t end_ts_r;
    char *type_r = NULL;
    char *step_r = NULL;
    uint64_t element_id_r = 0;
    char *source_r = NULL;
   
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

    if (type_r)
        free (type_r);
    if (step_r)
        free (step_r);
    if (source_r)
        free (source_r);
}

TEST_CASE ("bios web average reply encoded & decoded", "[agents][public_api]") {
    const char *json = "abrakadabra";
    ymsg_t *msg = bios_web_average_reply_encode (json);
    REQUIRE ( msg );

    char *json_r = NULL;
    int rv = bios_web_average_reply_extract (msg, &json_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK ( rv == 0 );

    CHECK ( json_r );
    CHECK ( str_eq (json, json_r) );
    if (json_r)
        free (json_r);
}

TEST_CASE ("bios db measurement read request encoded & decoded", "[agents][public_api]") {
    int64_t start_ts = 1428928778;
    int64_t end_ts = 14287322211;
    uint64_t element_id = 412;
    const char *source = "temperature.thermal_zone0";
    char *subject = NULL;
   
    ymsg_t *msg = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &subject);
    CHECK ( msg );
    CHECK ( subject );
    CHECK ( str_eq (subject, "get_measurements") );
    if (subject)
        free (subject);

    int64_t start_ts_r = -1;
    int64_t end_ts_r = -1;
    uint64_t element_id_r = 0;
    char *source_r = NULL;

    int rv = bios_db_measurements_read_request_extract (msg, &start_ts_r, &end_ts_r, &element_id_r, &source_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK (rv == 0);
    
    CHECK ( source_r );

    CHECK ( start_ts == start_ts_r );
    CHECK ( end_ts == end_ts_r );
    CHECK ( element_id == element_id_r );
    CHECK ( str_eq (source, source_r) );
    if (source_r)
        free (source_r);
}

TEST_CASE ("bios db measurement read reply encoded & decoded", "[agents][public_api]") {
    const char *json = "{ \"key\" : \"value\", \"key2\" : [1, 2, 3, 4]}";
    ymsg_t *msg = bios_db_measurements_read_reply_encode (json);
    CHECK ( msg );

    char *json_r = NULL;
    int rv = bios_db_measurements_read_reply_extract (msg, &json_r);
    CHECK ( msg );
    ymsg_destroy (&msg);
    CHECK ( rv == 0 );
    CHECK ( json_r );
    CHECK ( str_eq (json, json_r) );
    if (json_r)
        free (json_r);
}

TEST_CASE ("bios alert message encoded & decoded", "[agents][public_api]") {

    ymsg_t *msg = bios_alert_encode(
        "testrule",
        ALERT_PRIORITY_P2,
        ALERT_STATE_ONGOING_ALERT,
        "myDev",
        "some text",
        42);
    REQUIRE ( msg );

    char *rule = NULL, *devices = NULL, *description = NULL;
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
    ymsg_destroy( &msg );
    if(rule) free (rule);
    if(devices) free (devices);
    if(description) free (description);
}

TEST_CASE ("bios alsset message encoded & decoded", "[agents][public_api]") {

    ymsg_t *msg = bios_asset_encode(
        "device1",
        1,
        2,
        "ok",
        3);

    REQUIRE ( msg );

    char *device = NULL, *status = NULL;
    uint32_t type_id = 0, parent_id = 0, priority = 0;

    int x = bios_asset_extract( msg, &device, &type_id, &parent_id, &status, &priority );
    REQUIRE( msg );
    REQUIRE ( x == 0 );
    CHECK ( str_eq( device, "device1") );
    CHECK ( type_id == 1 );
    CHECK ( parent_id == 2 );
    CHECK ( str_eq( status, "ok" ) );
    CHECK ( priority == 3 );
    ymsg_destroy( &msg );
    FREE0( device );
    FREE0( status );
}

