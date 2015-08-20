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
 * \file test-autoconfig.cc
 * \author Tomas Halman
 * \brief Not yet documented file
 */
/*
 * tests for autoconfig
 */

#include <catch.hpp>
#include <czmq.h>
#include <regex>
#include <cxxtools/regex.h>

#include "configurator.h"

const char *nutEpduSnmp = "[nutdev1]\n"
    "\tdriver = \"snmp-ups\"\n"
    "\tport = \"10.130.38.111\"\n"
    "\tdesc = \"Eaton ePDU AM 1P IN:C14 10A OUT:16xC13\"\n"
    "\tmibs = \"eaton_epdu\"\n"
    "\tcommunity = \"public\"\n";

const char *nutUpsXml = "[nutdev2]\n"
    "\tdriver = \"netxml-ups\"\n"
    "\tport = \"http://10.130.36.79\"\n"
    "\tdesc = \"Mosaic 4M 16M\"\n";

const char *nutUpsSnmp = "[nutdev1]\n"
    "\tdriver = \"snmp-ups\"\n"
    "\tport = \"10.130.38.21\"\n"
    "\tdesc = \"Eaton 9PX\"\n"
    "\tmibs = \"mge\"\n"
    "\tcommunity = \"public\"\n";

const char *nutUpsSnmpPw = "[nutdev1]\n"
    "\tdriver = \"snmp-ups\"\n"
    "\tport = \"10.130.38.21\"\n"
    "\tdesc = \"Eaton 9PX\"\n"
    "\tmibs = \"pw\"\n"
    "\tcommunity = \"public\"\n";

const char *nutEpduXml = nutUpsXml; // TODO: get xml scan for epdu
    
TEST_CASE("autoconfig-preference", "[agent-autoconfig]") {
    std::vector<std::string> EPDU = { nutEpduSnmp, nutEpduXml };
    std::vector<std::string> UPSXML = { nutUpsSnmp, nutUpsXml };
    std::vector<std::string> UPSPW = { nutUpsSnmp, nutUpsSnmpPw };
    NUTConfigurator nut;

    auto it = nut.selectBest(EPDU);
    CHECK( ( it == EPDU.end() ? "" : *it ) == nutEpduSnmp );
    it = nut.selectBest(UPSXML);
    CHECK( ( it == UPSXML.end() ? "" : *it ) == nutUpsXml );
    it = nut.selectBest(UPSPW);
    CHECK( ( it == UPSPW.end() ? "" : *it ) == nutUpsSnmpPw );
}

