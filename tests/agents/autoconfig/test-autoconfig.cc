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
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \brief Not yet documented file
 */
/*
 * tests for autoconfig
 */
#include <iostream>
#include <vector>
#include <string>
#include <catch.hpp>
#include <czmq.h>
#include <regex>
#include <cxxtools/regex.h>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationinfo.h>


#include "NUTConfigurator.h"
#include "RuleConfigurator.h"

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

TEST_CASE("RuleConfigurator", "[RuleConfigurator][agent-autoconfig]")
{
    RuleConfigurator rc;

    SECTION ("method makeThresholdRule") {
        std::string threshold = rc.makeThresholdRule (
            "meno",
            std::vector<std::string>{"topic"},
            "parek",
            std::make_tuple ("10", std::vector <std::string>{"low_1", "low_2", "low3" }, "high", "x"),
            std::make_tuple ("23", std::vector <std::string>{"low_4", "low_5"}, "low", "yy"),
            std::make_tuple ("50", std::vector <std::string>{"asdfw"}, "stredna", "aaa"),
            std::make_tuple ("75", std::vector <std::string>{"high_1", "high_2", "high_3", "high_4"}, "vysoka", "bbbb"),
            "one line\n"
            "\tsecond\"line"
        );
        CHECK (!threshold.empty ());
        
        // deserialize
        std::istringstream in (threshold);
        cxxtools::JsonDeserializer deserializer(in);
        cxxtools::SerializationInfo si;
        CHECK_NOTHROW (deserializer.deserialize (si); );
        
        CHECK (si.category () == cxxtools::SerializationInfo::Object);
        auto si_threshold = si.getMember ("threshold");
        CHECK (si_threshold.category () == cxxtools::SerializationInfo::Object);
        std::string tmpstr;
        si_threshold.getMember ("rule_name") >>= tmpstr;
        CHECK (tmpstr == "meno" );
        si_threshold.getMember ("target") >>= tmpstr;
        CHECK (tmpstr == "topic");
        si_threshold.getMember ("element") >>= tmpstr;
        CHECK (tmpstr == "parek");
        si_threshold.getMember ("evaluation") >>= tmpstr;
        CHECK (tmpstr == "one line\n\tsecond\"line");
        CHECK (si_threshold.getMember("values").category () == cxxtools::SerializationInfo::Array);
        CHECK (si_threshold.getMember("results").category () == cxxtools::SerializationInfo::Array);
        // TODO value checks of 'values', 'results'
 
        threshold = rc.makeThresholdRule (
            "hata titla",
            std::vector<std::string>{"jedna", "dva", "tri"},
            "parek",
            std::make_tuple ("10", std::vector <std::string>{}, "high", "x"),
            std::make_tuple ("23", std::vector <std::string>{"", "low_5"}, "low", "yy"),
            std::make_tuple ("50", std::vector <std::string>{"asdfw"}, "stredna", "aaa"),
            std::make_tuple ("75", std::vector <std::string>{"high_1", "high_2", "high_3", "high_4"}, "vysoka", "bbbb"),
            NULL
        );
        CHECK (!threshold.empty ());
        
        // deserialize
        std::istringstream in2 (threshold);
        cxxtools::JsonDeserializer deserializer2 (in2);
        cxxtools::SerializationInfo si2;
        CHECK_NOTHROW (deserializer2.deserialize (si2););
        
        CHECK (si2.category () == cxxtools::SerializationInfo::Object);
        si_threshold = si2.getMember ("threshold");
        CHECK (si_threshold.category () == cxxtools::SerializationInfo::Object);
        si_threshold.getMember ("rule_name") >>= tmpstr;
        CHECK (tmpstr == "hata titla" );

        std::vector<std::string> tmpvector;
        si_threshold.getMember ("target") >>= tmpvector;
        CHECK (tmpvector.size () == 3);
        CHECK (tmpvector[0] == "jedna");
        CHECK (tmpvector[1] == "dva");
        CHECK (tmpvector[2] == "tri");
        si_threshold.getMember ("element") >>= tmpstr;
        CHECK (tmpstr == "parek");
        CHECK (si_threshold.getMember("values").category () == cxxtools::SerializationInfo::Array);
        CHECK (si_threshold.getMember("results").category () == cxxtools::SerializationInfo::Array);       
    }

}
