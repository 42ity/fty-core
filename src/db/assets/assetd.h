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

/*! \file   assetd.h
    \brief  Basic delete functions for assets
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#ifndef SRC_DB_ASSETS_ASSETD_H_
#define SRC_DB_ASSETS_ASSETD_H_

#include "dbtypes.h"
#include "defs.h"
#include "dbhelpers.h"
#include <tntdb/connect.h>
#include "asset_types.h"
#include "calc_power.h"

namespace persist {
////////////////// DELETE

db_reply_t
    delete_asset_link
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id_src,
         a_elmnt_id_t asset_element_id_dest);

db_reply_t
    delete_asset_links_all
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id);

db_reply_t
    delete_asset_links_to
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_device_id);

db_reply_t
    delete_asset_links_from
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_device_id);

db_reply_t
    delete_asset_group_links
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_group_id);

db_reply_t
    delete_asset_ext_attribute
        (tntdb::Connection &conn,
         const char   *keytag,
         a_elmnt_id_t  asset_element_id);

db_reply_t
    delete_asset_ext_attributes
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id);

db_reply_t
    delete_asset_element
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id);

db_reply_t
    delete_asset_element_from_asset_groups
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id);

db_reply_t
    delete_asset_element_from_asset_group
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_group_id,
         a_elmnt_id_t asset_element_id);

db_reply_t
    delete_monitor_asset_relation
        (tntdb::Connection &conn,
         ma_rltn_id_t id);

db_reply_t
    delete_monitor_asset_relation_by_a
        (tntdb::Connection &conn,
         a_elmnt_id_t id);

} // end namespace
#endif // SRC_DB_ASSETS_ASSETD_H_
