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
