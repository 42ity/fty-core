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
