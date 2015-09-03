/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file test-csv.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Tests for CSV module
 */
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

    auto titles = cm.getTitles();
    REQUIRE(titles.size() == 5);
    REQUIRE(titles.count("name") == 1);
    REQUIRE(titles.count("type") == 1);
    REQUIRE(titles.count("group.1") == 1);
    REQUIRE(titles.count("group.2") == 1);
    REQUIRE(titles.count("description") == 1);

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

    std::ifstream buf{path};
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

TEST_CASE("CSV findDelimiter", "[csv]")
{
    {
    std::stringstream buf;
    buf << "Some;delimiter\n";
    auto delim = findDelimiter(buf);

    REQUIRE(buf.str().size() == 15);
    REQUIRE(delim == ';');
    REQUIRE(buf.str().size() == 15);
    }

    {
    std::stringstream buf;
    buf << "None delimiter\n";
    auto delim = findDelimiter(buf);

    REQUIRE(buf.str().size() == 15);
    REQUIRE(delim == '\x0');
    REQUIRE(buf.str().size() == 15);
    }

    {
    std::stringstream buf;
    buf << "None_delimiter\n";
    auto delim = findDelimiter(buf);

    REQUIRE(buf.str().size() == 15);
    REQUIRE(delim == '\x0');
    REQUIRE(buf.str().size() == 15);
    }

}

TEST_CASE("CSV from_serialization_info", "[csv][si]")
{

    cxxtools::SerializationInfo si;

    si.setTypeName("Object");
    si.addMember("name") <<= "DC-1";
    si.addMember("type") <<= "datacenter";

    cxxtools::SerializationInfo& esi = si.addMember("ext");

    esi.setTypeName("Object");
    esi.addMember("ext1") <<= "ext1";
    esi.addMember("ext2") <<= "ext2";

    CsvMap map = CsvMap_from_serialization_info(si);
    REQUIRE(map.cols() == 4);
    REQUIRE(map.rows() == 2);

    std::vector<std::vector<std::string>> EXP = {
        {"name", "type", "ext1", "ext2"},
        {"DC-1", "datacenter", "ext1", "ext2"}
    };

    int i = 0;
    for (const auto& title : EXP[0]) {
        REQUIRE(map.hasTitle(title));
        REQUIRE(map.get(1, title) == EXP[1][i++]);
    }

}

TEST_CASE("CSV from_serialization_info bad i", "[csv][si]")
{

    cxxtools::SerializationInfo si;

    si.setTypeName("Array");

    REQUIRE_THROWS_AS(CsvMap_from_serialization_info(si), std::invalid_argument);

}
