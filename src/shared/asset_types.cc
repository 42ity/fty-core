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
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 */
#include "asset_types.h"

#include <algorithm>

namespace persist {

a_elmnt_tp_id_t
    type_to_typeid
        (const std::string &type)
{
    std::string t (type);
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    if(t == "datacenter") {
        return asset_type::DATACENTER;
    } else if(t == "room") {
        return asset_type::ROOM;
    } else if(t == "row") {
        return asset_type::ROW;
    } else if(t == "rack") {
        return asset_type::RACK;
    } else if(t == "group") {
        return asset_type::GROUP;
    } else if(t == "device") {
        return asset_type::DEVICE;
    } else
        return asset_type::TUNKNOWN;
}

std::string
    typeid_to_type
        (a_elmnt_tp_id_t type_id)
{
    switch(type_id) {
        case asset_type::DATACENTER:
            return "datacenter";
        case asset_type::ROOM:
            return "room";
        case asset_type::ROW:
            return "row";
        case asset_type::RACK:
            return "rack";
        case asset_type::GROUP:
            return "group";
        case asset_type::DEVICE:
            return "device";
        default:
            return "unknown";
    }
}

a_elmnt_stp_id_t
    subtype_to_subtypeid
        (const std::string &subtype)
{
    std::string st(subtype);
    std::transform(st.begin(), st.end(), st.begin(), ::tolower);
    if(st == "ups") {
        return asset_subtype::UPS;
    } else if(st == "genset") {
        return asset_subtype::GENSET;
    } else if(st == "epdu") {
        return asset_subtype::EPDU;
    } else if(st == "server") {
        return asset_subtype::SERVER;
    } else if(st == "pdu") {
        return asset_subtype::PDU;
    } else if(st == "main") {
        return asset_subtype::MAIN;
    } else if(st == "sts") {
        return asset_subtype::STS;
    } else if(st == "switch") {
        return asset_subtype::SWITCH;
    } else if(st == "storage") {
        return asset_subtype::STORAGE;
    } else if(st == "N_A") {
        return asset_subtype::N_A;
    } else if(st == "") {
        return asset_subtype::N_A;
    } else
        return asset_subtype::SUNKNOWN;
}

std::string
    subtypeid_to_subtype
        (a_elmnt_tp_id_t subtype_id)
{
    switch(subtype_id) {
        case asset_subtype::UPS:
            return "ups";
        case asset_subtype::GENSET:
            return "genset";
        case asset_subtype::STORAGE:
            return "storage";
        case asset_subtype::STS:
            return "sts";
        case asset_subtype::MAIN:
            return "main";
        case asset_subtype::EPDU:
            return "epdu";
        case asset_subtype::PDU:
            return "pdu";
        case asset_subtype::SERVER:
            return "server";
        case asset_subtype::SWITCH:
            return "switch";
        case asset_subtype::N_A:
            return "";
        default:
            return "unknown";
    }
}

}; // namesace end
