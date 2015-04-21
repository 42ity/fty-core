/*
Copyright (C) 2014 Eaton
 
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

/*! \file assecrud.h
    \brief Basic functions for assets
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_ASSETCRUD_H_
#define SRC_PERSIST_ASSETCRUD_H_

#include "asset_msg.h"
#include "dbtypes.h"
#include "defs.h"
#include "dbhelpers.h"
#include <tntdb/connect.h>
#include "asset_types.h"

typedef struct _asset_link
{
    a_elmnt_id_t    src;     //!< id of src element
    a_elmnt_id_t    dest;    //!< id of dest element
    a_lnk_src_out_t src_out; //!< outlet in src
    a_lnk_src_out_t dest_in; //!< inlet in dest
    a_lnk_tp_id_t   type;    //!< link type id
} link_t;



// ===============================================================
// Functions for processing a special message type
// ===============================================================

/**
 * \brief This function processes the ASSET_MSG_GET_ELEMENT message.
 *
 * In case of success it generates ASSET_MSG_RETURN_ELEMENT message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * Message msg left unchanged.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_ELEMENT 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or ASSET_MSG_RETURN_ELEMENT
 *                  message.
 */
zmsg_t* get_asset_element(const char *url, asset_msg_t *msg);

/**
 * \brief This function processes the ASSET_MSG_GET_ELEMENTS message.
 *
 * In case of success it generates ASSET_MSG_RETURN_ELEMENTS message.
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * Message msg left unchanged.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_ELEMENTS 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or ASSET_MSG_RETURN_ELEMENTS
 *                  message.
 */
zmsg_t* get_asset_elements(const char *url, asset_msg_t *msg);


// ===============================================================
// Helper functions for direct interacting with database
// ===============================================================

/**
 * \brief Gets information about specified element (and its specified type)
 * with extra attributes.
 *
 * In case of success it generates ASSET_MSG_ELEMENT message.
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - the connection to database.
 * \param element_id     - the id of the element (from t_bios_asset_element) 
 *                         we search for.
 * \param elemet_type_id - the id of element's type.
 *
 * \return zmsg_t - an encoded ASSET_MSG_ELEMENT or COMMON_MSG_FAIL message.
 */
zmsg_t* select_asset_element(tntdb::Connection &conn, a_elmnt_id_t element_id, 
                              a_elmnt_tp_id_t element_type_id);

/**
 * \brief Gets additional information about specified device.
 *
 * Converts the the message of the type ASSET_MSG_ELEMENT to ASSET_MSG_DEVICE.
 *
 * In case of success it generates ASSET_MSG_DEVICE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * It destoyes the message element.
 * 
 * \param url     - the connection to database.
 * \param element - the message of the type ASSET_MSG_ELEMENT we would like 
 *                  to extend.
 *
 * \return zmsg_t - an encoded ASSET_MSG_DEVICE or COMMON_MSG_FAIL message.
 */
zmsg_t* select_asset_device(tntdb::Connection &conn, 
                            a_elmnt_id_t element_id);

/**
 * \brief Gets extra attributes for the specified element.
 *
 * Get a hash table of key-value for existing extra attributes for 
 * the specified element.
 *
 * \param url        - the connection to database.
 * \param element_id - the id of the element (from t_bios_asset_element) 
 *                     we search extra attributes for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zhash_t   if there is no any specified 
 *                                extra attributes.
 *         filled object zhash_t  if there is at least one extra attribute 
 *                                for the specified element.
 */
zhash_t* select_asset_element_attributes(tntdb::Connection &conn, 
                                         a_elmnt_id_t element_id);
/**
 * \brief Gets data about links for the specifeid device.
 *
 * Device is specified with t_bios_asset_element.id_asset_element (it is 
 * different from t_bios_asset_device.id_asset_device).
 *
 * TODO rewrite with using convert_powerchain_powerlink2list 
 * TODO from assettopology.cc
 *
 * Get a list of links the device belongs to.
 * The every element of the list would be a string "A:B:C:D", where 
 * A - src_out
 * B - src_id
 * C - dest_in
 * D - dest_id.
 * If A or C is SRCOUT_DESTIN_IS_NULL then it means that these parameters 
 * where not specified in the database.
 * 
 * \param url          - the connection to database.
 * \param device_id    - the id of the device 
 *                       (t_bios_asset_device.id_asset_device) we search 
 *                       links for.
 * \param link_type_id - the type of the link.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified device doesn't 
 *                                belong to any link.
 *         filled object zlist_t  if the specified device belongs to 
 *                                some links.
 */
zlist_t* select_asset_device_link(tntdb::Connection &conn, 
                a_elmnt_id_t device_id, a_lnk_tp_id_t link_type_id);

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

db_reply_t insert_into_asset_ext_attribute (tntdb::Connection &conn,
                                   const char   *value,
                                   const char   *keytag,
                                   a_elmnt_id_t  asset_element_id,
                                   bool          read_only);

db_reply_t insert_into_asset_ext_attributes (tntdb::Connection &conn,
                                   zhash_t      *attributes,
                                   a_elmnt_id_t  asset_element_id,
                                   bool          read_only);

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

/// insert

db_reply_t
    insert_asset_element_into_asset_group 
        (tntdb::Connection &conn,
         a_elmnt_id_t group_id,
         a_elmnt_id_t asset_element_id);

db_reply_t
    insert_into_asset_device
        (tntdb::Connection &conn,
         a_elmnt_id_t   asset_element_id,
         a_dvc_tp_id_t  asset_device_type_id);

db_reply_t
    insert_into_asset_link
        (tntdb::Connection &conn,
         a_elmnt_id_t    asset_element_src_id,
         a_elmnt_id_t    asset_element_dest_id,
         a_lnk_tp_id_t   link_type_id,
         const a_lnk_src_out_t src_out,
         const a_lnk_dest_in_t dest_in);
/*
db_reply_t
    insert_into_asset_ext_attribute
        (tntdb::Connection &conn,
         const char   *keytag,
         a_elmnt_id_t  asset_element_id);

db_reply_t
    insert_into_asset_ext_attributes
        (tntdb::Connection &conn, 
         zhash_t      *attributes,
         a_elmnt_id_t  asset_element_id);
*/
db_reply_t
    insert_into_asset_element
        (tntdb::Connection &conn, 
         const char      *element_name, 
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc);

db_reply_t
    insert_into_asset_links
        (tntdb::Connection       &conn,
         std::vector <link_t> const &links);

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
         std::set <a_elmnt_id_t> const &groups);

db_reply_t
    insert_device
        (tntdb::Connection &conn,
         std::vector <link_t> &links,
         std::set <a_elmnt_id_t> const &groups,
         const char    *element_name, 
         a_elmnt_id_t   parent_id,
         zhash_t       *extattributes,
         a_dvc_tp_id_t  asset_device_type_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc);

////////////////// DELETE
//
db_reply_t
    delete_asset_device
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id);

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
         a_dvc_id_t asset_device_id);

db_reply_t
    delete_asset_links_from
        (tntdb::Connection &conn, 
         a_dvc_id_t asset_device_id);

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

db_reply_t
    delete_monitor_asset_relation
        (tntdb::Connection &conn, 
         ma_rltn_id_t id);

db_reply_t
    insert_into_monitor_asset_relation
        (tntdb::Connection &conn,
         m_dvc_id_t   monitor_id,
         a_elmnt_id_t element_id);

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

#endif // SRC_PERSIST_ASSETCRUD_H_
