/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file location_helpers.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <cstdio>
#include <cstring>
#include <exception>
#include <algorithm>
#include <list>
#include <tnt/http.h>

#include "asset_types.h"
#include "asset_msg.h"
#include "location_helpers.h"
#include "cleanup.h"

// Note: in the future, if more than one operation requires parsing these
//       strings on input, we should consider moving this to a general
//       helpers.cc file and rewrite these functions for a more general case
//       e.g. namespace shared:: src/include/utils.h src/shared/utils.c and
//       have asset_types in src/include.
//       There is no need to wrap a class around simple functions.

// TODO:case "api/v1/asset/5" (missing element type, i.e. room, datacenter...) not covered
int element_id (const std::string& from, int& element_id) {
    if (from.empty()) {
        return -1;
    }
    if (from.find ("api/v1/asset/") != 0) {
        return -1;
    }
    std::size_t last = from.find_last_of ("/");
    std::string number = from.substr (last + 1, from.length() - last);
    if (number.length () == 0) {
        return -1;
    }
    try {
        element_id = std::stoi (number);
    } catch (std::exception& e) {
        return -1;
    }
    return 0;
}

int asset_location_r(asset_msg_t** asset_msg, std::string& json) {
    int element_id = asset_msg_element_id(*asset_msg);
    int type_id = asset_msg_type(*asset_msg);
    std::string name = asset_msg_name(*asset_msg);
    std::string type_name = asset_msg_type_name(*asset_msg);


    json += "{"; 
    json += "\"name\" : \"" + name + "\", ";
    json += "\"id\" : \"" + std::to_string(element_id) + "\"";
    if (type_id == asset_type::DEVICE || type_id == asset_type::GROUP) { 
        json += ", \"type\" : \"" + std::string(asset_msg_type_name(*asset_msg)) + "\"";
    }

    std::list<zframe_t *> frames;
    std::list<std::string> names;

    frames.push_back(asset_msg_get_dcs(*asset_msg));
    names.push_back("datacenters");
    frames.push_back(asset_msg_get_rooms(*asset_msg));
    names.push_back("rooms");
    frames.push_back(asset_msg_get_rows(*asset_msg));
    names.push_back("rows");
    frames.push_back(asset_msg_get_racks(*asset_msg));
    names.push_back("racks");
    frames.push_back(asset_msg_get_devices(*asset_msg));
    names.push_back("devices");
    frames.push_back(asset_msg_get_grps(*asset_msg));
    names.push_back("groups");

    asset_msg_destroy(asset_msg);

    bool first_contains = true;
    _scoped_zframe_t *it_f = NULL;
    _scoped_zmsg_t *zmsg = NULL;
    _scoped_asset_msg_t *item = NULL;
    while(!(frames.empty() || names.empty())) {
        it_f = frames.front();
        frames.pop_front();
        std::string name_it = names.front();
        names.pop_front();
        if(it_f == NULL)
            continue;
        byte *buffer = zframe_data(it_f);
        if(buffer == NULL)
            goto err_cleanup;               
        zmsg = zmsg_decode(buffer, zframe_size(it_f));
        if(zmsg == NULL || !zmsg_is (zmsg))
            goto err_cleanup;               
        zframe_destroy(&it_f);

        _scoped_zmsg_t *pop = NULL;
        bool first = true;
        while((pop = zmsg_popmsg(zmsg)) != NULL) { // caller owns zmgs_t
            if(!is_asset_msg (pop))
                goto err_cleanup;
            item = asset_msg_decode(&pop); // zmsg_t is freed
            if(item == NULL)
                goto err_cleanup;
            if(first == false) {
                json += ", ";
            } else {
                if(first_contains == false) {
                    json += ", ";
                } else {
                    first_contains = false;
                    json += ", \"contains\" : { ";
                }
                json += "\"" + name_it + "\" : [";
                first = false;
            }
            if(asset_location_r(&item, json) != HTTP_OK)
                goto err_cleanup;               
            asset_msg_destroy(&item);
        }
        zmsg_destroy(&zmsg);

        if(first == false)
            json += "]";
    }
    if(!first_contains)
        json += "}"; // level-1 "contains"
    json += "}"; // json closing curly bracket
    return HTTP_OK;

err_cleanup:
    zmsg_destroy (&zmsg);
    asset_msg_destroy (&item);
    zframe_destroy(&it_f);
    while(!(frames.empty())) {
        it_f = frames.front();
        frames.pop_front();
        zframe_destroy(&it_f);
    }
    json = "";
    return HTTP_INTERNAL_SERVER_ERROR;
}

int asset (const std::string& from) {
    if (from.empty()) {
        return -1;
    }
    if (from.find ("api/v1/asset/") != 0) {
        return -1;
    }
    std::size_t pos = strlen ("api/v1/asset/");   
    std::string cut = from.substr (pos, from.length () - pos);
    if (cut.empty()) {
        return -1;
    }
    pos = cut.find_first_of ("/");
    if (pos == 0) {
        return -1;
    }
    cut = cut.substr(0, pos);

    int ret = asset_type::UNKNOWN;
    if (cut == "datacenter") {
        ret = asset_type::DATACENTER;
    } else if (cut == "room") {
        ret = asset_type::ROOM;
    } else if (cut == "row") {
        ret = asset_type::ROW;
    } else if (cut == "rack") {
        ret = asset_type::RACK;
    } else if (cut == "group") {
        ret = asset_type::GROUP;
    } else if (cut == "device") {
        ret = asset_type::DEVICE;
    }
    return ret;
}

