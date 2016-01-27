/*
Copyright (C) 2014 Eaton
 
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
 \file   UptimeConfigurator.cc
 \brief  Configuration of uptime
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include <functional>
#include <exception>
#include <tntdb.h>
#include <tntdb/error.h>

#include "log.h"
#include "dbpath.h"
#include "asset_types.h"
#include "assets.h"

#include "UptimeConfigurator.h"

bool UptimeConfigurator::sendMessage (std::map <std::string, std::vector <std::string>>& dc_upses, mlm_client_t *client)
{
    if (!client)
        return false;

    zmsg_t *msg = zmsg_new ();
    if (!msg) {
        log_error ("zmsg_new () failed.");
        return false;
    }
    
    zmsg_addstrf (msg, "%s", "SET");
    for (const auto& it : dc_upses) {
        zmsg_addstrf (msg, "%s", it.first.c_str());
        for (const auto& it2 : it.second) {
            zmsg_addstrf (msg, "%s", it2.c_str());
        }
    }

    if (mlm_client_sendto (client, "uptime", "UPTIME", NULL, 1000, &msg) != 0) {
        zmsg_destroy (&msg);
        log_error ("mlm_client_sendto (address = '%s', subject = '%s', timeout = '1000') failed.",
            "uptime", "UPTIME");
        return false;
    }
    return true;
}

bool UptimeConfigurator::obtainData (std::map <std::string, std::vector <std::string>>& dc_upses)
{
    try {
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

        // select dcs
        db_reply <std::map <uint32_t, std::string> > reply =
            persist::select_short_elements (conn, persist::asset_type::DATACENTER, persist::asset_subtype::N_A);
        if (reply.status == 0) {
            log_error ("Cannot select datacenters");
            return false;
        }
        for (const auto& dc : reply.item ) {
            int rv = persist::select_assets_by_container (conn, dc.first, func);
            if (rv != 0) {
                log_error ("Cannot read upses for dc with id = '%" PRIu32"'", dc.first);
                continue;
            }
            dc_upses.emplace (dc.second, container_upses);
            container_upses.clear();
        }
        conn.close ();        
    }
    catch (const tntdb::Error& e) {
        log_error ("Database error: %s", e.what());
        return false;
    }
    catch (const std::exception& e) {
        log_error ("Error: %s", e.what());
        return false;
    }
    return true;
}

bool UptimeConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    log_debug ("UptimeConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
        name.c_str(), info.type, info.subtype);    
    // dc_name is mapped on the ups names
    std::map <std::string , std::vector<std::string> > dc_upses;

    if (!obtainData (dc_upses))
        return false;
    if (!sendMessage (dc_upses, client))
        return false;
    return true;
}

bool UptimeConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DATACENTER || info.subtype == persist::asset_subtype::UPS)
    {
        return true;
    }
    return false;
}
