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

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <preproc.h>

#include "agent-autoconfig.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "utils++.h"
#include "asset_types.h"
#include "utils_ymsg.h"
#include "db/agentstate.h"

#define AUTOCONFIG "AUTOCONFIG"

using namespace asset_type;
using namespace persist;

// cxxtool serialization operators

inline void operator<<= (cxxtools::SerializationInfo& si, const AutoConfigurationInfo& info)
{
    si.setTypeName("AutoConfigurationInfo");
    // serializing integer doesn't work for unknown reason
    si.addMember("type") <<= std::to_string(info.type);
    si.addMember("subtype") <<= std::to_string(info.subtype);
    si.addMember("operation") <<= std::to_string(info.operation);
    si.addMember("configured") <<= info.configured;
}

inline void operator>>= (const cxxtools::SerializationInfo& si, AutoConfigurationInfo& info)
{
    si.getMember("configured") >>= info.configured;
    {
        // serializing integer doesn't work
        std::string tmp;
        si.getMember("type") >>= tmp;
        info.type = atoi(tmp.c_str());

        si.getMember("subtype") >>= tmp;
        info.subtype = atoi(tmp.c_str());

        si.getMember("operation") >>= tmp;
        info.operation = atoi(tmp.c_str());
    }
}

// autoconfig agent public methods

void Autoconfig::onStart( )
{
    loadState();
    setPollingInterval();
}

void Autoconfig::onReply( ymsg_t **message )
{
    if( ! message || ! *message ) return;
    char *name = NULL;
    zhash_t *extAttributes = NULL;
    uint32_t type, subtype;
    int8_t event_type;

    int extract = bios_asset_extra_extract( *message, &name, &extAttributes, &type, &subtype, NULL, NULL, NULL, &event_type );
    log_debug("received extended attributes for device %s, device type %" PRIu32 "/%" PRIu32 ", extract result %i",
              name, type, subtype, extract );
    if(
        extract  == 0 &&
        _configurableDevices.find(name) != _configurableDevices.end()
    ) {
        _configurableDevices[name].attributes = utils::zhash_to_map(extAttributes);
    }
    FREE0(name);
}

void Autoconfig::onSend( ymsg_t **message )
{
    if( ! message || ! *message ) return;

    char *device_name = NULL;
    uint32_t type = 0;
    uint32_t subtype = 0;
    int8_t operation;

    int extract = bios_asset_extract( *message, &device_name, &type, &subtype, NULL, NULL, NULL, &operation );
    if( ! extract ) {
        log_debug("asset message for %s type %" PRIu32 " subtype %" PRIu32, device_name, type, subtype );
        if( type == asset_type::DEVICE && ( subtype == asset_subtype::UPS || subtype == asset_subtype::EPDU ) ) {
            // this is a device that we should configure, we need extended attributes (ip.1 particulary)
            addDeviceIfNeeded( device_name, type, subtype );
            _configurableDevices[device_name].configured = false;
            _configurableDevices[device_name].attributes.clear();
            _configurableDevices[device_name].operation = operation;
            saveState();
            setPollingInterval();
            requestExtendedAttributes( device_name );
        }
    } else {
        log_debug("this is not bios_asset message (error %i)", extract);
    }
    FREE0( device_name );
}

void Autoconfig::onPoll( )
{
    bool save = false;
    
    for( auto &it : _configurableDevices) {
        // check not configured devices
        if( ! it.second.configured ) {
            // we don't need extended attributes for deleting configuration
            // but we need them for update/insert
            if(
                ! it.second.attributes.empty() ||
                it.second.operation == asset_operation::DELETE ||
                it.second.operation == asset_operation::RETIRE
            )
            {
                if( ConfigFactory().configureAsset( it.first, it.second ) ) {
                    it.second.configured = true;
                    save = true;
                }
                it.second.date = time(NULL);
            }
        }
        // get extended attributes for not configured device
        if(
            ! it.second.configured &&
            it.second.attributes.empty() &&
            ( it.second.operation == asset_operation::INSERT ||
              it.second.operation == asset_operation::UPDATE )
        )
        {
            requestExtendedAttributes( it.first.c_str() );
        }
    }
    if( save ) { cleanupState(); saveState(); }
    setPollingInterval();
}

// autoconfig agent private methods

void Autoconfig::setPollingInterval( )
{
    _timeout = -1;
    for( auto &it : _configurableDevices) {
        if( ! it.second.configured ) {
            if( it.second.date == 0 ) {
                // there is device that we didn't try to configure
                // let's try to do it soon
                _timeout = 5000;
                return;
            } else {
                // we failed to configure some device
                // let's try after one minute again 
                _timeout = 60000;
            }
        }
    }
}

void Autoconfig::addDeviceIfNeeded(const char *name, uint32_t type, uint32_t subtype) {
    if( _configurableDevices.find(name) == _configurableDevices.end() ) {
        AutoConfigurationInfo device;
        device.type = type;
        device.subtype = subtype;
        _configurableDevices[name] = device;
    }
}

void Autoconfig::requestExtendedAttributes( const char *name )
{
    ymsg_t *extended = bios_asset_extra_encode( name, NULL, 0, 0, NULL, 0, 0, 0 );
    if( extended ) {
        log_debug("requesting extended attributes for %s", name);
        sendto(BIOS_AGENT_NAME_DB_MEASUREMENT,"get_asset_extra", &extended );
        ymsg_destroy( &extended );
    }
}

void Autoconfig::loadState()
{
    std::string json = load_agent_info( AUTOCONFIG );
    std::istringstream in(json);

    try {
        _configurableDevices.clear();
        cxxtools::JsonDeserializer deserializer(in);
        std::vector<AutoConfigurationInfo> items;
        deserializer.deserialize(_configurableDevices);
    } catch( std::exception &e ) {
        log_error( "can't load state from database: %s", e.what() );
    }
}

void Autoconfig::cleanupState()
{
    for( auto it = _configurableDevices.cbegin(); it != _configurableDevices.cend() ; ) {
        if( it->second.configured ) {
            _configurableDevices.erase(it++);
        } else {
            ++it;
        }
    }
}

void Autoconfig::saveState()
{
    std::ostringstream stream;
    cxxtools::JsonSerializer serializer(stream);

    serializer.serialize( _configurableDevices ).finish();
    std::string json = stream.str();
    save_agent_info( AUTOCONFIG, json );
}

int main( UNUSED_PARAM int argc, UNUSED_PARAM char *argv[] )
{
    int result = 1;
    log_open();
    log_info ("autoconfig agent started");
    Autoconfig agent( AUTOCONFIG );
    if( agent.connect( MLM_ENDPOINT, bios_get_stream_main(), "^configure@" ) ) {
        result = agent.run();
    }
    log_info ("autoconfig agent exited with code %i\n", result);
    return result;
}
