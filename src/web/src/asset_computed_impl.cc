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

#include <tntdb/connection.h>
#include <cxxtools/regex.h>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/jsonserializer.h>

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

int free_u_size( const std::string &element, std::string &jsonResult)
{
    int freeusize = 0;
    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
        a_elmnt_id_t elementId =  string_to_uint32( element.c_str() );
        //TODO: if UINT32_MAX
        auto rack = persist::select_asset_element_web_byId( conn, elementId );

        if( rack.status && ( rack.item.type_id == persist::asset_type::RACK ) ){
            auto ext = persist::select_ext_attributes( conn, elementId );
            auto us = ext.item.find("u_size");
            if( us == ext.item.end() ) {
                // not size
                jsonResult = create_error_json( "Rack doesn't have u_size specified", 103 );
                return 1;
            }
            freeusize = usize_to_int( us->second.first );
            log_debug( "rack size is %i", freeusize );

            // get devices inside the rack
            auto devices = persist::select_asset_device_by_container(conn, elementId);
            if( ! devices.status ) {
                jsonResult = create_error_json( "Error reading rack content", 104 );
                return 1;
            }
            /*
              select usize for elements
            */
            jsonResult = std::string("{ \"id\":") + element + ", \"freeusize\":" + std::to_string(freeusize) + " }" ;
            return 0;
        } else {
            // this is not rack
            jsonResult = create_error_json( "Specified asset element is not rack", 105 );
            return 2;
        }
    } catch(std::exception &e) {
        jsonResult = create_error_json( e.what(), 105 );
        return 1;
    }
}

// MERGE TO WEB_UTILS
class Foo {

    public:
        constexpr static const char* NAME = "rack.outlets.available";

        Foo (unsigned id, const std::map<std::string, int>& results):
            id{id},
            results{results}
        {}

        unsigned id;
        std::map<std::string, int> results;


};

void operator<<= (cxxtools::SerializationInfo& si, const Foo& object)
{
  si.setTypeName("Object");
  si.addValue("id", object.id);

  cxxtools::SerializationInfo &si2 = si.addMember(Foo::NAME);
  si2.setTypeName("Object");

  for (const auto& it : object.results) {
    if (it.second >= 0)
        si2.addValue(it.first, it.second);
    else {
        auto &si3 = si2.addMember(it.first);
        si3.setNull();
    }
  }
}

template <typename T>
static std::string toString(const T& type)
{
    std::ostringstream os;
    cxxtools::JsonSerializer s;
    s.beautify(true);
    s.begin(os);
    s.serialize(type);
    s.finish();
    return os.str();
}

int
rack_outlets_available(
        const std::string &element, std::string &json_result)
{
    a_elmnt_id_t elementId =  string_to_uint32( element.c_str() );
    std::map<std::string, int> res;
    int sum = 0;
    bool tainted = false;

    std::function<void(const tntdb::Row &row)> cb = \
        [&sum, &tainted, &res](const tntdb::Row &row)
        {
            std::string device_type_name = "";
            row[3].get(device_type_name);
            if (device_type_name != "epdu")
                return;

            a_elmnt_id_t device_asset_id = 0;
            row[1].get(device_asset_id);
            res[std::to_string(device_asset_id)] = 10;

            // if no outlet.count && tainted = true;
            sum += 10;
        };

    try{
        tntdb::Connection conn;
        conn = tntdb::connectCached(url);

        persist::select_asset_device_by_container(
                conn, elementId, cb);

    } catch (std::exception &e) {
        json_result = create_error_json(e.what(), 105);
        return -1;
    }
    res["sum"] = !tainted ? sum : -1;

    Foo f{elementId, res};
    json_result = toString(f);
    return 0;
}
