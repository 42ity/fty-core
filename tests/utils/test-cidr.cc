#include <catch.hpp>
#include <cidr.h>

using namespace utils;

TEST_CASE("CIDR Operators","[cidr][operators]") {
  CIDRAddress a4small("127.0.0.1"),a4big("127.0.0.2"),a6small("::1"),a6big("::2");
  REQUIRE( a4small < a4big );
  REQUIRE( a4big > a4small );
  REQUIRE( a6small < a6big );
  REQUIRE( a6big > a6small );
  a4small++;
  REQUIRE( a4small == a4big );
  a6small++;
  REQUIRE( a6small == a6big );
  a4small--;
  REQUIRE( a4small < a4big );
  a6small--;
  REQUIRE( a6small < a6big );
}

TEST_CASE("CIDR string manipulation","[cidr][operators]") {
  CIDRAddress a4,a6;
  a4 = "127.0.0.1";
  a6 = "::1";
  REQUIRE( a4.toString() == "127.0.0.1" );
  REQUIRE( a6.toString() == "::1" );
  REQUIRE( a4.toString(CIDR_WITH_PREFIX) == "127.0.0.1/32" );
  REQUIRE( a6.toString(CIDR_WITH_PREFIX) == "::1/128" );
}

TEST_CASE("CIDR list","[cidr][list iterating]") {
  CIDRAddress a;
  CIDRList L;
  L.add("1.2.3.0/29");
  L.exclude("1.2.3.0/30");
  L.add("1.2.3.2/32");
  L.next(a);
  REQUIRE( a.toString() == "1.2.3.2" );
  L.next(a);
  REQUIRE( a.toString() == "1.2.3.4" );
  L.next(a);
  REQUIRE( a.toString() == "1.2.3.5" );
  L.next(a);
  REQUIRE( a.toString() == "1.2.3.6" );
  L.next(a);
  REQUIRE( ! a.valid() );
}

