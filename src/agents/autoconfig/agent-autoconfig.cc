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
#include <fstream>

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <tntdb.h>
#include <preproc.h>

#include "dbpath.h"
#include "agent-autoconfig.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "utils++.h"
#include "asset_types.h"
#include "assets.h"
#include "utils_ymsg.h"
#include "filesystem.h"

#define AUTOCONFIG "AUTOCONFIG"

using namespace persist;

// cxxtool serialization operators
static const char* PATH = "/var/lib/bios/agent-autoconfig";
static const char* STATE = "/var/lib/bios/agent-autoconfig/state";

static int
load_agent_info(std::string &info)
{
    if (shared::is_file (STATE))
    {
        try {
            std::fstream f{STATE};
            f >> info;
            return 0;
        }
        catch (const std::exception& e)
        {
            log_error("Fail to read '%s', %s", PATH, e.what());
            return -1;
        }
    }
    info = "";
    return 0;
}

static int
save_agent_info(const std::string& json)
{
    if (!shared::is_dir (PATH)) {
        zsys_error ("Can't serialize state, '%s' is not directory", PATH);
        return -1;
    }
    try {
        std::fstream f{STATE};
        f << json;
    }
    catch (const std::exception& e) {
        zsys_error ("Can't serialize state, %s", e.what());
        return -1;
    }
    return 0;
}

inline void operator<<= (cxxtools::SerializationInfo& si, const AutoConfigurationInfo& info)
{
    si.setTypeName("AutoConfigurationInfo");
    // serializing integer doesn't work for unknown reason
    si.addMember("type") <<= std::to_string(info.type);
    si.addMember("subtype") <<= std::to_string(info.subtype);
    si.addMember("operation") <<= std::to_string(info.operation);
    si.addMember("configured") <<= info.configured;
    si.addMember("attributes") <<= info.attributes;
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
    si.getMember("attributes")  >>= info.attributes;
}

// autoconfig agent public methods

void Autoconfig::onStart( )
{
    loadState();
    setPollingInterval();
}

// MVY: this is huge hack - needs to be better integrated to aoutconfig
// // TROWS!!!!!!!!!!!!!!!!!!
static void
    s_configure_uptime_agent()
{
    tntdb::Connection conn = tntdb::connectCached (url);

    std::vector <std::string> container_upses{};
    std::function<void(const tntdb::Row&)> func = \
            [&container_upses](const tntdb::Row& row)
            {
                a_elmnt_tp_id_t type_id = 0;
                row["type_id"].get(type_id);
                
                std::string device_type_name = "";
                row["subtype_name"].get(device_type_name);

                if ( ( type_id == persist::asset_type::DEVICE ) && ( device_type_name == "ups" ) )
                {
                    std::string device_name = "";
                    row["name"].get(device_name);

                    container_upses.push_back(device_name);
                }
            };

    // dc_name is mapped on the ups names
    std::map <std::string , std::vector<std::string> > dc_upses;
    // select dcs
    db_reply <std::map <uint32_t, std::string> > reply = 
        persist::select_short_elements 
        (conn, persist::asset_type::DATACENTER, persist::asset_subtype::N_A);
    if ( reply.status == 0 ) {
        zsys_error ("Cannot select datacenters");
        return;
    }
    for ( const auto dc : reply.item ) {
        int rv = select_assets_by_container (conn, dc.first, func);
        if ( rv != 0 ) {
            zsys_error ("Cannot read upses for dc with is %" PRIu32, dc.first);
            continue;
        }
        dc_upses.emplace (dc.second, container_upses);
        container_upses.clear();
    }

    conn.close ();

    mlm_client_t *client = mlm_client_new ();
    if (!client) {
        log_error ("Fail to call mlm_client_new");
        return;
    }

    mlm_client_connect (client, "ipc://@/malamute", 1000, "agent-autoconfig");
    zmsg_t *msg = zmsg_new ();
    if (!msg) {
        mlm_client_destroy (&client);
        log_error ("Fail to allocate new zmsg_t");
        return;
    }

    zmsg_addstrf (msg, "%s", "SET");
    for (const auto &it :dc_upses) {
        zmsg_addstrf (msg, "%s", it.first.c_str());
        for (const auto &it2 : it.second) {
            zmsg_addstrf (msg, "%s", it2.c_str());
        }
    }

    mlm_client_sendto (client, "uptime", "UPTIME", NULL, 1000, &msg);
    mlm_client_destroy (&client);

    return;
}

void Autoconfig::onSend( ymsg_t **message )
{
    if( ! message || ! *message ) return;

    char *device_name = NULL;
    uint32_t type = 0;
    uint32_t subtype = 0;
    int8_t operation;
    zhash_t *extAttributes = NULL;

    int extract = bios_asset_extra_extract( *message, &device_name, &extAttributes, &type, &subtype, NULL, NULL, NULL, &operation );
    if( ! extract ) {
        log_debug("asset message for %s type %" PRIu32 " subtype %" PRIu32, device_name, type, subtype );
        if( type == asset_type::DEVICE && ( subtype == asset_subtype::UPS || subtype == asset_subtype::EPDU ) ) {
            // this is a device that we should configure, we need extended attributes (ip.1 particulary)
            addDeviceIfNeeded( device_name, type, subtype );
            _configurableDevices[device_name].configured = false;
            _configurableDevices[device_name].attributes.clear();
            _configurableDevices[device_name].operation = operation;
            _configurableDevices[device_name].attributes = utils::zhash_to_map(extAttributes);
            saveState();
            setPollingInterval();
        }
        //MVY: hack for uptime calculation
        if (type == asset_type::DATACENTER || type == asset_subtype::UPS)
        {
            s_configure_uptime_agent ();
        }
    } else {
        log_debug("this is not bios_asset message (error %i)", extract);
    }
    FREE0( device_name );
}

void Autoconfig::sendNewRules(std::vector<std::string> const &rules) {
    for( const auto &rule: rules ) {
        log_debug("Sending new lua rule: %s", rule.c_str ());
        zmsg_t *message = zmsg_new ();
        zmsg_addstr(message, "ADD");
        zmsg_addstr(message, rule.c_str());
        if( mlm_client_sendto( bios_agent_client( _bios_agent ), BIOS_AGENT_NAME_ALERT_AGENT, "rfc-evaluator-rules", NULL, 5000, &message) != 0) {
            zsys_error("failed to send new rule to alert-agent");
        }
        zmsg_destroy (&message);   
    }
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
                auto factory = ConfigFactory();
                if( factory.configureAsset (it.first, it.second)) {
                    sendNewRules (factory.getNewRules (it.first, it.second));
                    it.second.configured = true;
                    save = true;
                }
                it.second.date = time(NULL);
            }
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
    ymsg_t *extended = bios_asset_extra_encode( name, NULL, 0, 0, 0, NULL, 0, 0 );
    if( extended ) {
        log_debug("requesting extended attributes for %s", name);
        sendto(BIOS_AGENT_NAME_DB_MEASUREMENT,"get_asset_extra", &extended );
        ymsg_destroy( &extended );
    }
}

void Autoconfig::loadState()
{
    std::string json = "";
    int rv = load_agent_info(json);
    if ( rv != 0 || json.empty() )
        return;

    std::istringstream in(json);

    try {
        _configurableDevices.clear();
        cxxtools::JsonDeserializer deserializer(in);
        deserializer.deserialize(_configurableDevices);
    } catch( std::exception &e ) {
        log_error( "can't parse state: %s", e.what() );
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
    save_agent_info(json );
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
