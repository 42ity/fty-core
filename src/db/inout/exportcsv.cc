/*
Copyright (C) 2014-2015 Eaton
 
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

/*! \file   exportcsv.cc
    \brief  Export assets via csv
    \author Michal Vyskocil <michalvyskocil@eaton.com>
*/

#include <iostream>
#include <algorithm>

#include "tntdb/row.h"
#include "db/assets.h"
#include "dbpath.h"
#include "log.h"

namespace persist {

// names of element from v_web_asset_element - they'll be printed as the first ones
static std::vector<std::string> ASSET_ELEMENT_KEYTAGS{
    "id", "name", "type", "sub_type", "location", "status", "priority", "business_critical"};

// get all keytags available in the system and update the s argument
static void
s_update_keytags(
        tntdb::Connection& conn,
        std::vector<std::string>& s) {

    // gcc 4.8 did not allowed me to declare the lamda in func call!
    std::function<void(const tntdb::Row&)> \
        foo = [&s](const tntdb::Row& r)
        {
            std::string keytag;
            r["keytag"].get(keytag);
            if (std::find(s.cbegin(), s.cend(), keytag) == s.end())
                s.push_back(keytag);
        };

    select_ext_attributes_keytags(
            conn,
            foo);
}

void
    export_asset_csv
        (void)
{
    // 0.) tntdb connection
    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
    }
    catch(...)
    {
        std::string msg{"no connection to database"};
        log_error("%s", msg.c_str());
        LOG_END;
        throw std::runtime_error(msg.c_str());
    }

    // TODO: move somewhere else
    std::vector<std::string> KEYTAGS = {
        "description", "IP.1", "company", "site_name", "region", "country", "address",
        "contact_name", "contact_email", "contact_phone", "maximum_number_racks", "u_size",
        "manufacturer", "model", "serial_no", "runtime", "phase", "installation_date",
        "maintenance_date", "maintenance_due", "service_contact_name", "service_contact_mail",
        "service_contact_phone", "location_front_pos", "location_rear_pos", "location_u_pos",
        "location_w_pos", "location_d_pos", "end_warranty_date", "battery_type",
        "battery_installation_date", "battery_maintenance_date", "hostname.1", "http_link.1"
    };

    // TODO: get number of power links!!!
    uint32_t max_power_links = 3;
    uint32_t max_groups = 2;


    // put all remaining keys from the database
    s_update_keytags(conn, KEYTAGS);

    // 1 print the first row with names
    // 1.1      names from asset element table itself
    for (const auto& k : ASSET_ELEMENT_KEYTAGS) {
        std::cout << k << ",";
    }
    
    // 1.2      print power links
    for (uint32_t i = 0; i != max_power_links; i++) {
        std::string si = std::to_string(i+1);
        std::cout << "power_source."   + si << ",";
        std::cout << "power_plug_src." + si << ",";
        std::cout << "power_input."    + si << ",";
    }

    // 1.3      print extended attributes
    for (const auto& k : KEYTAGS) {
        std::cout << k << ",";
    }

    // 1.4      print groups
    for (uint32_t i = 0; i != max_groups; i++) {
        std::string si = std::to_string(i+1);
        std::cout << "group."   + si << ",";
    }
    std::cout << std::endl;

    // 2. FOR EACH ROW from v_web_asset_element / t_bios_asset_element do ...
    std::function<void(const tntdb::Row&)>
        process_v_web_asset_element_row \
        = [&conn, &KEYTAGS, max_power_links, max_groups](const tntdb::Row& r)
    {
        a_elmnt_id_t id = 0;
        r["id"].get(id);

        a_elmnt_id_t id_parent = 0;
        r["id_parent"].get(id_parent);

        // 2.1      select all extended attributes

        std::map <std::string, std::pair<std::string, bool> > ext_attrs;
        select_ext_attributes(conn, id, ext_attrs);

        // 2.2      get name of parent
        auto dbreply = select_asset_element_web_byId(
                conn,
                id_parent);
        std::string location;
        if (dbreply.status == 1)
            location = dbreply.item.name;

        /* TODO TODO TODO TODO */
        // 2.3 links
        // auto location_to = select_asset_device_
        //
        // 3.4 groups

        // 2.5      PRINT IT
        // 2.5.1    things from asset element table itself
        {
        std::cout << id << ",";

        std::string name;
        r["name"].get(name);
        std::cout << name << ",";

        std::string type_name;
        r["type_name"].get(type_name);
        std::cout << type_name << ",";

        std::string subtype_name;
        r["subtype_name"].get(subtype_name);
        std::cout << subtype_name << ",";

        std::cout << location << ",";

        std::string status;
        r["status"].get(status);
        std::cout << status << ",";

        uint32_t priority;
        r["priority"].get(priority);
        std::cout << "P" << priority << ",";

        uint32_t business_critical = 0;
        r["business_crit"].get(business_critical);
        std::cout << (business_critical == 0 ? "no" : "yes") << ",";
        }

        // 2.5.2        power location
        for (uint32_t i = 0; i != max_power_links; i++) {
            std::cout << ",";
            std::cout << ",";
            std::cout << ",";
        }

        // 2.5.3        extended attributes
        for (const auto& k : KEYTAGS) {
            if (ext_attrs.count(k) == 1)
                std::cout << ext_attrs[k].first << ",";
            else
                std::cout << ",";
        }
        
        // 2.5.4        groups
        for (uint32_t i = 0; i != max_groups; i++) {
            std::cout << ",";
        }
        std::cout << std::endl;

    };

    select_asset_element_all(
            conn,
            process_v_web_asset_element_row);


}

} // namespace persist
