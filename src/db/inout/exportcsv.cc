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
    for (uint32_t i = max_power_links; i != 0; i--) {
        std::string si = std::to_string(i);
        KEYTAGS.insert(KEYTAGS.begin(), "power_input." + si);
        KEYTAGS.insert(KEYTAGS.begin(), "power_plug_src." + si);
        KEYTAGS.insert(KEYTAGS.begin(), "power_source." + si);
    }

    uint32_t max_groups = 2;
    for (uint32_t i = max_groups; i != 0; i--) {
        KEYTAGS.push_back("group." + std::to_string(i));
    }

    // put all remaining keys from the database
    s_update_keytags(conn, KEYTAGS);

    // 0. print the row with headers
    for (const auto& k : ASSET_ELEMENT_KEYTAGS) {
        std::cout << k << ",";
    }
    for (const auto& k : KEYTAGS) {
        std::cout << k << ",";
    }
    std::cout << std::endl;

    std::function<void(const tntdb::Row&)>
        process_v_web_asset_element_row \
        = [&conn, &KEYTAGS](const tntdb::Row& r)
    {
        a_elmnt_id_t id;
        r["id"].get(id);

        a_elmnt_id_t id_parent;
        r["id_parent"].get(id_parent);

        // 2.) select * from t_bios_asset_ext_attributes

        std::map <std::string, std::pair<std::string, bool> > ext_attrs;
        select_ext_attributes(conn, id, ext_attrs);

        //parent name
        auto dbreply = select_asset_element_web_byId(
                conn,
                id_parent);
        std::string location;
        if (dbreply.status == 1)
            location = dbreply.item.name;

        /* TODO TODO TODO TODO */
        // 3.) links
        // auto location_to = select_asset_device_

        // 4.) select * from v_bios_asset_link

        // 5.) PRINT IT
        // 5.1)     things from asset element table itself
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
        std::cout << priority << ",";

        std::string business_critical;
        r["business_crit"].get(business_critical);
        std::cout << business_critical << ",";

        }

        // 5.2 other keytags
        for (const auto& k : KEYTAGS) {
            std::cout << ",";
        }
        std::cout << std::endl;

    };


    // for each row from v_web_asset_element do ...
    select_asset_element_all(
            conn,
            process_v_web_asset_element_row);


}

} // namespace persist
