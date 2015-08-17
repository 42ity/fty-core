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

#include "agent-autoconfig.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "asset_types.h"
#include "utils_ymsg.h"

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <preproc.h>

// FIXME: move to shared

std::map<std::string,std::string> zhash_to_map(zhash_t *hash)
{
    std::map<std::string,std::string> map;
    char *item = (char *)zhash_first(hash);
    while(item) {
        const char * key = zhash_cursor(hash);
        const char * val = (const char *)zhash_lookup(hash,key);
        if( key && val ) map[key] = val;
        item = (char *)zhash_next(hash);
    }
    return map;
}

inline void operator<<= (cxxtools::SerializationInfo& si, const AutoConfigurationInfo& info)
{
    si.setTypeName("AutoConfigurationInfo");
    // si.setName(info.name);
    si.addMember("name") <<= info.name;
    si.addMember("type") <<= info.type;
    si.addMember("configured") <<= info.configured;
    si.addMember("date") <<= std::to_string(info.date); // there are some issues on (int) serialization
}

inline void operator>>= (const cxxtools::SerializationInfo& si, AutoConfigurationInfo& info)
{
    si.getMember("name") >>= info.name;
    si.getMember("type") >>= info.type;
    si.getMember("configured") >>= info.configured;
    {
        std::string date;
        si.getMember("date") >>= date;
        info.date = atoi(date.c_str());
    }
}

void Autoconfig::loadState()
{
    std::string json = R"#([{"first":"mydevice","second":{"name":"mydevice","type":"ups","configured":false, "date":"42"}}])#";

    std::istringstream in(json);

    try {
        _configurableDevices.clear();
        cxxtools::JsonDeserializer deserializer(in);
        std::vector<AutoConfigurationInfo> items;
        deserializer.deserialize(_configurableDevices);
    } catch( std::exception &e ) {
        log_error( "Can't load state from database: %s", e.what() );
    }
    // log
    for( auto a : _configurableDevices) {
        std::cout << a.first << " " << a.second.type << "\n";
    }
}

void Autoconfig::saveState()
{
    std::ostringstream stream;
    cxxtools::JsonSerializer serializer(stream);

    serializer.serialize( _configurableDevices ).finish();
    std::cout <<">"<< stream.str() << "<\n";
}

void Autoconfig::onStart( )
{
    loadState();
}

void Autoconfig::onReply( ymsg_t **message )
{
    if( ! message || ! *message ) return;
    char *name = NULL;
    zhash_t *extAttributes = NULL;
    uint32_t type_id;
    int8_t event_type;

    int extract = bios_asset_extra_extract( *message, &name, &extAttributes, &type_id, NULL, NULL, NULL, NULL, &event_type );
    log_debug("bios_asset_extra_extract result %i, device type %i", extract, type_id );
    if(
        extract  == 0 &&
        type_id == asset_type::DEVICE
    ) {
        // TODO: add subtype to asset message and decide according it
        // use something like deviceType = zhash_lookup(extAttributes,"subtype");
        AutoConfigurationInfo device = { .name = name, .type = "ups", .configured = false, .date = 0, .attributes = zhash_to_map(extAttributes) };
         _configurableDevices[name] = device;
        /*
        Configurator *C = getConfigurator( deviceType );
        C->configure( name, extAttributes, event_type );
        delete C;
        */
    }
    FREE0(name);

    //ConfigFactory().configureAsset( *message );
}

void Autoconfig::onSend( ymsg_t **message )
{
    if( ! message || ! *message ) return;

    char *device_name = NULL;
    uint32_t type = 0;
    uint32_t subtype = 0;
    int8_t operation;

    int x;
    if( (x = bios_asset_extract( *message, &device_name, &type, &subtype, NULL, NULL, NULL, &operation )) == 0 ) {
        log_debug("Asset message for %s type %" PRIu32, device_name, type );
        if( type == asset_type::DEVICE ) {
            ymsg_t *extended = bios_asset_extra_encode( device_name, NULL, 0, 0, NULL, 0, 0, operation );
            if( extended ) {
                log_debug("Sending bios_asset_extra_encode");
                sendto(BIOS_AGENT_NAME_DB_MEASUREMENT,"get_asset_extra", &extended );
                ymsg_destroy( &extended );
            }
        }
    } else {
        log_debug("This is not bios_asset message (error %i)", x);
    }
    FREE0( device_name );
}

void Autoconfig::onPoll( )
{
    _timeout = 0;
    for( auto &it : _configurableDevices) {
        if( ! it.second.configured ) {
            

        }
    }
}

int main( UNUSED_PARAM int argc, UNUSED_PARAM char *argv[] )
{
    int result = 1;
    log_open();
    log_info ("autoconfig agent started");
    Autoconfig agent("AUTOCONFIG");
    if( agent.connect( MLM_ENDPOINT, bios_get_stream_main(), "^configure@" ) ) {
        result = agent.run();
    }
    log_info ("autoconfig agent exited with code %i\n", result);
    return result;
}
