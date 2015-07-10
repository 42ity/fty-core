/* Description: This program is unit-testing the $BIOS project's libbiosapi
 * functionality during "make distcheck" (compiled via the Makefiles).
 *
 * Author(s): Michal Hrusecky <MichalHrusecky@eaton.com>
 */
#include <catch.hpp>
#include <string.h>

#include "cleanup.h"
#include "bios_agent.h"

TEST_CASE("Aux setters and getters work","[api][ymsg]") {
    _scoped_ymsg_t *msg = ymsg_new(YMSG_SEND);
    ymsg_set_string(msg, "string",  "test_string");
    ymsg_set_int32(msg, "int32", 256);
    ymsg_set_int64(msg, "int64", 512);
    REQUIRE(strcmp("test_string",  ymsg_get_string(msg, "string" )) == 0);
    REQUIRE(strcmp("256", ymsg_get_string(msg, "int32")) == 0);
    REQUIRE(strcmp("512", ymsg_get_string(msg, "int64")) == 0);
    REQUIRE(256 == ymsg_get_int32(msg, "int32"));
    REQUIRE(512 == ymsg_get_int64(msg, "int64"));
    ymsg_destroy(&msg);
}

TEST_CASE ("stream names", "[api][bios_agent]") {
    int count = 0;
    const char **streams = bios_get_streams ((uint8_t *)&count);
    REQUIRE ( streams != NULL );
    REQUIRE ( count != 0 );
/* The expected streams are defined in /include/bios_agent.h
* and currently we have 5 of them -- all must be expected here: */
    for (int i = 0; i < count; ++i) {
        CHECK ((strcmp (streams[i], bios_get_stream_main ()) == 0 ||
            strcmp (streams[i], bios_get_stream_measurements ()) == 0 ||
            strcmp (streams[i], bios_get_stream_assets ()) == 0 ||
            strcmp (streams[i], bios_get_stream_networks ()) == 0 ||
            strcmp (streams[i], bios_get_stream_alerts ()) == 0));
    } 
}


