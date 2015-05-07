#include <catch.hpp>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <tntdb/value.h>

#include <vector>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/jsondeserializer.h>
#include <errno.h>

#include "measurement.h"
#include "dbpath.h"
#include "log.h"
#include "bios_agent.h"
#include "persistencelogic.h"
#include "utils.h"
#include "cleanup.h"

using namespace persist;

TEST_CASE("measurement INSERT/SELECT/DELETE #1", "[db][CRUD][insert][delete][select][t_bios_measurement][t_bios_measurement_topic]")
{

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
    // First find highest id
    uint64_t highest_measurements_id = 0;
    conn.selectValue("select id from t_bios_measurement ORDER BY id DESC LIMIT 1").get (highest_measurements_id); 
    CHECK (highest_measurements_id != 0 );  
        
    ret2 = delete_from_measurement_by_id(conn, highest_measurements_id + 1);
    REQUIRE(ret2.status == 1);
    REQUIRE(ret2.affected_rows == 0);
    
    // 5.) DELETE EXISTING
    for (i = ret1.rowid; i <=ret3.rowid; i++) {
        ret2 = delete_from_measurement_by_id(conn, i);
        REQUIRE(ret2.status == 1);
        REQUIRE(ret2.affected_rows == 1);
    }

}

TEST_CASE("measurement_getter", "[db][select][t_bios_measurement][t_bios_measurement_topic][measurement_getter]")
{
    
    INFO ("measurement_getter start\n");

    _scoped_ymsg_t *in = ymsg_new(YMSG_SEND);
    _scoped_ymsg_t *out = ymsg_new(YMSG_REPLY);
    _scoped_char *out_s = NULL;
    REQUIRE ( in != NULL);
    REQUIRE ( out != NULL);

    errno = 0;
    ymsg_set_int64(in, "element_id", 26);
    ymsg_set_int64(in, "start_ts", 0);
    ymsg_set_int64(in, "end_ts", time(NULL));
    ymsg_set_string(in, "source", "temperature.thermal_zone0");
    ymsg_set_string(in, "step", "8h");
    ymsg_set_string(in, "type", "arithmetic_mean");
    REQUIRE(errno == 0);

    persist::process_ymsg(out, &out_s, in, "get_measurements");

    CHECK ( out_s != NULL );
    CHECK ( str_eq (out_s, "return_measurements") );    

    _scoped_zchunk_t *ch = ymsg_get_response(out);
    REQUIRE ( ch != NULL);

    std::string json ((char *) zchunk_data (ch), zchunk_size (ch));
    std::stringstream str (json, std::ios_base::in);

    cxxtools::SerializationInfo si;
    cxxtools::JsonDeserializer ds(str);
    ds.deserialize(si);

    std::string test;
    REQUIRE(si.getMember("unit", test));
    REQUIRE(test == "C");
    REQUIRE(si.getMember("data").memberCount() > 2);

    
}
