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

