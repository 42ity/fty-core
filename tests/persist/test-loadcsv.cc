#include <catch.hpp>
#include <cxxtools/csvdeserializer.h>
#include <tntdb/result.h>
#include <tntdb/statement.h>
#include "csv.h"
#include "log.h"
#include "loadcsv.h"
#include "assetcrud.h"
#include "dbpath.h"
#include <fstream>    

std::string get_dc_lab_description() {
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    tntdb::Statement st = conn.prepareCached(
        " SELECT value from t_bios_asset_ext_attributes"
        " WHERE id_asset_element in ("
        "   SELECT id_asset_element FROM t_bios_asset_element"
        "   WHERE NAME = 'DC-LAB')"
        " AND keytag='description'"
    );

    // this is ugly, but was lazy to read the docs
    tntdb::Result result = st.select();
    for ( auto &row: result )
    {
        std::string ret;
        row[0].get(ret);
        return ret;
    }

    return std::string{"could-not-get-here"};  //to make gcc happpy
}

inline std::string to_utf8(const cxxtools::String& ws) {
    return cxxtools::Utf8Codec::encode(ws);
}

TEST_CASE("CSV multiple field names", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".csv";
    std::string tsv = base_path + ".tsv";
    std::string ssv = base_path + ".ssv";
    std::string usv = base_path + ".usv";
    static std::string exp = to_utf8(cxxtools::String(L"Lab DC(тест)"));
    REQUIRE_NOTHROW(get_dc_lab_description() == exp);

    // test coma separated values format
    std::fstream csv_buf{csv};
    REQUIRE_NOTHROW(load_asset_csv(csv_buf));

    // test tab separated values
    std::fstream tsv_buf{tsv};
    REQUIRE_NOTHROW(load_asset_csv(tsv_buf));
    
    // test semicolon separated values
    std::fstream ssv_buf{ssv};
    REQUIRE_NOTHROW(load_asset_csv(ssv_buf));

    // test underscore separated values
    std::fstream usv_buf{usv};
    REQUIRE_THROWS_AS (load_asset_csv(usv_buf), std::invalid_argument);
}

TEST_CASE("CSV bug 661 - segfault with quote in name", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".661.csv";

    std::fstream csv_buf{csv};
    REQUIRE_THROWS_AS(load_asset_csv(csv_buf), std::out_of_range);
}

TEST_CASE("CSV bug 661 - segfault on csv without mandatory columns", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".661.2.csv";

    std::fstream csv_buf{csv};
    REQUIRE_THROWS_AS(load_asset_csv(csv_buf), std::invalid_argument);
}
