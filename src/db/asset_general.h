/*
Copyright (C) 2014-2015 Eaton

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

/*! \file   asset_general.h
    \brief  Header file for all not low level functions for asset
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#ifndef SRC_DB_ASSETS_GENERAL_H
#define SRC_DB_ASSETS_GENERAL_H

#include <tntdb/connect.h>
#include "db/assetdef.h"

namespace persist {

int
    update_dc_room_row_rack_group
        (tntdb::Connection  &conn,
         a_elmnt_id_t     element_id,
         const char      *element_name,
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         zhash_t         *extattributes,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         std::set <a_elmnt_id_t> const &groups,
         const std::string &asset_tag,
         std::string     &errmsg);


int
    update_device
        (tntdb::Connection  &conn,
         a_elmnt_id_t     element_id,
         const char      *element_name,
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         zhash_t         *extattributes,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         std::set <a_elmnt_id_t> const &groups,
         std::vector <link_t> &links,
         const std::string &asset_tag,
         std::string     &errmsg);


db_reply_t
    insert_dc_room_row_rack_group
        (tntdb::Connection  &conn,
         const char      *element_name,
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         zhash_t         *extattributes,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         std::set <a_elmnt_id_t> const &groups,
         const std::string &asset_tag);


db_reply_t
    insert_device
       (tntdb::Connection &conn,
        std::vector <link_t> &links,
        std::set <a_elmnt_id_t> const &groups,
        const char    *element_name,
        a_elmnt_id_t   parent_id,
        zhash_t       *extattributes,
        a_dvc_tp_id_t  asset_device_type_id,
        const char    *asset_device_type_name,
        const char    *status,
        a_elmnt_pr_t   priority,
        a_elmnt_bc_t   bc,
        const std::string &asset_tag);


db_reply_t
    delete_dc_room_row_rack
        (tntdb::Connection &conn,
        a_elmnt_id_t element_id);


db_reply_t
    delete_group
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);


db_reply_t
    delete_device
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);


}   // end namespace
#endif // SRC_DB_ASSETS_GENERAL_H
