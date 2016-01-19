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
 \file   ConfiguratorFactory.cc
 \brief  ConfiguratorFactory implementation
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include "log.h"
#include "preproc.h"
#include "NUTConfigurator.h" 

#include "ConfiguratorFactory.h"


Configurator* ConfiguratorFactory::getConfigurator (uint32_t type, uint32_t subtype)
{
    if (type == persist::asset_type::DEVICE &&
        (subtype == persist::asset_subtype::UPS || subtype == persist::asset_subtype::EPDU)) {
        return new NUTConfigurator();
    }
    // if( type == "server" ) return ServerConfigurator();
    // if( type == "wheelbarrow" ) retrun WheelBarrowConfigurator();
    return new Configurator();
}

bool ConfiguratorFactory::configureAsset (const std::string& name, AutoConfigurationInfo& info)
{
    log_debug ("Attempting to configure device name = '%s', type = '%" PRIu32 "', subtype = '%" PRIu32"'",
                name.c_str(), info.type, info.subtype);
    Configurator *C = getConfigurator (info.type, info.subtype);
    bool result = C->configure (name, info);
    delete C;
    return result;  
}

std::vector<std::string> ConfiguratorFactory::getNewRules (const std::string& name, AutoConfigurationInfo& info)
{
    log_debug("rules attempt device name %s type %" PRIu32 "/%" PRIu32, name.c_str(), info.type, info.subtype ); // ?
    Configurator *C = getConfigurator( info.type, info.subtype );
    std::vector<std::string> result = C->createRules (name);
    delete C;
    return result;
}

