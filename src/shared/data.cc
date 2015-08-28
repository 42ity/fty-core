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
 * \file data.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <tnt/http.h>
#include <algorithm>
#include <map>
#include <limits.h>

#include "data.h"

#include "log.h"
#include "asset_types.h"
#include "dbpath.h"
#include "monitor.h"
#include "upsstatus.h"
#include "defs.h"

#include "asset_general.h"
#include "utils.h"

#include "db/agentstate.h"

typedef std::string (*MapValuesTransformation)(std::string);


static std::string s_scale(const std::string& val, int8_t scale) {
// TODO: Refactor away multiple calls to val.size(),
// they seem redundant (val does not change here?)
    assert(val != "");
// The string.size() is a size_t (unsigned int or larger), and
// our scale is a signed byte; make sure they fit each other
    assert(val.size() <= SCHAR_MAX);

    std::string ret{val};

    //1. no scale, nothing to do
    if (!scale)
        return ret;

    //2. positive scale, simply append things
    if (scale > 0) {
        while (scale > 0) {
            ret.append("0");
            scale--;
        }
        return ret;
    }

    //3. scale is "bigger" than size of string,
    if (-scale >= (int8_t)val.size()) {
        //3a. prepend zeroes
        while (-scale != (int8_t)val.size()) {
            ret.insert(0, "0");
            scale++;
        }

        //3b and prepend 0.
        ret.insert(0, "0.");
        return ret;
    }

    //4. just find the right place for dot
    ret.insert(val.size() + scale, ".");
    return ret;

}

std::string measures_manager::apply_scale(const std::string &val, const std::string &scale)
{
    int scale_num = std::stoi (scale, nullptr, 10);
    return s_scale(val, scale_num);
}

std::string measures_manager::map_values(std::string name, std::string value) {
    static std::map<std::string, MapValuesTransformation > map = {
        { "status.ups", &shared::upsstatus_to_string }
    };
    
    auto it = map.find(name);
    if(it != map.end()) {
        return "\"" + it->second(value) + "\"";
    }
    return value;
}

int
    ui_props_manager::get
        (std::string& result)
{
    void *data = NULL;
    size_t size = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        auto ret = persist::select_agent_info (conn, agent_name(), &data, size);
        if ( ret )
        {
            result = std::string("Can't load ui/properties from database");
            FREE0 (data);
            return 2;
        }

        result = std::string((const char *)data, size);

        FREE0 (data);
        return 0;
    }
    catch (const std::exception &e) {
        FREE0 (data);
        LOG_END_ABNORMAL(e);
        return 1;
    }
}

std::string
    ui_props_manager::agent_name()
{
    return "UI_PROPERTIES";
}

int
    ui_props_manager::put(
        const std::string &properties,
        std::string &result
        )
{
 
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        uint16_t affected_rows = 0;
        auto ret = persist::update_agent_info (conn, agent_name(), properties.c_str(), properties.size(), affected_rows);
        if ( ret )
        {
            result = std::string("{\"error\" : \"Can't import ui/properties to database\"}");
            return 2;
        }
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
}

/**
 * \brief get the error string, msg string and HTTP code from common_msg
 */
void common_msg_to_rest_error(common_msg_t* cm_msg, std::string& error, std::string& msg, unsigned* code) {

    assert(cm_msg);

    if (common_msg_id(cm_msg) == COMMON_MSG_FAIL) {
        switch (common_msg_errorno(cm_msg)) {
            case DB_ERROR_NOTFOUND:
                error = "not_found";
                *code = HTTP_NOT_FOUND;
                break;
            case DB_ERROR_BADINPUT:
                error = "bad_input";
                *code = HTTP_BAD_REQUEST;
                break;
            case DB_ERROR_NOTIMPLEMENTED:
                error = "not_implemented";
                *code = HTTP_NOT_IMPLEMENTED;
                break;
            default:
                error = "internal_error";
                *code = HTTP_INTERNAL_SERVER_ERROR;
                break;
        }
        const char *cmsg = common_msg_errmsg(cm_msg);
        msg = (cmsg == NULL) ? "" : cmsg;
    }
    else {
        error = "internal_error";
        msg = "unexpected message";
        *code = HTTP_INTERNAL_SERVER_ERROR;
    }
}


