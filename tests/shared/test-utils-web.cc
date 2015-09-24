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
 * \file test-utils-web.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <czmq.h>
#include <string>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/jsondeserializer.h>
#include <limits.h>

#include "utils_web.h"

TEST_CASE ("utils::json::escape","[utils::math::dtos][json][escape]")
{

    // utils::json::escape (<first>) should equal <second>
    std::vector <std::pair <std::string, std::string>> tests {
        {"'jednoduche ' uvozovky'",                                     "'jednoduche ' uvozovky'"},
        {"'jednoduche '' uvozovky'",                                    "'jednoduche '' uvozovky'"},
        {"dvojite \" uvozovky",                                         R"(dvojite \" uvozovky)"},
        {"dvojite \\\" uvozovky",                                       R"(dvojite \\\" uvozovky)"},
        {"dvojite \\\\\" uvozovky",                                     R"(dvojite \\\\\" uvozovky)"},
        {"dvojite \\\\\\\" uvozovky",                                   R"(dvojite \\\\\\\" uvozovky)"},
        {"dvojite \\\\\\\\\" uvozovky",                                 R"(dvojite \\\\\\\\\" uvozovky)"},
        {"'",                                                           R"(')"},
        {"\"",                                                          R"(\")"},
        {"'\"",                                                         R"('\")"},
        {"\"\"",                                                        R"(\"\")"},
        {"\"\"\"",                                                      R"(\"\"\")"},
        {"\\\"\"\"",                                                    R"(\\\"\"\")"},
        {"\"\\\"\"",                                                    R"(\"\\\"\")"},
        {"\"\"\\\"",                                                    R"(\"\"\\\")"},
        {"\\\"\\\"\\\"",                                                R"(\\\"\\\"\\\")"},
        {"\" dvojite \\\\\" uvozovky \"",                               R"(\" dvojite \\\\\" uvozovky \")"},
        {"dvojite \"\\\"\" uvozovky",                                   R"(dvojite \"\\\"\" uvozovky)"},
        {"dvojite \\\\\"\\\\\"\\\\\" uvozovky",                         R"(dvojite \\\\\"\\\\\"\\\\\" uvozovky)"},

        {"\b",                                                          R"(\b)"},
        {"\\b",                                                         R"(\\b)"},
        {"\\\b",                                                        R"(\\\b)"},
        {"\\\\b",                                                       R"(\\\\b)"},
        {"\\\\\b",                                                      R"(\\\\\b)"},
        {"\\\\\\b",                                                     R"(\\\\\\b)"},
        {"\\\\\\\b",                                                    R"(\\\\\\\b)"},
        {"\\\\\\\\b",                                                   R"(\\\\\\\\b)"},
        {"\\\\\\\\\b",                                                  R"(\\\\\\\\\b)"},
        
        {"\f",                                                          R"(\f)"},
        {"\\f",                                                         R"(\\f)"},
        {"\\\f",                                                        R"(\\\f)"},
        {"\\\\f",                                                       R"(\\\\f)"},
        {"\\\\\f",                                                      R"(\\\\\f)"},
        {"\\\\\\f",                                                     R"(\\\\\\f)"},
        {"\\\\\\\f",                                                    R"(\\\\\\\f)"},
        {"\\\\\\\\f",                                                   R"(\\\\\\\\f)"},
        {"\\\\\\\\\f",                                                  R"(\\\\\\\\\f)"},

        {"\n",                                                          R"(\n)"},
        {"\\n",                                                         R"(\\n)"},
        {"\\\n",                                                        R"(\\\n)"},
        {"\\\\n",                                                       R"(\\\\n)"},
        {"\\\\\n",                                                      R"(\\\\\n)"},
        {"\\\\\\n",                                                     R"(\\\\\\n)"},
        {"\\\\\\\n",                                                    R"(\\\\\\\n)"},
        {"\\\\\\\\n",                                                   R"(\\\\\\\\n)"},
        {"\\\\\\\\\n",                                                  R"(\\\\\\\\\n)"},

        {"\r",                                                          R"(\r)"},
        {"\\r",                                                         R"(\\r)"},
        {"\\\r",                                                        R"(\\\r)"},
        {"\\\\r",                                                       R"(\\\\r)"},
        {"\\\\\r",                                                      R"(\\\\\r)"},
        {"\\\\\\r",                                                     R"(\\\\\\r)"},
        {"\\\\\\\r",                                                    R"(\\\\\\\r)"},
        {"\\\\\\\\r",                                                   R"(\\\\\\\\r)"},
        {"\\\\\\\\\r",                                                  R"(\\\\\\\\\r)"},

        {"\t",                                                          R"(\t)"},
        {"\\t",                                                         R"(\\t)"},
        {"\\\t",                                                        R"(\\\t)"},
        {"\\\\t",                                                       R"(\\\\t)"},
        {"\\\\\t",                                                      R"(\\\\\t)"},
        {"\\\\\\t",                                                     R"(\\\\\\t)"},
        {"\\\\\\\t",                                                    R"(\\\\\\\t)"},
        {"\\\\\\\\t",                                                   R"(\\\\\\\\t)"},
        {"\\\\\\\\\t",                                                  R"(\\\\\\\\\t)"},

        {"\\",                                                          R"(\\)"},
        {"\\\\",                                                        R"(\\\\)"},
        {"\\\\\\",                                                      R"(\\\\\\)"},
        {"\\\\\\\\",                                                    R"(\\\\\\\\)"},
        {"\\\\\\\\\\",                                                  R"(\\\\\\\\\\)"},

        {"\uA66A",                                                      R"(\u00ea\u0099\u00aa)"},
        {"Ꙫ",                                                           R"(\u00ea\u0099\u00aa)"},
        {"\uA66A Ꙫ",                                                    R"(\u00ea\u0099\u00aa \u00ea\u0099\u00aa)"},
        {"\\uA66A",                                                     R"(\\uA66A)"},
        {"\\Ꙫ",                                                         R"(\\\u00ea\u0099\u00aa)"},
        {"\u040A Њ",                                                    R"(\u00d0\u008a \u00d0\u008a)"},
        {"\u0002\u0005\u0018\u001B",                                    R"(\u0002\u0005\u0018\u001b)"},

        {"\\\uA66A",                                                    R"(\\\u00ea\u0099\u00aa)"},
        {"\\\\uA66A",                                                   R"(\\\\uA66A)"},
        {"\\\\\uA66A",                                                  R"(\\\\\u00ea\u0099\u00aa)"},
        {"\\\\\\uA66A",                                                 R"(\\\\\\uA66A)"},
        {"\\\\\\\uA66A",                                                R"(\\\\\\\u00ea\u0099\u00aa)"},

        {"\\\\Ꙫ",                                                       R"(\\\\\u00ea\u0099\u00aa)"},
        {"\\\\\\Ꙫ",                                                     R"(\\\\\\\u00ea\u0099\u00aa)"},
        {"\\\\\\\\Ꙫ",                                                   R"(\\\\\\\\\u00ea\u0099\u00aa)"},
        {"\\\\\\\\\\Ꙫ",                                                 R"(\\\\\\\\\\\u00ea\u0099\u00aa)"},

        {"first second \n third\n\n \\n \\\\\n fourth",                 R"(first second \n third\n\n \\n \\\\\n fourth)"},
        {"first second \n third\n\"\n \\n \\\\\"\f\\\t\\u\u0007\\\n fourth", R"(first second \n third\n\"\n \\n \\\\\"\f\\\t\\u\u0007\\\n fourth)"},
    };

    // a valid json { key : utils::json::escape (<string> } is constructed,
    // fed into cxxtools::JsonDeserializer (), read back and compared 
    std::vector <std::string> tests_reading{
        {"newline in \n text \n\"\n times two"},
        {"x\tx"},
        {"x\\tx"},
        {"x\\\tx"},
        {"x\\\\tx"},
        {"x\\\\\tx"},
        {"x\\\\\\tx"},
        {"x\\\\\\\tx"},
        {"x\\Ꙫ\uA66A\n \\nx"},
        {"sdf\ndfg\n\\\n\\\\\n\b\tasd \b f\\bdfg"},
        {"first second \n third\n\"\n \\n \\\\\"\f\\\t\\u\u0007\\\n fourth"}
    };

    SECTION ("Manual comparison.") {
        std::string escaped;
        for (auto const& item : tests) {
            escaped = utils::json::escape (item.first);
            CAPTURE (escaped);
            CHECK ( escaped.compare (item.second) == 0);
        }
    }

    SECTION ("Validate whether the escaped string is a valid json using cxxtools::JsonDeserializer.") {
        for (auto const& item : tests) {
            std::string json;
            std::string escaped = utils::json::escape (item.first);

            json.assign("{ \"string\" : ").append ("\"").append (escaped).append ("\" }");

            std::stringstream input (json, std::ios_base::in);
            cxxtools::JsonDeserializer deserializer (input);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW (deserializer.deserialize (si));
        }
    }

    SECTION ("Construct json, read back, compare.") {
        for (auto const& it : tests_reading) {
            std::string json;
            json.assign("{ \"read\" : ").append ("\"").append (utils::json::escape (it)).append ("\" }");

            std::stringstream input (json, std::ios_base::in);
            cxxtools::JsonDeserializer deserializer (input);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW (deserializer.deserialize (si));
            std::string read;
            si.getMember ("read") >>= read;
            CHECK ( read.compare (it) == 0 );
        }   
    }
}

