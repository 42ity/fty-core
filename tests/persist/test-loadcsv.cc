#include <catch.hpp>
#include <cxxtools/csvdeserializer.h>
#include "csv.h"
#include "log.h"
#include "loadcsv.h"
#include "assetcrud.h"
#include <fstream>    


TEST_CASE("CSV multiple field names", "[csv]") {

    std::string path{__FILE__};
    path += ".csv";
    std::cout << path << std::endl;
    std::fstream buf{path};

    load_asset_csv(buf);

    
  /*       char *buffer = new char [100];
    buf.read (buffer, 100);
    log_debug ("test buffer %s", buffer);
//    buf << "Name, Type, Group.1, group.2,description\n";
//    buf << "RACK-01,rack,GR-01,GR-02,\"just,my,dc\"\n";
//    buf << "RACK-02,rack,GR-011,GR-02,\"just\tmy\nrack\"\n";

 
    std::vector<std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(buf);
    log_debug ("1");
    deserializer.delimiter('\t');
    deserializer.readTitle(false);
    deserializer.deserialize(data);
    
    log_debug ("11");
    shared::CsvMap cm{data};
    log_debug ("111");
    cm.deserialize();
    log_debug ("1111");
    
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
   */ 
}
