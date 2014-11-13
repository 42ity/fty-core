#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "assetmsg.h"

#include "dbpath.h"

TEST_CASE("asset_msg: select_last_measurements","[asset][select][55][lastmeasurements]")
{
    zlist_t* measurements = select_last_measurements (url.c_str(), 1);
    printf ("%s", (char*) zlist_first (measurements));
}