TEST_CASE ("utils::json::jsonify","[utils::json::make][json][escape]")
{
    // 
    int var_int = -1;
    long int var_long_int = -2;
    long long int var_long_long_int = -3;
    short var_short = 4;
    int32_t var_int32_t = 6;
    int64_t var_int64_t = 7;
    byte var_byte = 8;

    unsigned int var_unsigned_int = 10;
    unsigned long int var_unsigned_long_int = 20;
    unsigned long long int var_unsigned_long_long_int = 30;
    unsigned short var_unsigned_short = 40;
    uint32_t var_uint32_t = 50;
    uint64_t var_uint64_t = 60;

    //
    const char *const_char = "*const char with a '\"' quote and newline \n '\\\"'";
    std::string str = const_char;
    std::string& str_ref = str;
    std::string* str_ptr = &str;

    std::string x; // temporary result placeholder
    
    SECTION ("single parameter ('inttype') invocation") {
        x = utils::json::jsonify (var_int);
        CHECK ( x.compare (std::to_string (var_int)) == 0);

        x = utils::json::jsonify (var_long_int);
        CHECK ( x.compare (std::to_string (var_long_int)) == 0);

        x = utils::json::jsonify (var_long_long_int);
        CHECK ( x.compare (std::to_string (var_long_long_int)) == 0);

        x = utils::json::jsonify (var_short);
        CHECK ( x.compare (std::to_string (var_short)) == 0);

        x = utils::json::jsonify (var_int32_t);
        CHECK ( x.compare (std::to_string (var_int32_t)) == 0);

        x = utils::json::jsonify (var_int64_t);
        CHECK ( x.compare (std::to_string (var_int64_t)) == 0);

        x = utils::json::jsonify (var_byte);
        CHECK ( x.compare (std::to_string (var_byte)) == 0);

        x = utils::json::jsonify (var_unsigned_int);
        CHECK ( x.compare (std::to_string (var_unsigned_int)) == 0);

        x = utils::json::jsonify (var_unsigned_long_int);
        CHECK ( x.compare (std::to_string (var_unsigned_long_int)) == 0);

        x = utils::json::jsonify (var_unsigned_long_long_int);
        CHECK ( x.compare (std::to_string (var_unsigned_long_long_int)) == 0);

        x = utils::json::jsonify (var_unsigned_short);
        CHECK ( x.compare (std::to_string (var_unsigned_short)) == 0);

        x = utils::json::jsonify (var_uint32_t);
        CHECK ( x.compare (std::to_string (var_uint32_t)) == 0);

        x = utils::json::jsonify (var_uint64_t);
        CHECK ( x.compare (std::to_string (var_uint64_t)) == 0);

    }

    SECTION ("single parameter ('string') invocation") {

        x = utils::json::jsonify (const_char);
        CHECK ( x.compare (R"("*const char with a '\"' quote and newline \n '\\\"'")") == 0);

        x = utils::json::jsonify (str);
        CHECK ( x.compare (R"("*const char with a '\"' quote and newline \n '\\\"'")") == 0);

        x = utils::json::jsonify (str_ref);

        x = utils::json::jsonify (*str_ptr);
        CHECK ( x.compare (R"("*const char with a '\"' quote and newline \n '\\\"'")") == 0);

    }

    SECTION ("pairs") {
        x = utils::json::jsonify (*str_ptr, var_int64_t);
        CHECK ( x.compare (std::string(R"("*const char with a '\"' quote and newline \n '\\\"'" : )") + std::to_string (var_int64_t)) == 0);

        
        x = utils::json::jsonify ("hey\"!\n", str_ref);
        CHECK ( x.compare (R"("hey\"!\n" : "*const char with a '\"' quote and newline \n '\\\"'")") == 0);

        x = utils::json::jsonify (-6, -7);
        CHECK ( x.compare (R"("-6" : -7)") == 0 ); 

        x = utils::json::jsonify (var_uint64_t, str);
        CHECK ( x.compare (std::string ("\"") + std::to_string (var_uint64_t) + "\" : " + R"("*const char with a '\"' quote and newline \n '\\\"'")") == 0 ); 
    }
}
