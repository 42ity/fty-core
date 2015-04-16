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

    std::string path{__FILE__};
    path += ".csv";
    std::cout << path << std::endl;
    std::fstream buf{path};

    load_asset_csv(buf);
    
    static std::string exp = to_utf8(cxxtools::String(L"Lab DC(тест)"));

    REQUIRE_NOTHROW(get_dc_lab_description() == exp);
}
