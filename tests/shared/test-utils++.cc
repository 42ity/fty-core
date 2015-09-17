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

TEST_CASE ("utils::json::escape","[utils::math::dtos][json][escape]")
{
    std::string x;

    x = utils::json::escape ("'jednoduche ' uvozovky'");
    CHECK ( x.compare ("'jednoduche ' uvozovky'") == 0);

    x = utils::json::escape ("dvojite \" stred");
    CHECK ( x.compare (R"(dvojite \" stred)") == 0);

    x = utils::json::escape ("dvojite \\\" stred");
    CHECK ( x.compare (R"(dvojite \" stred)") == 0);

    x = utils::json::escape ("dvojite \\\\\" stred");
    CHECK ( x.compare (R"(dvojite \\\" stred)") == 0);

    x = utils::json::escape ("dvojite \\\\\\\" stred");
    CHECK ( x.compare (R"(dvojite \\\" stred)") == 0);

    x = utils::json::escape ("dvojite \\\\\\\\\" uvozovky");
    CHECK ( x.compare (R"(dvojite \\\\\" uvozovky)") == 0);

    x = utils::json::escape ("dvojite \"\" stred");
    CHECK ( x.compare (R"(dvojite \"\" stred)") == 0);

    x = utils::json::escape ("dvojite \"\"\" stred");
    CHECK ( x.compare (R"(dvojite \"\"\" stred)") == 0);

    x = utils::json::escape ("dvojite \"\\\"\" stred");
    CHECK ( x.compare (R"(dvojite \"\"\" stred)") == 0);    

    x = utils::json::escape ("dvojite \"\"\\\" stred");
    CHECK ( x.compare (R"(dvojite \"\"\" stred)") == 0);

    x = utils::json::escape ("dvojite \\\"\\\"\\\" uvozovky");
    CHECK ( x.compare (R"(dvojite \"\"\" uvozovky)") == 0);

    x = utils::json::escape ("dvojite \\\\\"\\\\\"\\\\\" uvozovky");
    CHECK ( x.compare (R"(dvojite \\\"\\\"\\\" uvozovky)") == 0);

    x = utils::json::escape ("dvojite \uA66A uvozovky");
    CHECK ( x.compare ("dvojite \uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \"\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\"\uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \\uA66A uvozovky");
    CHECK ( x.compare (R"(dvojite \uA66A uvozovky)") == 0);

    x = utils::json::escape ("dvojite \\\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\\\uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \\\\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\\uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \\\\\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\\\uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \\\\\\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\\\\uA66A uvozovky") == 0);

    x = utils::json::escape ("dvojite \\\\\\\uA66A uvozovky");
    CHECK ( x.compare ("dvojite \\\\\\\\\uA66A uvozovky") == 0);

    x =  utils::json::escape ("dvojite \uA66A\" uvozovky");
    CHECK ( x.compare ("dvojite \uA66A\\\" uvozovky") == 0);

    x = utils::json::escape ("dvojite \\b \\\\b \\\\\\b \\\\\\\\b \\\\\\\\\\b uvozovky");
    CHECK ( x.compare (R"(dvojite \b \\b \\\b \\\\b \\\\\b uvozovky)") == 0);

    x = utils::json::escape ("dvojite \\t \\\t \\\\t \\\\\t \\\\\\t uvozovky");
    CHECK ( x.compare ("dvojite \\t \\\\\t \\\\t \\\\\t \\\\\\t uvozovky") == 0);

    x = utils::json::escape ("dvojite \\f \\\f \\\\f \\\\\f \\\\\\f uvozovky");
    CHECK ( x.compare ("dvojite \\f \\\\\f \\\\f \\\\\f \\\\\\f uvozovky") == 0);

    x = utils::json::escape ("dvojite \\n \\\n \\\\n \\\\\n \\\\\\n uvozovky");
    CHECK ( x.compare ("dvojite \\n \\\\\n \\\\n \\\\\n \\\\\\n uvozovky") == 0);

    x = utils::json::escape ("dvojite \\r \\\r \\\\r \\\\\r \\\\\\r uvozovky");
    CHECK ( x.compare ("dvojite \\r \\\\\r \\\\r \\\\\r \\\\\\r uvozovky") == 0);

    x = utils::json::escape ("dvojite \\ \\\\ \\\\\\ \\\\\\\\ uvozovky");
    CHECK ( x.compare ("dvojite \\\\ \\\\ \\\\\\\\ \\\\\\\\ uvozovky") == 0);
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
