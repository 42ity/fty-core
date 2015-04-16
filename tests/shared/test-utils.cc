#include <catch.hpp>

#include "utils.h"

TEST_CASE("str_eq","[utils][streq]"){
    CHECK(streq(NULL, NULL));
    CHECK_FALSE(str_eq("a", NULL));
    CHECK_FALSE(str_eq(NULL, "b"));
    CHECK(streq("a", "a"));
    CHECK_FALSE(str_eq("yyy","bb"));
    CHECK_FALSE(str_eq("yyy","yy"));
    CHECK_FALSE(str_eq("YY","yy"));
    CHECK_FALSE(str_eq("\0",NULL));
    CHECK(streq("","\0")); 
} 


TEST_CASE("str_bool","[str_bool][utils]") {
    CHECK(streq(str_bool(true), "true"));
    CHECK(streq(str_bool(false), "false"));
    CHECK_FALSE(str_eq(str_bool(true),"True"));
    CHECK_FALSE(str_eq(str_bool(false),"False"));
}

TEST_CASE("safe_str","[safe_str][utils]"){
    CHECK(streq(safe_str(NULL), "(null)"));
    CHECK(streq(safe_str("a"), "a"));
    CHECK(streq(safe_str("\0"), ""));
}
