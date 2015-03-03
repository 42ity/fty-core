#include <catch.hpp>

#include "bios_agent.h"

TEST_CASE("Aux setters and getters work","[api][ymsg]") {
    ymsg_t *msg = ymsg_new(YMSG_SEND);
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
