#include <catch.hpp>
#include <cxxtools/csvdeserializer.h>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

#include "csv.h"
using namespace shared;

TEST_CASE("CSV map basic get test", "[csv]") {

    std::stringstream buf;

    buf << "Name, Type, Group.1, group.2,description\n";
    buf << "RACK-01,rack,GR-01,GR-02,\"just,my,dc\"\n";
    buf << "RACK-02,rack,GR-01,GR-02,\"just\tmy\nrack\"\n";

    std::vector<std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(buf);
    deserializer.delimiter(',');
    deserializer.readTitle(false);
    deserializer.deserialize(data);

    shared::CsvMap cm{data};
    cm.deserialize();
    
    // an access to headers
    REQUIRE(cm.get(0, "Name") == "Name");
    REQUIRE(cm.get(0, "name") == "Name");
    REQUIRE(cm.get(0, "nAMe") == "Name");

    // an access to data
    REQUIRE(cm.get(1, "Name") == "RACK-01");
    REQUIRE(cm.get(2, "Name") == "RACK-02");
    
    // an access to data
    REQUIRE(cm.get(1, "Type") == "rack");
    REQUIRE(cm.get(2, "tYpe") == "rack");

    // out of bound access
    REQUIRE_THROWS_AS(cm.get(42, ""), std::out_of_range);
    // unknown key
    REQUIRE_THROWS_AS(cm.get(0, ""), std::out_of_range);

    // test values with commas
    REQUIRE(cm.get(1, "description") == "just,my,dc");
    REQUIRE(cm.get(2, "description") == "just\tmy\nrack");
}

TEST_CASE("CSV multiple field names", "[csv]") {

    std::stringstream buf;

    buf << "Name, name\n";

    std::vector<std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(buf);
    deserializer.delimiter(',');
    deserializer.readTitle(false);
    deserializer.deserialize(data);

    shared::CsvMap cm{data};
    REQUIRE_THROWS_AS(cm.deserialize(), std::invalid_argument);
    
}

inline std::string to_utf8(const cxxtools::String& ws) {
    return cxxtools::Utf8Codec::encode(ws);
}

/*
 * This tests output from MS Excel, type Unicode Text, which is UTF-16 LE with BOM
 * As we support utf-8 only, file has been converted using iconv
 * iconv -f UTF-16LE -t UTF-8 INPUT > OUTPUT
 *
 * The file still have BOM, but at least unix end of lines, which is close to
 * expected usage, where iconv will be involved!
 *
 */
TEST_CASE("CSV utf-8 input", "[csv]") {

    std::string path{__FILE__};
    path += ".csv";

    std::fstream buf{path};
    skip_utf8_BOM(buf);

    std::vector<std::vector<cxxtools::String> > data;

    cxxtools::CsvDeserializer deserializer(buf);
    deserializer.delimiter('\t');
    deserializer.readTitle(false);
    deserializer.deserialize(data);

    shared::CsvMap cm{data};
    REQUIRE_NOTHROW(cm.deserialize());

    REQUIRE(cm.get(0, "field") == "Field");
    REQUIRE(cm.get(0, "Ananotherone") == "An another one");

    REQUIRE(cm.get(1, "field") == to_utf8(cxxtools::String(L"тест")));
    REQUIRE(cm.get(2, "field") == to_utf8(cxxtools::String(L"测试")));
    REQUIRE(cm.get(3, "field") == to_utf8(cxxtools::String(L"Test")));

}
