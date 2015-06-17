/* 
Copyright (C) 2014 - 2015 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Tomas Halman <tomashalman@eaton.com>
 
Description: Clases for autoconfiguration
*/

#include "configurator.h"

#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "nutscan.h"
#include "log.h"
#include "subprocess.h"
#include "asset_types.h"
#include "utils.h"
#include "filesystem.h"

#define NUT_PART_STORE "/etc/bios/nut/devices"

void NUTConfigurator::updateNUTConfig() {
    std::ofstream cfgFile;
    std::string spath = NUT_PART_STORE;
    spath += shared::path_separator();
    const char *NUT_CONFIG_DIR = NULL;

    if( shared::is_dir("/etc/nut") ) { NUT_CONFIG_DIR = "/etc/nut"; }
    else if( shared::is_dir("/etc/ups") ) { NUT_CONFIG_DIR = "/etc/ups"; }

    if( ! NUT_CONFIG_DIR ) { log_debug("can't find NUT configuration directory"); return; }

    cfgFile.open(std::string(NUT_CONFIG_DIR) + shared::path_separator() + "ups.conf");    
    for( auto it : shared::files_in_directory( NUT_PART_STORE ) ) {
        std::ifstream device( spath + it );
        cfgFile << device.rdbuf();
        device.close();
    }
    cfgFile.close();
    std::vector<std::string> _argv = {"/usr/bin/systemctl", "restart", "nut-driver"};
    shared::SubProcess systemd( _argv );
    systemd.run();
    int restart = systemd.wait();
    log_debug("nut-driver restart result: %i (%s)", restart, (restart == 0 ? "ok" : "failed"));
}

bool NUTConfigurator::configure( const char *name, zhash_t *extendedAttributes, int8_t eventType ) {
    std::vector<std::string> configs;
    log_debug("NUT configurator");
    if( eventType == 1 || eventType == 2 ) { // TODO: enum? numbers come from agents.h bios_asset_encode/extract
        log_debug("configuration updating request");
        const char *IP = (const char *)zhash_lookup( extendedAttributes, "ip.1" );
        if( ! IP ) {
            log_debug("device %s has no IP address", name);
            return false;
        }
        shared::nut_scan_snmp( name, shared::CIDRAddress(IP), NULL, configs );
        auto it = selectBest( configs );
        if( it == configs.end() ) {
            log_debug("nutscanner failed");
            return false;
        }
        std::string deviceDir = NUT_PART_STORE;
        shared::mkdir_if_needed( deviceDir.c_str() );
        std::ofstream cfgFile;
        log_debug("creating new config file %s/%s", NUT_PART_STORE, name );
        cfgFile.open(std::string(NUT_PART_STORE) + shared::path_separator() + name );
        cfgFile << *it;
        cfgFile.close();
        updateNUTConfig();
        return true;
    }
    return false;
}

bool Configurator::configure( const char *name, zhash_t *extendedAttributes, int8_t eventType )
{
    if( name || extendedAttributes || eventType ) { } // silence compiler warning
    log_debug("dummy configurator");
    return false;
}

Configurator * ConfigFactory::getConfigurator(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    log_debug("device type is %s", type.c_str() );
    if( type == "epdu" || type == "ups" ) {
        return new NUTConfigurator();
    }
    // if( type == "server" ) return ServerConfigurator();
    // if( type == "wheelbarrow" ) retrun WheelBarrowConfigurator();
    return new Configurator();
}

bool ConfigFactory::configureAsset(ymsg_t *message) {
    if( ! message ) return false;
    char *name = NULL;
    zhash_t *extAttributes = NULL;
    uint32_t type_id;
    int8_t event_type;
    bool result = false;

    int extract = bios_asset_extra_extract( message, &name, &extAttributes, &type_id, NULL, NULL, NULL, NULL, &event_type );
    log_debug("bios_asset_extra_extract result %i, device type %i", extract, type_id );
    if(
        extract  == 0 &&
        type_id == asset_type::DEVICE
    ) {
        // TODO: add subtype to asset message and decide according it
        // use something like deviceType = zhash_lookup(extAttributes,"subtype");
        const char *deviceType = "ups";
        // result = getConfigurator( deviceType ).configure( name, extAttributes, event_type );
        Configurator *C = getConfigurator( deviceType );
        C->configure( name, extAttributes, event_type );
        delete C;
    }
    FREE0(name);
    return result;
}
