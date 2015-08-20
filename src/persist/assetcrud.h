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

/*! \file   assecrud.h
    \brief  Basic functions for assets
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/

#ifndef SRC_PERSIST_ASSETCRUD_H_
#define SRC_PERSIST_ASSETCRUD_H_

#include <tntdb/connect.h>

#include "dbtypes.h"
#include "defs.h"
#include "dbhelpers.h"
#include "asset_types.h"


// ===============================================================
// Helper functions for direct interacting with database
// ===============================================================


/**
 * \brief Gets data about groups the specifeid element belongs to.
 *
 * Get only a list of group IDs the element belongs to.
 *
 * \param url        - the connection to database.
 * \param element_id - the id of the element (from 
 *                     t_bios_asset_element) we search groups for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified element doesn't 
 *                                belong to any group.
 *         filled object zlist_t  if the specified element belongs to 
 *                                some groups.
 */
zlist_t* select_asset_element_groups(const char* url,
                                   a_elmnt_id_t element_id);

db_reply_t delete_asset_ext_attributes(tntdb::Connection &conn,
                                   a_elmnt_id_t  asset_element_id);

db_reply_t delete_asset_ext_attribute(tntdb::Connection &conn,
                                   const char   *keytag,
                                   a_elmnt_id_t  asset_element_id);
db_reply_t
    process_insert_inventory
        (tntdb::Connection &conn, const char *device_name, zhash_t *ext_attributes);


std::set <a_elmnt_id_t> select_asset_group_elements (tntdb::Connection &conn, a_elmnt_id_t group_id);


zlist_t* select_asset_device_links_all(tntdb::Connection &conn,
                a_elmnt_id_t device_id, a_lnk_tp_id_t link_type_id);

db_reply <db_a_elmnt_t>
    select_asset_element_by_name
        (tntdb::Connection &conn,
         const char *element_name);


// dictionaries

/**
 * \brief Reads from database all available element types.
 *
 * Reads from the table t_bios_asset_element_type;
 *
 * \param[in] conn - the connection to database.
 *
 * \return a database reply where item is a map of names at the ids.
 *         In case of any erorrs item would be empty.
 */
db_reply < std::map <std::string, int> >
    get_dictionary_element_type
        (tntdb::Connection &conn);


/**
 * \brief Reads from database all available device types.
 *
 * Reads from the table t_bios_asset_device_type;
 *
 * \param[in] conn - the connection to database.
 *
 * \return a database reply where item is a map of names at the ids.
 *         In case of any erorrs item would be empty.
 */
db_reply < std::map <std::string, int> >
    get_dictionary_device_type
        (tntdb::Connection &conn);

db_reply <std::vector<db_a_elmnt_t>>
    select_asset_elements_by_type
        (tntdb::Connection &conn,
         a_elmnt_tp_id_t type_id);

db_reply <std::set <std::pair<a_elmnt_id_t ,a_elmnt_id_t>>>
    select_links_by_container
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);
db_reply <std::vector<device_info_t>>
    select_asset_device_by_container
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);


#endif // SRC_PERSIST_ASSETCRUD_H_
