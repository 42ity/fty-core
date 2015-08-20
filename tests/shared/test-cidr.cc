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
 * \file test-cidr.cc
 * \author Tomas Halman
 * \author Karol Hrdina
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <cidr.h>

using namespace shared;

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
  REQUIRE( a6small != a6big );
  struct in_addr a = { 0x0200007F }; // ugly, this can fail on big-endian machine
  REQUIRE( CIDRAddress(&a) == "127.0.0.2" );
  REQUIRE( CIDRAddress(&a).prefix() == 32 );
  struct sockaddr_in6 a6 = {
      .sin6_family = AF_INET6,
      .sin6_port = 0,
      .sin6_flowinfo = 0,
      .sin6_addr = { '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
                     '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x42' },
      .sin6_scope_id = 0
  };
  REQUIRE( CIDRAddress( (struct sockaddr *)&a6 ) == "::42" );
  REQUIRE( CIDRAddress( "1.1.1.1/255.255.255.0" ).valid() );
  REQUIRE( ! CIDRAddress( "1.1.1.1/255.255.255.1" ).valid() );
  REQUIRE( CIDRAddress( "1.1.1.1/24" ).netmask() == "255.255.255.0" );
  REQUIRE( ! CIDRAddress("255.255.0.3").isNetmask() );
  REQUIRE( CIDRAddress("255.128.0.0").isNetmask() );
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

