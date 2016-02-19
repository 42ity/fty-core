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
#include <algorithm>

#include "data.h"
#include "utils_web.h"

#include "log.h"
#include "asset_types.h"
#include "dbpath.h"
#include "defs.h"

#include "asset_general.h"
#include "utils.h"
#include "measurements.h"

typedef std::string (*MapValuesTransformation)(std::string);

int
    measures_manager::get_last_10minute_measurement(
        const std::string &source,
        const std::string &device_name,
        m_msrmnt_value_t  &value,
        m_msrmnt_scale_t  &scale)
{
    std::string topic = source + "@" + device_name;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        reply_t ret = persist::select_measurement_last_web_byTopic (conn, topic, value, scale);
        return ret.rv;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}


db_reply <db_web_element_t>
    asset_manager::get_item1
        (const std::string &id)
{
    db_reply <db_web_element_t> ret;

    unsigned long real_id_l = 0;
    try {
        real_id_l = std::stoul (id);
    }
    catch (const std::out_of_range& e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_BADINPUT;
        return ret;
    }
    catch (const std::invalid_argument& e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_BADINPUT;
        return ret;
    }

    if (real_id_l == 0 || real_id_l > UINT_MAX) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_BADINPUT;
        return ret;
    }
    uint32_t real_id = static_cast<uint32_t> (real_id_l);
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
    LOG_START;
    log_debug ("subtypename = '%s', typename = '%s'", subtypeName.c_str(),
                    typeName.c_str());
    a_elmnt_stp_id_t subtype_id = 0;
    db_reply <std::map <uint32_t, std::string> > ret;

    a_elmnt_tp_id_t type_id = persist::type_to_typeid(typeName);
    if ( type_id == persist::asset_type::TUNKNOWN ) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = "Unsupported type of the elemnts";
        log_error (ret.msg.c_str());
        // TODO need to have some more precise list of types, so we don't have to change anything here,
        // if something was changed
        bios_error_idx(ret.rowid, ret.msg, "request-param-bad", "type", typeName.c_str(), "datacenters,rooms,ros,racks,devices");
        return ret;
    }
    if ( ( typeName == "device" ) && ( !subtypeName.empty() ) )
    {
        subtype_id = persist::subtype_to_subtypeid(subtypeName);
        if ( subtype_id == persist::asset_subtype::SUNKNOWN ) {
            ret.status        = 0;
            ret.errtype       = DB_ERR;
            ret.errsubtype    = DB_ERROR_INTERNAL;
            ret.msg           = "Unsupported subtype of the elemnts";
            log_error (ret.msg.c_str());
            // TODO need to have some more precise list of types, so we don't have to change anything here,
            // if something was changed
            bios_error_idx(ret.rowid, ret.msg, "request-param-bad", "subtype", subtypeName.c_str(), "ups, epdu, pdu, genset, sts, server, feed");
            return ret;
        }
    }
    log_debug ("subtypeid = %" PRIi16 " typeid = %" PRIi16, subtype_id, type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        ret = persist::select_short_elements(conn, type_id, subtype_id);
        if ( ret.status == 0 )
            bios_error_idx(ret.rowid, ret.msg, "internal-error");
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        bios_error_idx(ret.rowid, ret.msg, "internal-error");
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
