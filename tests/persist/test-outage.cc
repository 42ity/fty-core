#include <catch.hpp>
#include "db/calculation.h"
#include "log.h"
#include "dbpath.h"
#include "measurements.h"

TEST_CASE("f:select_alertDates_byRuleName_byInterval_byDcId #1","[db][outage][select][test_outage.sql]")
{
    log_open ();
    log_info ("=============== OUTAGE #1 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const std::string rule_name = "upsonbattery%";
    int64_t start_date = 1433808000; // UNIX_TIMESTAMP('2015-06-09 00:00:00')
    int64_t end_date = 1433894400;   // UNIX_TIMESTAMP('2015-06-10 00:00:00')
    a_elmnt_id_t dc_id = 1;
    std::vector <int64_t> start{};
    std::vector <int64_t> end{};

    auto reply_select = persist::select_alertDates_byRuleName_byInterval_byDcId (conn, rule_name, start_date, end_date, dc_id, start, end);
    REQUIRE ( reply_select.rv == 0 );
    REQUIRE ( start.size() == 10 );
    REQUIRE ( end.size() == 10 );

    log_close();
}


TEST_CASE("f:calculate_outage_byInerval_byDcId #2","[db][calculate][outage][test_outage.sql]")
{
    log_open ();
    log_info ("=============== OUTAGE #2 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
 
    int64_t start_date = 1433808000; // UNIX_TIMESTAMP('2015-06-09 00:00:00')
    int64_t end_date = 1433894400;   // UNIX_TIMESTAMP('2015-06-10 00:00:00')
    a_elmnt_id_t dc_id = 1;

    int64_t outage = 0;
    int rv = persist::calculate_outage_byInerval_byDcId (conn, start_date, end_date, dc_id, outage);

    REQUIRE ( rv == 0 );
    REQUIRE ( outage == 60*60*24 );

    log_close();
}


TEST_CASE("f:calculate_outage_byInerval_byDcId #3","[db][calculate][outage][test_outage.sql]")
{
    log_open ();
    log_info ("=============== OUTAGE #3 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
 
    int64_t start_date = 1436400000; // UNIX_TIMESTAMP('2015-07-09 00:00:00')
    int64_t end_date = 1436486400;   // UNIX_TIMESTAMP('2015-07-10 00:00:00')
    a_elmnt_id_t dc_id = 1;

    int64_t outage = 0;
    int rv = persist::calculate_outage_byInerval_byDcId (conn, start_date, end_date, dc_id, outage);

    REQUIRE ( rv == 0 );
    REQUIRE ( outage == 8361 );

    log_close();
}


TEST_CASE("f:insert_outage  select_outage_byDC_byInterval #4","[db][insert][select][outage][test_outage.sql][4]")
{
    log_open ();
    log_info ("=============== OUTAGE #4 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
 
    int64_t date = 19400000;
    const char *dc_name = "DC-outage";
    a_elmnt_id_t dc_id = 1;

    int64_t value = 12345;
    reply_t r = persist::insert_outage (conn, dc_name, value, date);

    REQUIRE ( r.rv == 0 );
    REQUIRE ( r.row_id > 0 );
    uint64_t rowid = r.row_id;
    REQUIRE ( r.affected_rows == 1 );

    std::map <int64_t, double> measurements;
    r =  persist::select_outage_byDC_byInterval (conn, dc_id, 19400000 -24*60*60 , 19400000 + 24*60*60, measurements);
    REQUIRE ( r.rv == 0 );
    REQUIRE ( measurements.size() == 1 );

    db_reply_t ret = persist::delete_from_measurement_by_id (conn, rowid);

    REQUIRE ( ret.status == 1 );
    REQUIRE ( ret.affected_rows == 1 );
    log_close();
}
