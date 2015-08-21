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

/*! \file   assetr.h
    \brief  Basic select-functions for assets
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
*/
#ifndef SRC_DB_ASSETS_ASSETR_H
#define SRC_DB_ASSETS_ASSETR_H

#include <functional>
#include <vector>
#include <map>

#include <tntdb/connect.h>

#include "dbtypes.h"
#include "dbhelpers.h"
#include "db/types.h"

/**
 * \brief helper structure for results of v_web_element
 */
struct db_web_basic_element_t {
    a_elmnt_id_t     id;
    std::string      name;
    std::string      status;
    a_elmnt_pr_t     priority;
    a_elmnt_bc_t     bc;        // business critical
    a_elmnt_tp_id_t  type_id;
    std::string      type_name;
    a_elmnt_id_t     parent_id;
    a_elmnt_id_t     parent_type_id;
    a_dvc_tp_id_t    subtype_id;
    // TODO location
    std::string      subtype_name;
    std::string      asset_tag;
};

/**
 * \brief helper structure for links
 */
struct db_tmp_link_t {
    a_elmnt_id_t     src_id;
    a_elmnt_id_t     dest_id;
    std::string      src_socket;
    std::string      dest_socket;
};

struct db_web_element_t{
    db_web_basic_element_t basic;
    std::vector <a_elmnt_id_t> groups;
    std::vector <db_tmp_link_t> powers;
    std::map <std::string, std::pair<std::string, bool> > ext;
};


namespace persist{



db_reply <db_web_basic_element_t>
    select_asset_element_web_byId
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);

db_reply < std::map <std::string, std::pair<std::string, bool> > >
    select_ext_attributes
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);
int
select_ext_attributes(
        tntdb::Connection &conn,
        a_elmnt_id_t element_id,
        std::map <std::string, std::pair<std::string, bool> >& out);


db_reply <std::vector <db_tmp_link_t> >
    select_asset_device_links_to
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id,
         a_lnk_tp_id_t link_type_id);

db_reply <std::vector <a_elmnt_id_t> >
    select_asset_element_groups
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id);

db_reply <std::map <uint32_t, std::string> >
    select_short_elements
        (tntdb::Connection &conn,
         a_elmnt_tp_id_t type_id);

reply_t
    select_dc_of_asset_element
        (tntdb::Connection &conn,
         a_elmnt_id_t  element_id,
         a_elmnt_id_t &dc_id);

/**\brief select everything from v_web_element
 *
 * \param[in]   tntdb connection
 * \param[in]   callback function to operate on result row
 */
int
    select_asset_element_all(
            tntdb::Connection& conn,
            std::function<void(const tntdb::Row&)>& cb);

/**\brief select all stored keytag names from v_bios_asset_ext_attributes
 *
 * \param[in]   tntdb connection
 * \param[in]   callback function to operate on result row
 */
int
    select_ext_attributes_keytags(
            tntdb::Connection& conn,
            std::function<void(
                const tntdb::Row&
                )>& cb);

int
    select_asset_element_parent_name(
            tntdb::Connection& conn,
            a_elmnt_id_t id,
            std::string& name);

/** \brief selects all group names for given element id
 *
 *  \param[in] conn is tntdb connection
 *  \param[in] id is asset element id
 *  \param[in] callback function to operate on result row
 *
 *  \return -1 in case of error or 0 for success
 */
int
select_group_names(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        std::function<void(const tntdb::Row&)> cb);

/** \brief selects all group names for given element id
 *
 *  \param[in]  conn is tntdb connection
 *  \param[in]  id is asset element id
 *  \param[out] vector of strings with results
 *
 *  \return -1 in case of error or 0 for success
 */
int
select_group_names(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        std::vector<std::string>& out);

/** \brief selects information about power links for given device id
 *
 *  \param[in] conn is tntdb connection
 *  \param[in] id is asset element id
 *  \param[in] callback function to operate on result row
 *
 *  \return -1 in case of error or 0 for success
 */
int
select_v_web_asset_power_link_src_byId(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        row_cb_f& cb);

/** \brief select maximal number of groups in the system
 *
 *  \param[in] conn is tntdb connection
 *
 *  \return -1 in case of error otherwise number of groups
 */
int
max_number_of_asset_groups(
        tntdb::Connection& conn);


/** \brief select maximal number of power sources for device in the system
 *
 *  \param[in] conn is tntdb connection
 *
 *  \return -1 in case of error otherwise number of power sources
 */
int
max_number_of_power_links(
        tntdb::Connection& conn);


/** \brief check if the pair (key, value) is unique
 *
 *  \param[in] conn is tntdb connection
 *  \param[in] keytag is a keytag to check
 *  \param[in] value is value to check
 *
 *  \return -1 in case of error
 *           0 if there is no such pair in db yet
 *           otherwise number of such pairs
 */
int
unique_keytag(
        tntdb::Connection &conn,
        const std::string &keytag,
        const std::string &value,
        a_elmnt_id_t       element_id);

db_reply_t
    select_monitor_device_type_id
        (tntdb::Connection &conn,
         const char *device_type_name);

int
    convert_asset_to_monitor(
        tntdb::Connection &conn,
        a_elmnt_id_t       asset_element_id,
        m_dvc_id_t        &monitor_element_id);

} //namespace end
#endif // SRC_DB_ASSETS_ASSETR_H
