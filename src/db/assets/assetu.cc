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

/*! \file   assetu.cc
    \brief  Basic update-functions for assets
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <exception>
#include <assert.h>

#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "db/assets/assetu.h"

#define ERRCODE_ABNORMAL 1

namespace persist{

// type is permanent and cannot be changed
int
    update_asset_element
        (tntdb::Connection &conn,
         a_elmnt_id_t     element_id,
         const char      *element_name,
         a_elmnt_id_t     parent_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         const char      *asset_tag,
         int32_t         &affected_rows)
{
    LOG_START;
    log_debug ("  element_id = %" PRIi32, element_id);
//    log_debug ("  element_name = '%s'", element_name);
    log_debug ("  parent_id = %" PRIu32, parent_id);
    log_debug ("  status = '%s'", status);
    log_debug ("  priority = %" PRIu16, priority);
    log_debug ("  bc = %" PRIu16, bc);
    log_debug ("  asset_tag = '%s'", asset_tag);

    // if parent id == 0 ->  it means that there is no parent and value
    // should be updated to NULL
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_asset_element"
            " SET"
//            "   name = :name,"
            "   asset_tag = :asset_tag,"
            "   id_parent = :id_parent,"
            "   business_crit = :bc,"
            "   status = :status,"
            "   priority = :priority"
            " WHERE id_asset_element = :id"
        );

        st = st.set("id", element_id).
//                           set("name", element_name).
                           set("status", status).
                           set("priority", priority).
                           set("bc", bc).
                           set("asset_tag", asset_tag);

        if ( parent_id != 0 )
        {
            affected_rows = st.set("id_parent", parent_id).
                               execute();
        }
        else
        {
            affected_rows = st.setNull("id_parent").
                               execute();
        }
        log_debug("[t_asset_element]: updated %" PRIu32 " rows", affected_rows);
        LOG_END;
        // if we are here and affected rows = 0 -> nothing was updated because
        // it was the same
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return ERRCODE_ABNORMAL;
    }
}

// as most of the time we can not determine if we have
// an update or insert. But this can be used somewhere
int
    update_asset_ext_attribute
        (tntdb::Connection &conn,
         a_elmnt_id_t     element_id,
         const char      *keytag,
         const char      *value)
{
    LOG_START;
    log_debug ("  element_id = %" PRIi32, element_id);
    log_debug ("  keytag = '%s'", keytag);
    log_debug ("  value = '%s'", value);

    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_asset_ext_atribute"
            " SET"
            "   value = :value"
            " WHERE"
            "   keytag = :keytag AND"
            "   id_asset_element = :id"
        );

        auto n = st.set("id", element_id).
                    set("value", value).
                    set("keytag", keytag).
                    set("id", element_id).
                    execute();
        log_debug("[t_asset_ext_attribute]: updated %" PRIu32 " rows", n);
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return ERRCODE_ABNORMAL;
    }
}


} // namespace end
