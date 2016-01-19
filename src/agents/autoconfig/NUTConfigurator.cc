/* 
Copyright (C) 2014 - 2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*!
 \file   NutConfigurator.cc
 \brief  Implementation of class for nut configuration
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include <algorithm>
#include <iostream>
#include <fstream>

#include <cxxtools/regex.h>

#include "log.h"
#include "preproc.h"
#include "subprocess.h"
#include "utils.h"
#include "filesystem.h"
#include "nutscan.h"

#include "NUTConfigurator.h"

#define NUT_PART_STORE "/etc/bios/nut/devices"

static const char * NUTConfigXMLPattern = "[[:blank:]]driver[[:blank:]]+=[[:blank:]]+\"netxml-ups\"";
static const char * NUTConfigEpduPattern = "[[:blank:]](mibs[[:blank:]]+=[[:blank:]]+\"(eaton_epdu|aphel_genesisII)\"|"
                                           "desc[[:blank:]]+=[[:blank:]]+\"[^\"]+ epdu [^\"]+\")";
static const char * NUTConfigCanSnmpPattern = "[[:blank:]]driver[[:blank:]]+=[[:blank:]]+\"snmp-ups\"";

using namespace persist;

std::vector<std::string>::const_iterator NUTConfigurator::stringMatch(const std::vector<std::string> &texts, const char *pattern) {
    log_debug("regex: %s", pattern );
    cxxtools::Regex reg( pattern, REG_EXTENDED | REG_ICASE );
    for( auto it = texts.begin(); it != texts.end(); ++it ) {
        if( reg.match( *it ) ) {
            log_debug("regex: match found");
            return it;
        }
    }
    log_debug("regex: not found");
    return texts.end();
}


bool NUTConfigurator::match( const std::vector<std::string> &texts, const char *pattern) {
    return stringMatch(texts,pattern) != texts.end();
}

bool NUTConfigurator::isEpdu( const std::vector<std::string> &texts) {
    return match( texts, NUTConfigEpduPattern );
}

bool NUTConfigurator::isUps( const std::vector<std::string> &texts) {
    return ! isEpdu(texts);
}

bool NUTConfigurator::canSnmp( const std::vector<std::string> &texts) {
    return match( texts, NUTConfigCanSnmpPattern );
}

bool NUTConfigurator::canXml( const std::vector<std::string> &texts) {
    return match( texts, NUTConfigXMLPattern );
}

std::vector<std::string>::const_iterator NUTConfigurator::getBestSnmpMib(const std::vector<std::string> &configs) {
    static const std::vector<std::string> snmpMibPriority = {
        "pw", "mge", ".+"
    };
    for( auto mib = snmpMibPriority.begin(); mib != snmpMibPriority.end(); ++mib ) {
        std::string pattern = ".+[[:blank:]]mibs[[:blank:]]+=[[:blank:]]+\"" + *mib + "\"";
        auto it = stringMatch( configs, pattern.c_str() );
        if( it != configs.end() ) return it;
    }
    return configs.end();
}

std::vector<std::string>::const_iterator NUTConfigurator::selectBest(const std::vector<std::string> &configs) {
    // don't do any complicated decision on empty/single set
    if( configs.size() <= 1 ) return configs.begin();

    log_debug("isEpdu: %i; isUps: %i; canSnmp: %i; canXml: %i", isEpdu(configs), isUps(configs), canSnmp(configs), canXml(configs) );
    if( canSnmp( configs ) && isEpdu( configs ) ) {
        log_debug("SNMP capable EPDU => Use SNMP");
        return getBestSnmpMib( configs );
    } else {
        if( canXml( configs ) ) {
            log_debug("XML capable device => Use XML");
            return stringMatch( configs, NUTConfigXMLPattern );
        } else {
            log_debug("SNMP capable device => Use SNMP");
            return getBestSnmpMib( configs );
        }
    }
};

void NUTConfigurator::systemctl( const std::string &operation, const std::string &service )
{
    std::vector<std::string> _argv = {"sudo", "systemctl", operation, service };
    shared::SubProcess systemd( _argv );
    if( systemd.run() ) {
        int result = systemd.wait();
        log_info("sudo systemctl %s %s result: %i (%s)",
                 operation.c_str(),
                 service.c_str(),
                 result,
                 (result == 0 ? "ok" : "failed"));
    } else {
        log_error("can't run sudo systemctl %s %s command",
                  operation.c_str(),
                  service.c_str() );
    }
}

void NUTConfigurator::updateNUTConfig() {
    std::vector<std::string> _argv = { "sudo", "bios-nutconfig" };
    shared::SubProcess systemd( _argv );
    if( systemd.run() ) {
        int result = systemd.wait();
        log_info("sudo bios-nutconfig %i (%s)",
                 result,
                 (result == 0 ? "ok" : "failed"));
    } else {
        log_error("can't run sudo bios-nutconfig command");
    }
}

std::string NUTConfigurator::makeRule(std::string const &alert, std::string const &bit, std::string const &device, std::string const &description) const {
    return
        "{\n"
        "\"single\" : {\n"
        "    \"rule_name\"     :   \"" + alert + "-" + device + "\",\n"
        "    \"target\"        :   [\"status.ups@" + device + "\"],\n"
        "    \"element\"       :   \"" + device + "\",\n"
        "    \"results\"       :   [ {\"high_critical\"  : { \"action\" : [ \"EMAIL\" ], \"description\" : \""+description+"\" }} ],\n"
        "    \"evaluation\"    : \""
        " function has_bit(x,bit)"
        "     local mask = 2 ^ (bit - 1)"
        "     x = x % (2*mask)"
        "     if x >= mask then return true else return false end"
        " end"
        " function main(status)"
        "     if has_bit(status,"+bit+") then return HIGH_CRITICAL end"
        "     return OK"
        " end"
        "\"\n"
        "  }\n"
        "}";
};

std::vector<std::string> NUTConfigurator::createRules(std::string const &name) {
    std::vector<std::string> result;

    // bits OB - 5 LB - 7 BYPASS - 9

    result.push_back (makeRule ("onbattery","5",name,"UPS is running on battery!"));
    result.push_back (makeRule ("lowbattery","7",name,"Battery depleted!"));
    result.push_back (makeRule ("onbypass","9",name,"UPS is running on bypass!"));
    return result;
}

bool NUTConfigurator::configure( const std::string &name, const AutoConfigurationInfo &info ) {
    log_debug("NUT configurator created");

    switch( info.operation ) {
    case asset_operation::INSERT:
    case asset_operation::UPDATE:
        {
            auto ipit = info.attributes.find("ip.1");
            if( ipit == info.attributes.end() ) {
                log_error("device %s has no IP address", name.c_str() );
                return true;
            }
            std::string IP = ipit->second;

            std::vector<std::string> configs;
            shared::nut_scan_snmp( name, shared::CIDRAddress(IP), configs );
            shared::nut_scan_xml_http( name, shared::CIDRAddress(IP), configs );

            auto it = selectBest( configs );
            if( it == configs.end() ) {
                log_error("nut-scanner failed for device \"%s\", no suitable configuration found", name.c_str() );
                return false; // try again later
            }
            std::string deviceDir = NUT_PART_STORE;
            shared::mkdir_if_needed( deviceDir.c_str() );
            std::ofstream cfgFile;
            log_info("creating new config file %s/%s", NUT_PART_STORE, name.c_str() );
            cfgFile.open(std::string(NUT_PART_STORE) + shared::path_separator() + name );
            cfgFile << *it;
            {
                std::string s = *it;
                // prototypes expects std::vector <std::string> - lets create fake vector
                // this is not performance critical code anyway
                std::vector <std::string> foo = {s};
                if (isEpdu (foo) && canSnmp (foo)) {
                    log_debug ("add synchronous = yes");
                    cfgFile << "\tsynchronous = yes\n";
                }
            }
            cfgFile.close();
            updateNUTConfig();
            systemctl("enable",  std::string("nut-driver@") + name);
            systemctl("restart", std::string("nut-driver@") + name);
            systemctl("restart", "nut-server");
            return true;
        }
    case asset_operation::DELETE:
    case asset_operation::RETIRE:
        {
            log_info("removing configuration file %s/%s", NUT_PART_STORE, name.c_str() );
            std::string fileName = std::string(NUT_PART_STORE)
                + shared::path_separator()
                + name;
            remove( fileName.c_str() );
            updateNUTConfig();
            systemctl("stop",    std::string("nut-driver@") + name);
            systemctl("disable", std::string("nut-driver@") + name);
            systemctl("restart", "nut-server");
            return true;
        }
    default:
        log_error("invalid configuration operation %" PRIi8, info.operation);
        return true; // true means do not try again this
    }
}


