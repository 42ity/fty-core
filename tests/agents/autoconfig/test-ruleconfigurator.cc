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
 * tests for RuleConfigurator
 */

#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <catch.hpp>
#include <czmq.h>
#include <regex>
#include <cxxtools/regex.h>
#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationinfo.h>

#include "log.h"

#include "RuleConfigurator.h"

#define LUA_RULE(BIT) \
        " function has_bit(x,bit)"\
        "     local mask = 2 ^ (bit - 1)"\
        "     x = x % (2*mask)"\
        "     if x >= mask then return true else return false end"\
        " end"\
        " function main(status)"\
        "     if has_bit(status,"\
        #BIT\
        ") then return HIGH_CRITICAL end"\
        "     return OK"\
        " end"

TEST_CASE("RuleConfigurator", "[RuleConfigurator][agent-autoconfig]")
{
    RuleConfigurator rc;

    SECTION ("method make_results")
    {
        CHECK (
        rc.make_results (
            std::make_tuple (
            "kajo\"vajo",
            std::vector <std::string>{"action 1", "punk neni mrkef", "aja\nja"},
            "middle",
            "fuff")) ==
        "{ \"kajo\\\"vajo\" : { \"action\" : [ \"action 1\", \"punk neni mrkef\", \"aja\\nja\" ], \"severity\" : \"middle\", \"description\" : \"fuff\" }}"
        );

        CHECK (
        rc.make_results (
            std::make_tuple (
            "result name",
            std::vector <std::string>{},
            "low",
            "hata titla")) ==
        "{ \"result name\" : { \"action\" : [  ], \"severity\" : \"low\", \"description\" : \"hata titla\" }}"
        );

    }

    SECTION ("method make_rule") {
 
        std::string rule = rc.make_rule (
            RuleConfigurator::RuleType::SINGLE,
            "empty",
            std::vector<std::string>{"a"},
            NULL,
            std::vector <std::pair <std::string, std::string>>{},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {},
            NULL
        );
        CHECK (!rule.empty ());
        {
            std::istringstream in (rule);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("single");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            // rule_name
            root.getMember ("rule_name") >>= tmpstr;
            CHECK ( tmpstr == "empty" );
            // target
            CHECK ( root.getMember("target").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("target").memberCount () == 1 );
            root.getMember("target").begin ()->getValue (tmpstr);
            CHECK ( tmpstr == "a" );
            // element
            CHECK ( root.findMember ("element") == NULL );
            // values
            CHECK (root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            auto values = root.getMember("values");
            CHECK ( values.memberCount() == 0 );
            // results
            CHECK (root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            auto results = root.getMember("results");
            CHECK ( results.memberCount() == 0 );
            // evaluation
            CHECK ( root.findMember ("evaluation") == NULL );
        }

        rule = rc.make_rule (
            RuleConfigurator::RuleType::SINGLE,
            "full",
            std::vector<std::string>{ "a b", "cd", "abcd"},
            "b",
            std::vector <std::pair <std::string, std::string>>{ {"value name 1", "value 1"}, {"value_name_2", "value2"} },
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("result 1", std::vector <std::string> {}, "high", "hatta"),
                std::make_tuple ("result 2", std::vector <std::string> {"EMAIL", "SMS"}, "high", "titla")
            },
            "ab\ncd\""
        );
        CHECK (!rule.empty ());
        {
            std::istringstream in (rule);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("single");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            // rule_name
            root.getMember ("rule_name") >>= tmpstr;
            CHECK ( tmpstr == "full" );
            // target
            CHECK ( root.getMember("target").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("target").memberCount () == 3 );
            auto target = root.getMember("target");
            std::vector <std::string> target_vector;
            target >>= target_vector;
            CHECK ( target_vector[0] == "a b" );
            CHECK ( target_vector[1] == "cd" );
            CHECK ( target_vector[2] == "abcd" );

            // element
            root.getMember ("element") >>= tmpstr;
            CHECK ( tmpstr == "b" );
            // values
            CHECK (root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            auto values = root.getMember("values");
            CHECK ( values.memberCount() == 2 );
            auto values_it = values.begin ();
            values_it->getMember ("value name 1") >>= tmpstr;
            CHECK ( tmpstr == "value 1" );
            ++values_it;
            values_it->getMember ("value_name_2") >>= tmpstr;
            CHECK ( tmpstr == "value2" );
            // results
            CHECK (root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            auto results = root.getMember("results");
            CHECK ( results.memberCount() == 2 );
            auto results_it = results.begin ();
            CHECK (results_it->getMember ("result 1").getMember("action").category () == cxxtools::SerializationInfo::Array);
            CHECK (results_it->getMember ("result 1").getMember("action").memberCount () == 0);
            results_it->getMember ("result 1").getMember ("severity") >>= tmpstr;
            CHECK ( tmpstr == "high" );
            results_it->getMember ("result 1").getMember ("description") >>= tmpstr;
            CHECK ( tmpstr == "hatta" );
            
            ++results_it;
            CHECK (results_it->getMember ("result 2").getMember("action").category () == cxxtools::SerializationInfo::Array);
            CHECK (results_it->getMember ("result 2").getMember("action").memberCount () == 2);
            std::vector <std::string> results_action_vector;
            results_it->getMember ("result 2").getMember("action") >>= results_action_vector;
            CHECK (results_action_vector[0] == "EMAIL" );
            CHECK (results_action_vector[1] == "SMS" );

            results_it->getMember ("result 2").getMember ("severity") >>= tmpstr;
            CHECK ( tmpstr == "high" );
            results_it->getMember ("result 2").getMember ("description") >>= tmpstr;
            CHECK ( tmpstr == "titla" );

            // evaluation
            root.getMember ("evaluation") >>= tmpstr;
            CHECK ( tmpstr == "ab\ncd\"" );
        }

        rule = rc.make_rule (
            RuleConfigurator::RuleType::THRESHOLD,
            "simple - threshold",
            std::vector<std::string>{"average.temperature@DC-Roztoky"},
            "DC-Roztoky",
            std::vector <std::pair <std::string, std::string>>{
                {"low_critical","10"}, {"low_warning","20"}, {"high_warning","30"}, {"high_critical","40"}},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("low_critical", std::vector <std::string> {"EMAIL", "SMS"}, "high", "freezing"),
                std::make_tuple ("low_warning", std::vector <std::string> {}, "low", "cold"),
                std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "low", "warm"),
                std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL", "SMS", "RELEASE PIDGEON"}, "high", "hot")
            },
            NULL
        );
        CHECK (!rule.empty ());
        {
            std::istringstream in (rule);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("threshold");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            // rule_name
            root.getMember ("rule_name") >>= tmpstr;
            CHECK ( tmpstr == "simple - threshold" );
            // target
            CHECK ( root.getMember("target").category () == cxxtools::SerializationInfo::Value);
            root.getMember("target") >>= tmpstr;
            CHECK ( tmpstr == "average.temperature@DC-Roztoky" );
            // element
            root.getMember ("element") >>= tmpstr;
            CHECK ( tmpstr == "DC-Roztoky" );
            // values
            CHECK ( root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("values").memberCount() == 4 );
            // results
            CHECK ( root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("results").memberCount() == 4 );
            // evaluation
            CHECK ( root.findMember ("evaluation") == NULL );
        }

        rule = rc.make_rule (
            RuleConfigurator::RuleType::THRESHOLD,
            "complex - threshold",
            std::vector<std::string>{"average.temperature@DC-Roztoky", "x@y", "10"},
            "DC-Roztoky",
            std::vector <std::pair <std::string, std::string>>{
                {"low_critical","10"}, {"low_warning","20"}, {"high_warning","30"}, {"high_critical","40"}},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("low_critical", std::vector <std::string> {"EMAIL", "SMS"}, "high", "freezing"),
                std::make_tuple ("low_warning", std::vector <std::string> {}, "low", "cold"),
                std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "low", "warm"),
                std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL", "SMS", "RELEASE PIDGEON"}, "high", "hot")
            },
            LUA_RULE("xyz")
        );
        CHECK (!rule.empty ());
        {
            std::istringstream in (rule);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("threshold");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            // rule_name
            root.getMember ("rule_name") >>= tmpstr;
            CHECK ( tmpstr == "complex - threshold" );
            // target
            CHECK ( root.getMember("target").category () == cxxtools::SerializationInfo::Array );
            CHECK ( root.getMember("target").memberCount () == 3 );
            // values
            CHECK ( root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("values").memberCount() == 4 );
            // results
            CHECK ( root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("results").memberCount() == 4 );
 
            // evaluation
            root.getMember ("evaluation") >>= tmpstr;
            CHECK ( !tmpstr.empty() );
        }

        rule = rc.make_rule (
            RuleConfigurator::RuleType::PATTERN,
            "pattern",
            std::vector<std::string>{"regexp", "sdwrt rw", "dfwesdfw"},
            "DC-Roztoky",
            std::vector <std::pair <std::string, std::string>>{
                {"low_critical","10"}
                },
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("low_critical", std::vector <std::string> {"WWW"}, "", "")
            },
            LUA_RULE(7)
        );
        CHECK (!rule.empty ());
        {
            std::istringstream in (rule);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("pattern");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            // rule_name
            root.getMember ("rule_name") >>= tmpstr;
            CHECK ( tmpstr == "pattern" );
            // target
            CHECK ( root.getMember("target").category () == cxxtools::SerializationInfo::Value );
            root.getMember("target") >>= tmpstr;
            CHECK ( tmpstr == "regexp" );
            // values
            CHECK ( root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("values").memberCount() == 1 );
            // results
            CHECK ( root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            CHECK ( root.getMember("results").memberCount() == 1 );
            // evaluation
            root.getMember ("evaluation") >>= tmpstr;
            CHECK ( !tmpstr.empty() );
        }
    }

    SECTION ("method makeSimpleThresholdRule") {
        std::string threshold = rc.makeSimpleThresholdRule (
            "meno",
            "topic",
            "parek",
            std::make_tuple ("10", std::vector <std::string>{"low_1", "low_2", "low3" }, "high", "x"),
            std::make_tuple ("23", std::vector <std::string>{"low_4", "low_5"}, "low", "yy"),
            std::make_tuple ("50", std::vector <std::string>{"asdfw"}, "stredna", "aaa"),
            std::make_tuple ("75", std::vector <std::string>{"high_1", "high_2", "high_3", "high_4"}, "vysoka", "bbbb")
        );
        CHECK (!threshold.empty ());
        {
            // deserialize
            std::istringstream in (threshold);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW (deserializer.deserialize (si); );
            
            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto root = si.getMember ("threshold");
            CHECK (root.category () == cxxtools::SerializationInfo::Object);

            std::string tmpstr;
            root.getMember ("rule_name") >>= tmpstr;
            CHECK (tmpstr == "meno" );

            root.getMember ("target") >>= tmpstr;
            CHECK (tmpstr == "topic");

            root.getMember ("element") >>= tmpstr;
            CHECK (tmpstr == "parek");
            
            CHECK (root.findMember ("evaluation") == NULL );
            
            // values
            CHECK (root.getMember("values").category () == cxxtools::SerializationInfo::Array);
            auto values = root.getMember("values");
            CHECK ( values.memberCount() == 4 );
           
            auto values_it = values.begin ();
            values_it->getMember ("low_critical") >>= tmpstr;
            CHECK ( tmpstr == "10" );
            ++values_it;
            values_it->getMember ("low_warning") >>= tmpstr;
            CHECK ( tmpstr == "23" );
            ++values_it;
            values_it->getMember ("high_warning") >>= tmpstr;
            CHECK ( tmpstr == "50" );
            ++values_it;
            values_it->getMember ("high_critical") >>= tmpstr;
            CHECK ( tmpstr == "75" );

            // results
            CHECK (root.getMember("results").category () == cxxtools::SerializationInfo::Array);
            auto results = root.getMember("results");
            CHECK ( results.memberCount() == 4 );
            auto results_it = results.begin ();
            CHECK (results_it->getMember ("low_critical").getMember("action").category () == cxxtools::SerializationInfo::Array);
            CHECK (results_it->getMember ("low_critical").getMember("action").memberCount () == 3);
            std::vector <std::string> results_action_vector;
            results_it->getMember ("low_critical").getMember("action") >>= results_action_vector;
            CHECK (results_action_vector[0] == "low_1" );
            CHECK (results_action_vector[1] == "low_2" );
            CHECK (results_action_vector[2] == "low3" );

            results_it->getMember ("low_critical").getMember ("severity") >>= tmpstr;
            CHECK ( tmpstr == "high" );
            results_it->getMember ("low_critical").getMember ("description") >>= tmpstr;
            CHECK ( tmpstr == "x" );
            
            ++results_it;
            CHECK (results_it->getMember ("low_warning").getMember("action").category () == cxxtools::SerializationInfo::Array);
            CHECK (results_it->getMember ("low_warning").getMember("action").memberCount () == 2);
            results_it->getMember ("low_warning").getMember("action") >>= results_action_vector;
            CHECK (results_action_vector[0] == "low_4" );
            CHECK (results_action_vector[1] == "low_5" );

            results_it->getMember ("low_warning").getMember ("severity") >>= tmpstr;
            CHECK ( tmpstr == "low" );
            results_it->getMember ("low_warning").getMember ("description") >>= tmpstr;
            CHECK ( tmpstr == "yy" );

           
        }
    }

    SECTION ("method makeThresholdRule") {
         std::string threshold = rc.makeThresholdRule (
            "DcImbalance",
            std::vector<std::string>{"status.ups@ups-9", "status.sensor@ups-9"},
            "ups-9",
            std::vector <std::pair <std::string, std::string>>{ {"high-warning","100"}, {"critical","200"}},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("high-warning", std::vector <std::string> {"action 1", "action 2"}, "high", "l"),
                std::make_tuple ("critical", std::vector <std::string> {"action 1"}, "low", "h"),
            },
            "" 
        );
        // deserialize
        {
            std::istringstream in (threshold);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );

            // TODO      
        }
    }

    SECTION ("method makeSingleRule") {
        std::string single = rc.makeSingleRule (
            "alert-ups9",
            std::vector<std::string>{"status.ups@ups-9", "status.sensor@ups-9"},
            "ups-9",
            std::vector <std::pair <std::string, std::string>>{ {"value name 1","10"}, {"value name 2","20"}},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("result name 1", std::vector <std::string> {"action 1", "action 2"}, "high", "something"),
                std::make_tuple ("result name 2", std::vector <std::string> {"action 1"}, "low", "something else"),
            },
            LUA_RULE(5) 
        );
        
        // deserialize
        {
            std::istringstream in (single);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );      

            CHECK (si.category () == cxxtools::SerializationInfo::Object);
            auto si_single = si.getMember ("single");
            CHECK (si_single.category () == cxxtools::SerializationInfo::Object);
            std::string tmpstr;
            si_single.getMember ("rule_name") >>= tmpstr;
            CHECK (tmpstr == "alert-ups9" );
            
            CHECK (si_single.getMember("target").category () == cxxtools::SerializationInfo::Array);
            // TODO check values of target

            si_single.getMember ("element") >>= tmpstr;
            CHECK (tmpstr == "ups-9");
            si_single.getMember ("evaluation") >>= tmpstr;
            // TODO check value of evaluation
            CHECK (si_single.getMember("values").category () == cxxtools::SerializationInfo::Array);
            CHECK (si_single.getMember("results").category () == cxxtools::SerializationInfo::Array);
            // TODO value checks of 'values', 'results'
        }
    }
    
    SECTION ("method makePatternRule") {
        std::string pattern = rc.makePatternRule (
            "alert-ups9",
            "status.ups@ups*",
            std::vector <std::pair <std::string, std::string>>{ {"value name 1","10"}, {"value name 2","20"}},
            std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                std::make_tuple ("result name 1", std::vector <std::string> {"action 1", "action 2"}, "high", "something"),
                std::make_tuple ("result name 2", std::vector <std::string> {"action 1"}, "low", "something else"),
            },
            LUA_RULE(5) 
        );
        
        // deserialize
        {
            std::istringstream in (pattern);
            cxxtools::JsonDeserializer deserializer(in);
            cxxtools::SerializationInfo si;
            CHECK_NOTHROW ( deserializer.deserialize (si); );

            // TODO
        }
    }
}
