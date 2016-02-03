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
#include <string>

#include <cxxtools/jsonserializer.h>
#include <cxxtools/jsondeserializer.h>
#include <tntdb.h>

#include "preproc.h"
#include "dbpath.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "utils++.h"
#include "asset_types.h"
#include "assets.h"
#include "utils_ymsg.h"
#include "filesystem.h"

#include "agent-autoconfig.h"

#define AUTOCONFIG "AUTOCONFIG"

using namespace persist;

const char* Autoconfig::StateFilePath = "/var/lib/bios/agent-autoconfig";
const char* Autoconfig::StateFile = "/var/lib/bios/agent-autoconfig/state";

static int
load_agent_info(std::string &info)
{
    if (shared::is_file (Autoconfig::StateFile)) {
        log_error ("not a file");
        info = "";
        return 0;
    }

    std::ifstream f(Autoconfig::StateFile, std::ios::in | std::ios::binary);
    if (f) {
        f.seekg (0, std::ios::end);
        info.resize (f.tellg ());
        f.seekg (0, std::ios::beg);
        f.read (&info[0], info.size());
        f.close ();
        return 0;
    }
    log_error("Fail to read '%s'", Autoconfig::StateFilePath);
    return -1;
}

static int
save_agent_info(const std::string& json)
{
    if (!shared::is_dir (Autoconfig::StateFilePath)) {
        zsys_error ("Can't serialize state, '%s' is not directory", Autoconfig::StateFilePath);
        return -1;
    }
    try {
        std::ofstream f(Autoconfig::StateFile);
        f.exceptions (~std::ofstream::goodbit);
        f << json;
        f.close();
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
    si.addMember("type") <<= std::to_string (info.type);
    si.addMember("subtype") <<= std::to_string (info.subtype);
    si.addMember("operation") <<= std::to_string (info.operation);
    si.addMember("configured") <<= info.configured;
    si.addMember("date") <<= std::to_string (info.date);
    si.addMember("attributes") <<= info.attributes;
}

inline void operator>>= (const cxxtools::SerializationInfo& si, AutoConfigurationInfo& info)
{
    std::string temp;
    si.getMember("configured") >>= info.configured;
    si.getMember("type") >>= temp;
    info.type = std::stoi (temp);
    si.getMember("subtype") >>= temp;
    info.subtype = std::stoi (temp);
    si.getMember("operation") >>= temp;
    info.operation = std::stoi (temp);
    si.getMember("date") >>= temp;
    info.date = std::stoi (temp);
    si.getMember("attributes")  >>= info.attributes;
}

void Autoconfig::main ()
{
    zsock_t *pipe = msgpipe();
    if (!pipe) {
        log_error ("msgpipe () failed");
        return;
    }

    zpoller_t *poller = zpoller_new (pipe, NULL);
    if (!poller) {
        log_error ("zpoller_new () failed");
        return;
    }

    _timestamp = zclock_mono ();
    while (!zsys_interrupted) {
        void *which = zpoller_wait (poller, _timeout);
        if (which == NULL) {
            if (zpoller_terminated (poller) || zsys_interrupted) {
                log_warning ("zpoller_terminated () or zsys_interrupted ()");
                break;
            }
            if (zpoller_expired (poller)) {
                onPoll ();
                _timestamp = zclock_mono ();
                continue;
            }
            _timestamp = zclock_mono ();
            log_warning ("zpoller_wait () returned NULL while at the same time zpoller_terminated == 0, zsys_interrupted == 0, zpoller_expired == 0");
            continue;
        }

        int64_t now = zclock_mono ();
        if (now - _timestamp >= _timeout) {
            onPoll ();
            _timestamp = zclock_mono ();
        }

        ymsg_t *message = recv ();
        if (!message) {
            log_warning ("recv () returned NULL; zsys_interrupted == '%s'; command = '%s', subject = '%s', sender = '%s'",
                    zsys_interrupted ? "true" : "false", command (), subject (), sender ());
            continue;
        }

        switch (ymsg_id (message)) {
            case YMSG_REPLY:
            {    
                onReply (&message);
                break;
            }
            case YMSG_SEND:
            {
                onSend (&message);
                break;
            }
            default:
            {
                log_warning ("Weird ymsg received, id = '%d', command = '%s', subject = '%s', sender = '%s'",
                        ymsg_id (message), command (), subject (), sender ());
                ymsg_destroy (&message);
            }
        }
    }
    zpoller_destroy (&poller);
}

void Autoconfig::onSend (ymsg_t **message)
{
    if (!message || ! *message)
       return;

    AutoConfigurationInfo info;
    char *device_name = NULL;

    zhash_t *extAttributes = NULL;

    if (bios_asset_extra_extract (*message, &device_name, &extAttributes, &info.type, &info.subtype, NULL, NULL, NULL, &info.operation) != 0) {
        log_debug("bios_asset_extra_extract () failed.");
        FREE0(device_name);
        return;
    }
    log_debug("Decoded asset message - device name = '%s', type = '%" PRIu32 "', subtype = '%" PRIu32"', operation = '%" PRIi8"'",
           device_name, info.type, info.subtype, info.operation);
    info.attributes = utils::zhash_to_map(extAttributes);
    _configurableDevices.emplace (std::make_pair (device_name, info));
    saveState ();
    setPollingInterval();
    FREE0(device_name);
}

void Autoconfig::onPoll( )
{
    bool save = false;

    for (auto& it : _configurableDevices) {
        if (it.second.configured) {
            continue;
        }

        bool device_configured = true;
        for (const auto& configurator : ConfiguratorFactory::getConfigurator (it.second)) {
            device_configured &= configurator->configure (it.first, it.second, client ());
        }
        if (device_configured) {
            log_debug ("Device '%s' configured successfully", it.first.c_str ());
            it.second.configured = true;
            save = true;
        }
        else {
            log_debug ("Device '%s' NOT configured yet.", it.first.c_str ());
        }
        it.second.date = zclock_mono ();
    }

    if (save) {
        cleanupState();
        saveState();
    }
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
    log_debug ("Size before cleanup '%zu'", _configurableDevices.size ());
    for( auto it = _configurableDevices.cbegin(); it != _configurableDevices.cend() ; ) {
        if( it->second.configured ) {
            _configurableDevices.erase(it++);
        } else {
            ++it;
        }
    }
    log_debug ("Size after cleanup '%zu'", _configurableDevices.size ());
}

void Autoconfig::saveState()
{
    std::ostringstream stream;
    cxxtools::JsonSerializer serializer(stream);
    log_debug ("size = '%zu'",_configurableDevices.size ());
    serializer.serialize( _configurableDevices );
    serializer.finish();
    std::string json = stream.str();
    log_debug (json.c_str ());
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
