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
 * \file asset_computed_impl.cc
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \brief Not yet documented file
 */

#include <string>
#include <map>
#include <sstream>

#include <tnt/http.h>
#include <tntdb/connection.h>
#include <cxxtools/regex.h>
#include <cxxtools/char.h>
#include <cxxtools/jsonformatter.h>

#include "asset_computed_impl.h"
#include "shared/dbtypes.h"
#include "shared/utils.h"
#include "dbpath.h"
#include "db/assets.h"
#include "asset_types.h"
#include "assetcrud.h"
#include "web_utils.h"
#include "log.h"

/*!
 * \brief trim string and convert it to int
 *
 * \param usize must be number, otherwise returns 0
 */
int usize_to_int(const std::string &usize) {

    static const cxxtools::Regex re{"^[0-9]+$"};

    if( usize.empty() || ! re.match(usize)) return 0;

    uint32_t size = string_to_uint32( usize.c_str() );
    if( size == UINT32_MAX ) size = 0;
    return size;
}

int free_u_size( uint32_t elementId, std::string &jsonResult)
{
    int freeusize = 0;
    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
        auto rack = persist::select_asset_element_web_byId( conn, elementId );

        if( rack.status && ( rack.item.type_id == persist::asset_type::RACK ) ){
            auto ext = persist::select_ext_attributes( conn, elementId );
            auto us = ext.item.find("u_size");
            if( us == ext.item.end() ) {
                // not size
                jsonResult = create_error_json( "Rack doesn't have u_size specified", 103 );
                return HTTP_BAD_REQUEST;
            }
            freeusize = usize_to_int( us->second.first );
            log_debug( "rack size is %i", freeusize );

            // get devices inside the rack
            auto devices = persist::select_asset_device_by_container(conn, elementId);
            if( ! devices.status ) {
                jsonResult = create_error_json( "Error reading rack content", 104 );
                return HTTP_BAD_REQUEST;
            }
            /*
              select usize for elements
            */
            jsonResult = std::string("{ \"id\":") + std::to_string(elementId) + ", \"freeusize\":" + std::to_string(freeusize) + " }" ;
            return HTTP_OK;
        } else {
            // this is not rack
            jsonResult = create_error_json( "Specified asset element is not rack", 105 );
            return HTTP_BAD_REQUEST;
        }
    } catch(std::exception &e) {
        jsonResult = create_error_json( e.what(), 105 );
        return HTTP_INTERNAL_SERVER_ERROR;
    }
}

static uint32_t
s_select_outlet_count(
        tntdb::Connection &conn,
        a_elmnt_id_t id)
{
    static const char* KEY = "outlet.count";

    std::map <std::string, std::pair<std::string, bool> > res;
    int ret = persist::select_ext_attributes(conn, id, res);

    if (ret != 0 || res.count(KEY) == 0)
        return UINT32_MAX;

    std::string foo = res.at(KEY).first.c_str();
    auto dot_i = foo.find('.');
    if (dot_i != std::string::npos) {
        foo.erase(dot_i, foo.size() - dot_i);
    }

    return string_to_uint32(foo.c_str());
}

static bool
s_is_rack(
        tntdb::Connection &conn,
        a_elmnt_id_t id)
{
    auto res = persist::select_asset_element_web_byId(conn, id);
    return res.status == 1 && res.item.type_name == "rack";
}

int
rack_outlets_available(
        uint32_t elementId,
        std::map<std::string, int> &res,
        std::string &errmsg,
        int &errcode)
{
    int sum = -1;
    bool tainted = false;
    tntdb::Connection conn;

    std::function<void(const tntdb::Row &row)> cb = \
        [&conn, &sum, &tainted, &res](const tntdb::Row &row)
        {
            a_elmnt_id_t device_subtype = 0;
            row["subtype"].get(device_subtype);
            if (!persist::is_epdu(device_subtype) && !persist::is_pdu(device_subtype))
                return;

            a_elmnt_id_t device_asset_id = 0;
            row["asset_id"].get(device_asset_id);

            uint32_t foo = s_select_outlet_count(conn, device_asset_id);
            int outlet_count = foo != UINT32_MAX ? foo : -1;

            int outlet_used = persist::count_of_link_src(conn, device_asset_id);
            if (outlet_used == -1)
                outlet_count = -1;
            else
                outlet_count -= outlet_used;

            if (outlet_count < 0)
                tainted = true;
            else
                sum += outlet_count;
            res[std::to_string(device_asset_id)] = outlet_count;
        };

    try{
        conn = tntdb::connectCached(url);
        if (!s_is_rack(conn, elementId)) {
            errmsg = "Asset element with id '" + std::to_string(elementId) + "' does not exist or is not rack";
            errcode = 105;
            return HTTP_BAD_REQUEST;
        }

        persist::select_asset_device_by_container(
                conn, elementId, cb);

    } catch (std::exception &e) {
        errmsg = e.what();
        errcode = 105;
        return HTTP_BAD_REQUEST;
    }

    if (tainted || sum == -1)
        res["sum"] = -1;
    else
        res["sum"] = sum + 1;   //sum is initialized to -1

    return HTTP_OK;
}
