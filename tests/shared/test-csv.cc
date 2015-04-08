#include <catch.hpp>
#include <cxxtools/csvdeserializer.h>

#include <sstream>
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
/*
TEST_CASE("CSV multiple field names", "[csv]") {

    
    std::stringstream buf;

    buf << "Name, Type, Group.1, group.2,description\n";
    buf << "RACK-01,rack,GR-01,GR-02,\"just,my,dc\"\n";
    buf << "RACK-02,rack,GR-011,GR-02,\"just\tmy\nrack\"\n";

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

    load_asset_csv(buf);
    
}*/
