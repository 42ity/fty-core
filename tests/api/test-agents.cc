#include <catch.hpp>
#include <stdio.h>

#include "dbpath.h"
#include "log.h"
#include "agents.h"

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
        CHECK ( bios_web_average_request_encode (0, 0, NULL, "", 0, "") == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, "", NULL, 0, "") == NULL );
        CHECK ( bios_web_average_request_encode (0, 0, "", "", 0, NULL) == NULL );
    }

    SECTION ("bios_web_average_request_decode") {
        int64_t start_ts, end_ts;
        char *type, *step, *source;
        uint64_t element_id;
        ymsg_t *msg_null = NULL;
        ymsg_t *msg = ymsg_new (YMSG_SEND);
        REQUIRE (msg);

        CHECK ( bios_web_average_request_decode (NULL, &start_ts, &end_ts, &type, &step, &element_id, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg_null, &start_ts, &end_ts, &type, &step, &element_id, &source) == -1 );

        CHECK ( bios_web_average_request_decode (&msg, NULL, &end_ts, &type, &step, &element_id, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg, &start_ts, NULL, &type, &step, &element_id, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg, &start_ts, &end_ts, NULL, &step, &element_id, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg, &start_ts, &end_ts, &type, NULL, &element_id, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg, &start_ts, &end_ts, &type, &step, NULL, &source) == -1 );
        CHECK ( bios_web_average_request_decode (&msg, &start_ts, &end_ts, &type, &step, &element_id, NULL) == -1 );

        ymsg_destroy(&msg_null);
        ymsg_destroy(&msg);
    }

}

TEST_CASE ("bios web average request encoded & decoded", "[agents][public_api]") {
    int64_t start_ts = 20150301;
    int64_t end_ts = 20150302;
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
   
    int rv = bios_web_average_request_decode (&msg, &start_ts_r, &end_ts_r, &type_r, &step_r, &element_id_r, &source_r);
    REQUIRE (rv != -1);

    CHECK ( start_ts == start_ts_r );
    CHECK ( end_ts == end_ts_r );
    CHECK ( strcmp (type, type_r) == 0 );
    CHECK ( strcmp (step, step_r) == 0 );
    CHECK ( element_id == element_id_r );
    CHECK ( strcmp (source, source_r) == 0 );

    CHECK (msg == NULL);
    free (type_r);
    free (step_r);
    free (source_r);
}

TEST_CASE ("bios web average reply encoded & decoded", "[agents][public_api]") {
    const char *json = "abrakadabra ";
    ymsg_t *msg = bios_web_average_reply_encode (json);
    REQUIRE ( msg );

    char *json_r = NULL;
    int rv = bios_web_average_reply_decode (&msg, &json_r);
    REQUIRE ( rv != -1 );
    REQUIRE ( strcmp (json, json_r) == 0 );
    free (json_r);
    REQUIRE ( msg == NULL );
}

TEST_CASE ("bios db measurement read request encoded & decoded", "[agents][public_api]") {
    const char *start_ts = "2015030112000000Z";
    const char *end_ts = "2015030213000000Z";
    uint64_t element_id = 412;
    const char *source = "temperature.default";
    char *subject = NULL;
   
    ymsg_t *msg = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &subject);
    REQUIRE ( msg );
    REQUIRE ( subject );
    free (subject);

    char *start_ts_r = NULL;
    char *end_ts_r = NULL;
    uint64_t element_id_r = 0;
    char *source_r = NULL;

    int rv = bios_db_measurements_read_request_decode (&msg, &start_ts_r, &end_ts_r, &element_id_r, &source_r);
    REQUIRE (rv != -1);

    CHECK ( strcmp (start_ts, start_ts_r) == 0 );
    CHECK ( strcmp (end_ts, end_ts_r) == 0 );
    CHECK ( element_id == element_id_r );
    CHECK ( strcmp (source, source_r) == 0 );

    CHECK (msg == NULL);
    free (start_ts_r);
    free (end_ts_r);
    free (source_r);
}

TEST_CASE ("bios db measurement read reply encoded & decoded", "[agents][public_api]") {
    const char *json = "{ \"key\" : \"value\", \"key2\" : [1, 2, 3, 4]}";
    ymsg_t *msg = bios_db_measurements_read_reply_encode (json);
    REQUIRE ( msg );

    char *json_r = NULL;
    int rv = bios_db_measurements_read_reply_decode (&msg, &json_r);
    REQUIRE ( rv != -1 );
    REQUIRE ( strcmp (json, json_r) == 0 );
    free (json_r);
    REQUIRE ( msg == NULL );
   
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
    alert_priority_t priority = ALERT_PRIORITY_UNKNOWN;
    alert_state_t state = ALERT_STATE_UNKNOWN;
    time_t time = 0;

    bios_alert_decode( &msg, NULL, &priority, &state, &devices, &description, &time);
    CHECK( msg );
    
    int x = bios_alert_decode( &msg, &rule, &priority, &state, &devices, &description, &time);
    CHECK ( x == 0 );
    CHECK ( msg == NULL );
    CHECK ( strcmp (rule, "testrule") == 0 );
    CHECK ( strcmp (devices, "myDev") == 0 );
    CHECK ( strcmp (description, "some text") == 0 );
    CHECK ( priority == ALERT_PRIORITY_P2 );
    CHECK ( state == ALERT_STATE_ONGOING_ALERT );
    CHECK ( time == 42 );
    if(rule) free (rule);
    if(devices) free (devices);
    if(description) free (description);
}

