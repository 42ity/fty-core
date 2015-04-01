#include <catch.hpp>
#include <tntdb/connection.h>
#include <tntdb/connect.h>

#include <vector>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/jsondeserializer.h>
#include <errno.h>

#include "measurement.h"
#include "dbpath.h"
#include "log.h"
#include "bios_agent.h"
#include "persistencelogic.h"

using namespace persist;

TEST_CASE("measurement INSERT/SELECT/DELETE #1", "[db][CRUD][insert][delete][select][t_bios_measurement][t_bios_measurement_topic]")
{
    log_open();

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // it is supposed that topic has a predefined structure
    // in  transport layer it can be longer than in database,
    // because in database topic for measurements is supposed to be
    // name_of_the_measurement@device_name
    const char* topic1 = "this.is.complex.topic@DUMMY_DEVICE";
    const char* topic2 = "that.is.more.complex.topic@DUMMY_DEVICE";
    const char* unit1 = "unit1";
    const char* unit2 = "unit2";
    const char* query1 = "%this%";
    const char* device_name = "DUMMY_DEVICE";
    //1.) assert empty database
    auto ret = select_from_measurement_by_topic(conn, "%DUMMY_DEVICE%");
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 0);

    //2.) insert a few entries
    auto ret1 = insert_into_measurement(conn,
            topic1, 42, 1, 0, unit1, device_name);
    REQUIRE(ret1.status == 1);
    REQUIRE(ret1.affected_rows == 1);
    REQUIRE(ret1.rowid != 0);
    
    
    auto ret2 = insert_into_measurement(conn,
            topic1, 142, 1, 0, unit2, device_name);
    REQUIRE(ret2.status == 1);
    REQUIRE(ret2.affected_rows == 1);
    REQUIRE(ret2.rowid == ret1.rowid + 1);
    
    auto ret3 = insert_into_measurement(conn,
            topic2, 999, -1, 0, unit2,device_name);
    REQUIRE(ret3.status == 1);
    REQUIRE(ret3.affected_rows == 1);
    REQUIRE(ret3.rowid == ret2.rowid + 1);

    // 3.) SELECT
    ret = select_from_measurement_by_topic(conn, query1);
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 2);
    auto i = 1u;
    for (const auto it : ret.item) {
        bool condition_to_check = (it.id == ret1.rowid)  || (it.id == ret2.rowid);
        REQUIRE(condition_to_check);
        REQUIRE(it.topic == topic1);
        i++;
    }

    // 4.) SELECT all
    ret = select_from_measurement_by_topic(conn, "%DUMMY_DEVICE%");
    REQUIRE(ret.status == 1);
    REQUIRE(ret.item.size() == 3);

    // 5.) DELETE NON EXISTING
    ret2 = delete_from_measurement_by_id(conn, 42);
    REQUIRE(ret2.status == 1);
    REQUIRE(ret2.affected_rows == 0);
    
    // 5.) DELETE EXISTING
    for (i = ret1.rowid; i <=ret3.rowid; i++) {
        ret2 = delete_from_measurement_by_id(conn, i);
        REQUIRE(ret2.status == 1);
        REQUIRE(ret2.affected_rows == 1);
    }

    log_close();
}

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
