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
 * \file test-utils++.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <string>
#include <limits.h>

#include "utils++.h"

TEST_CASE ("utils::math::utils::math::dtos","[utils::math::dtos][utilities]")
{
    std::string result;
    CHECK_NOTHROW ( utils::math::dtos (1.44, 2, result) );
    CHECK ( result == "1.44" );

    CHECK_NOTHROW ( utils::math::dtos (1.45, 2, result) );
    CHECK ( result == "1.45" );

    CHECK_NOTHROW ( utils::math::dtos (-1.44, 2, result) );
    CHECK ( result == "-1.44" );

    CHECK_NOTHROW ( utils::math::dtos (-1.45, 2, result) );
    CHECK ( result == "-1.45" );

    CHECK_NOTHROW ( utils::math::dtos (1.449, 2, result) );
    CHECK ( result == "1.45" );

    CHECK_NOTHROW ( utils::math::dtos (1.446, 2, result) );
    CHECK ( result == "1.45" );

    CHECK_NOTHROW ( utils::math::dtos (1.445, 2, result) );
    CHECK ( result == "1.45" );

    CHECK_NOTHROW ( utils::math::dtos (1.444, 2, result) );
    CHECK ( result == "1.44" );

    CHECK_NOTHROW ( utils::math::dtos (1.4401, 2, result) );
    CHECK ( result == "1.44" );

    CHECK_NOTHROW ( utils::math::dtos (1.4, 2, result) );
    CHECK ( result == "1.40" );

    CHECK_NOTHROW ( utils::math::dtos (1.4449, 2, result) );
    CHECK ( result == "1.44" );

    CHECK_NOTHROW ( utils::math::dtos (-1.444, 2, result) );
    CHECK ( result == "-1.44" );

    CHECK_NOTHROW ( utils::math::dtos (-1.4401, 2, result) );
    CHECK ( result == "-1.44" );

    CHECK_NOTHROW ( utils::math::dtos (-1.4, 2, result) );
    CHECK ( result == "-1.40" );

    CHECK_NOTHROW ( utils::math::dtos (-1.4449, 2, result) );
    CHECK ( result == "-1.44" );
}



TEST_CASE ("escape", "[utilities]")
{

    static std::vector<std::pair<std::string, std::string >> vec = {
        {"", ""},
        {"a", "a"},
        {R"(\)", R"(\)"},
        {R"(a\)", R"(a\)"},
        {"a_", R"(a\_)"},
        {"_", R"(\_)"},
        {"foo", "foo"},
        {"ba_r", R"(ba\_r)"},
        {R"(ba\_r)", R"(ba\_r)"},
        {"h%m", R"(h\%m)"},
        {R"(h\%m)", R"(h\%m)"},
        {R"(foo\_bar_baz\%ham%)", R"(foo\_bar\_baz\%ham\%)"},
    };
    const std::string escape_chars{"_%"};

    for (const auto& i: vec) {
        const auto& inp = i.first;
        const auto& exp = i.second;

        std::string out = utils::escape(inp, escape_chars);
        CHECK(out == exp);
    }

    std::string inp{R"(\\\\)"};
    std::string out = utils::escape(inp, "\\");
    CHECK(out == inp);
}

TEST_CASE ("join_keys_map", "[utilities]")
{
    std::map <std::string, std::string> map1;
    std::map <int, std::string> map2;
    std::map <float, std::string> map3;
    std::map <uint32_t, std::string> map4;

    map1.emplace (std::make_pair ("a", "neco a"));
    map1.emplace (std::make_pair ("b", "neco b"));
    map1.emplace (std::make_pair ("c", "neco c"));

    map2.emplace (std::make_pair (1, "neco a"));
    map2.emplace (std::make_pair (-2, "neco b"));
    map2.emplace (std::make_pair (3, "neco c"));
    map2.emplace (std::make_pair (-10, "neco d"));

    map3.emplace (std::make_pair (1, "neco a"));
    map3.emplace (std::make_pair (2.2, "neco b"));
    map3.emplace (std::make_pair (3.33, "neco c"));

    map4.emplace (std::make_pair (1, "neco a"));
    map4.emplace (std::make_pair (2, "neco b"));
    map4.emplace (std::make_pair (3, "neco c"));



    CHECK (utils::join_keys_map (map1, ",").compare ("a,b,c") == 0);
    CHECK (utils::join_keys_map (map1, ", ").compare ("a, b, c") == 0);
    CHECK (utils::join_keys_map (map2, ";").compare ("-10;-2;1;3") == 0);
    CHECK (utils::join_keys_map (map3, ".").compare ("1.2.2.3.33") == 0);
    CHECK (utils::join_keys_map (map4, " ").compare ("1 2 3") == 0);

}

