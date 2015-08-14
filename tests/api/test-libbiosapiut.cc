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

/*! \file   test-libbiosapiut.cc
    \brief  Unit test for libbiosapi functionality during "make distcheck"
    \author Michal Hrusecky <MichalHrusecky@Eaton.com>
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
            strcmp (streams[i], bios_get_stream_alerts ()) == 0));
    } 
}