db_reply <db_web_element_t>
    asset_manager::get_item1
        (const std::string &id)
{
    db_reply <db_web_element_t> ret;

    // TODO add better converter
    uint32_t real_id = atoi(id.c_str());
    if ( real_id == 0 )
    {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = "cannot convert an id";
        log_warning (ret.msg.c_str());
        return ret;
    }
    log_debug ("id converted successfully");

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        log_debug ("connection was successful");

        auto basic_ret = persist::select_asset_element_web_byId(conn, real_id);
        log_debug ("1/4 basic select is done");

        if ( basic_ret.status == 0 )
        {
            ret.status        = basic_ret.status;
            ret.errtype       = basic_ret.errtype;
            ret.errsubtype    = basic_ret.errsubtype;
            ret.msg           = basic_ret.msg;
            log_warning (ret.msg.c_str());
            return ret;
        }
        log_debug ("    1/4 no errors");
        ret.item.basic = basic_ret.item;

        auto ext_ret = persist::select_ext_attributes(conn, real_id);
        log_debug ("2/4 ext select is done");

        if ( ext_ret.status == 0 )
        {
            ret.status        = ext_ret.status;
            ret.errtype       = ext_ret.errtype;
            ret.errsubtype    = ext_ret.errsubtype;
            ret.msg           = ext_ret.msg;
            log_warning (ret.msg.c_str());
            return ret;
        }
        log_debug ("    2/4 no errors");
        ret.item.ext = ext_ret.item;

        auto group_ret = persist::select_asset_element_groups(conn, real_id);
        log_debug ("3/4 groups select is done, but next one is only for devices");

        if ( group_ret.status == 0 )
        {
            ret.status        = group_ret.status;
            ret.errtype       = group_ret.errtype;
            ret.errsubtype    = group_ret.errsubtype;
            ret.msg           = group_ret.msg;
            log_warning (ret.msg.c_str());
            return ret;
        }
        log_debug ("    3/4 no errors");
        ret.item.groups = group_ret.item;

        if ( ret.item.basic.type_id == persist::asset_type::DEVICE )
        {
            auto powers = persist::select_asset_device_links_to (conn, real_id, INPUT_POWER_CHAIN);
            log_debug ("4/4 powers select is done");

            if ( powers.status == 0 )
            {
                ret.status        = powers.status;
                ret.errtype       = powers.errtype;
                ret.errsubtype    = powers.errsubtype;
                ret.msg           = powers.msg;
                log_warning (ret.msg.c_str());
                return ret;
            }
            log_debug ("    4/4 no errors");
            ret.item.powers = powers.item;
        }

        ret.status = 1;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

db_reply <std::map <uint32_t, std::string> >
    asset_manager::get_items1(
        const std::string &typeName,
        const std::string &subtypeName)
{
    db_reply <std::map <uint32_t, std::string> > ret;

    a_elmnt_tp_id_t type_id = persist::type_to_typeid(typeName);
    if ( type_id == persist::asset_type::TUNKNOWN ) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = "Unsupported type of the elemnts";
        log_error (ret.msg.c_str());
        return ret;
    }
    a_elmnt_stp_id_t subtype_id = persist::subtype_to_subtypeid(subtypeName);
    if ( subtype_id == persist::asset_subtype::SUNKNOWN ) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = "Unsupported subtype of the elemnts";
        log_error (ret.msg.c_str());
        return ret;
    }

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        log_debug ("connection was successful");
        ret = persist::select_short_elements(conn, type_id, subtype_id);
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


db_reply_t
    asset_manager::delete_item(
        const std::string &id,
        db_a_elmnt_t &element_info)
{
    db_reply_t ret = db_reply_new();

    // TODO add better converter
    uint32_t real_id = atoi(id.c_str());
    if ( real_id == 0 )
    {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_NOTFOUND;
        ret.msg           = "cannot convert an id";
        log_warning (ret.msg.c_str());
        return ret;
    }
    log_debug ("id converted successfully");

    // As different types should be deleted in differenct way ->
    // find out the type of the element.
    // Even if in old-style the type is already placed in URL,
    // we will ignore it and discover it by ourselves

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        db_reply <db_web_basic_element_t> basic_info =
            persist::select_asset_element_web_byId(conn, real_id);

        if ( basic_info.status == 0 )
        {
            ret.status        = 0;
            ret.errtype       = basic_info.errsubtype;
            ret.errsubtype    = DB_ERROR_NOTFOUND;
            ret.msg           = "problem with selecting basic info";
            log_warning (ret.msg.c_str());
            return ret;
        }
        // here we are only if everything was ok
        element_info.type_id = basic_info.item.type_id;
        element_info.subtype_id = basic_info.item.subtype_id;
        element_info.name = basic_info.item.name;

        switch ( basic_info.item.type_id ) {
            case persist::asset_type::DATACENTER:
            case persist::asset_type::ROW:
            case persist::asset_type::ROOM:
            case persist::asset_type::RACK:
            {
                ret = persist::delete_dc_room_row_rack(conn, real_id);
                break;
            }
            case persist::asset_type::GROUP:
            {
                ret = persist::delete_group(conn, real_id);
                break;
            }
            case persist::asset_type::DEVICE:
            {
                ret = persist::delete_device(conn, real_id);
                break;
            }
            default:
            {
                ret.status        = 0;
                ret.errtype       = basic_info.errsubtype;
                ret.errsubtype    = DB_ERROR_INTERNAL;
                ret.msg           = "unknown type";
                log_warning (ret.msg.c_str());
            }
        }
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


std::string
    create_error_json(
        const std::string &msg,
        int                code)
{
    std::string s = "{\n\t\"errors\": [\n\t  {\n\t\t\"message\" : \"";
    s+=msg;
    s+="\",\n\t\t\"code\" : ";
    s+=std::to_string(code);
    s+="\n\t  }\n\t]\n}";
    return s;
}