TEST_CASE ("join", "[utilities]") {
    const char *arr1[3] = { "mean", "min", "max" };
    uint32_t arr1_len = 3;

    const char *arr2[5] = { "mean", "min", "", "something", NULL };
    uint32_t arr2_len = 4;

    const char *arr3[4] = { "mean", "min", NULL, "max" };
    uint32_t arr3_len = 4;

    const char *arr4[4] = { NULL, "mean", "min", "max" };
    uint32_t arr4_len = 4;

    const char **arr5 = NULL;
    const char **arr6 = { NULL };
    const char **arr_empty = {};  
     

    SECTION ("version with length specified") {
        CHECK ( utils::join (arr1, arr1_len, ",").compare ("mean,min,max") == 0 );
        CAPTURE (utils::join (arr1, arr1_len, ","));
        CHECK ( utils::join (arr1, arr1_len, ", ").compare ("mean, min, max") == 0 );

        CHECK ( utils::join (arr2, arr2_len, ";").compare ("mean;min;;something") == 0 );
        CHECK ( utils::join (arr2, arr2_len, " , ").compare ("mean , min ,  , something") == 0 );

        CHECK ( utils::join (arr3, arr3_len, ",").compare ("mean,min") == 0 );
        CHECK ( utils::join (arr4, arr4_len, ",").empty () );

        CHECK ( utils::join (arr1, arr1_len - 1, ",").compare ("mean,min") == 0 );
        CHECK ( utils::join (arr2, arr2_len - 2, ";").compare ("mean;min") == 0 );
    }

    SECTION ("version without length specified") {
        CHECK ( utils::join (arr2, ",").compare ("mean,min,,something") == 0 );
        CHECK ( utils::join (arr2, ", ").compare ("mean, min, , something") == 0 );
        CHECK ( utils::join (arr3, ",").compare ("mean,min") == 0 );
    }
    SECTION ("bad invocation") {
        CHECK ( utils::join (arr1, arr1_len, NULL).empty () );
        CHECK ( utils::join (arr1, NULL).empty () );

        CHECK ( utils::join (NULL, ",").empty () );
        CHECK ( utils::join (NULL, 5, ",").empty () );
        CHECK ( utils::join (arr5, ",").empty () );
        CHECK ( utils::join (arr5, 5, ",").empty () );
        CHECK ( utils::join (arr6, ",").empty () );
        CHECK ( utils::join (arr6, 5, ",").empty () );
        CHECK ( utils::join (arr_empty, ",").empty () );
        CHECK ( utils::join (arr_empty, 5, ",").empty () );
        CHECK ( utils::join (arr4, arr4_len, NULL).empty () );
    }
}


TEST_CASE ("stobiosf", "[utilities]") {
    int8_t scale = 0;
    int32_t integer = 0;
    
    CHECK (utils::math::stobiosf ("12.835", integer, scale));
    CHECK ( integer == 12835 );
    CHECK ( scale == -3 );
 
    CHECK (utils::math::stobiosf ("178746.2332", integer, scale));
    CHECK ( integer == 1787462332 );
    CHECK ( scale == -4 );
 
    CHECK (utils::math::stobiosf ("0.00004", integer, scale));
    CHECK ( integer == 4 );
    CHECK ( scale == -5 );

    CHECK ( utils::math::stobiosf ("-12134.013", integer, scale) );
    CHECK ( integer == -12134013  );
    CHECK ( scale == -3 );

    CHECK ( utils::math::stobiosf ("-1", integer, scale) );
    CHECK ( integer == -1  );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("0", integer, scale) );
    CHECK ( integer == 0 );
    CHECK ( scale == 0 );
    
    CHECK ( utils::math::stobiosf ("1", integer, scale) );
    CHECK ( integer == 1 );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("0.0", integer, scale) );
    CHECK ( integer == 0 );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("0.00", integer, scale) );
    CHECK ( integer == 0 );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("1.0", integer, scale) );
    CHECK ( integer == 1 );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("1.00", integer, scale) );
    CHECK ( integer == 1 );
    CHECK ( scale == 0 );

    CHECK ( utils::math::stobiosf ("1234324532452345623541.00", integer, scale) == false );

    CHECK ( utils::math::stobiosf ("2.532132356545624522452456", integer, scale) == false );
    
    CHECK ( utils::math::stobiosf ("12x43", integer, scale) == false );
    CHECK ( utils::math::stobiosf ("sdfsd", integer, scale) == false );
}
