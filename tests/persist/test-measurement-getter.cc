#include <catch.hpp>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <sstream>
#include <vector>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/jsondeserializer.h>

#include "measurement.h"
#include "dbpath.h"
#include "log.h"
#include "bios_agent.h"
#include "persistencelogic.h"

#include <errno.h>

using namespace persist;

TEST_CASE("measurement_getter", "[db][select][t_bios_measurement][t_bios_measurement_topic][measurement_getter]")
{
    log_open();
    ymsg_t *in = ymsg_new(YMSG_SEND);
    ymsg_t *out = ymsg_new(YMSG_REPLY);
    char *out_s = NULL;

    errno = 0;
    ymsg_set_int64(in, "element_id", 26);
    ymsg_set_int64(in, "time_st", 0);
    ymsg_set_int64(in, "time_end", time(NULL));
    ymsg_set_string(in, "source", "temperature.thermal_zone0.avg_8");
    REQUIRE(errno == 0);

    persist::process_ymsg(out, &out_s, in, "get_measurements");

    REQUIRE(streq(out_s, "return_measurements"));

    zchunk_t *ch = ymsg_get_response(out);
    std::stringstream str;
    str << ((char*)zchunk_data(ch));
    cxxtools::SerializationInfo si;
    cxxtools::JsonDeserializer ds(str);
    ds.deserialize(si);

    std::string test;
    REQUIRE(si.getMember("units", test));
    REQUIRE(test == "C");
    REQUIRE(si.getMember("source", test));
    REQUIRE(test == "temperature.thermal_zone0.avg_8");
    REQUIRE(si.getMember("data").memberCount() > 2);

    log_close();
}
