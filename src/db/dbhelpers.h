/*
Copyright (C) 2015 Eaton
 
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

/*! \file   dbhelpers.h
    \brief  Helper function for direct interact with DB
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/

#ifndef SRC_DB_DBHELPERS_H_
#define SRC_DB_DBHELPERS_H_
#include <functional>
#include <string>
#include <map>
#include <tntdb/row.h>
#include <czmq.h>
#include <vector>
#include <tuple>
#include <string>
#include "dbtypes.h"

#include "preproc.h"
// all fields called name
#define MAX_NAME_LENGTH         50
// t_bios_asset_ext_attributes.keytag
#define MAX_KEYTAG_LENGTH       40
// t_bios_asset_ext_attributes.value
#define MAX_VALUE_LENGTH        255
// t_bios_asset_device.mac
#define MAX_MAC_LENGTH          17
// t_bios_asset_device.hostname
#define MAX_HOSTNAME_LENGTH     25
// t_bios_asset_device.fullhostname
#define MAX_FULLHOSTNAME_LENGTH 45

#define MAX_DESCRIPTION_LENGTH  255

typedef std::function<void(const tntdb::Row&)> row_cb_f ;

template <typename T>
struct db_reply{
    int status; // ok/fail
    int errtype;
    int errsubtype;
    uint64_t rowid;  // insert/update or http error code if status == 0
    uint64_t affected_rows; // for update/insert/delete
    std::string msg;
    zhash_t *addinfo;
    T item;
};

typedef db_reply<uint64_t> db_reply_t;

inline db_reply_t db_reply_new() {
    return db_reply_t {
        .status = 1,
        .errtype = 0,
        .errsubtype = 0,
        .rowid = 0,
        .affected_rows = 0,
        .msg = "",
        .addinfo = NULL,
        .item = 0};
}

template <typename T>
inline db_reply<T> db_reply_new(T& item) {
    return db_reply<T> {
        .status = 1,
        .errtype = 0,
        .errsubtype = 0,
        .rowid = 0,
        .affected_rows = 0,
        .msg = "",
        .addinfo = NULL,
        .item = item};
}

/**
 * \brief helper structure for results of v_bios_measurement
 */
struct db_msrmnt_t {
    m_msrmnt_id_t id;
    time_t timestamp;
    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    m_msrmnt_id_t device_id;
    std::string units;
    std::string topic;
};

/**
 * \brief helper structure for results of v_bios_asset_element
 */
struct db_a_elmnt_t {
    a_elmnt_id_t     id;
    std::string      name;
    std::string      status;
    a_elmnt_id_t     parent_id;
    a_elmnt_pr_t     priority;
    a_elmnt_tp_id_t  type_id;
    a_elmnt_stp_id_t subtype_id;
    std::string      asset_tag;
    std::map <std::string, std::string> ext;

    db_a_elmnt_t () :
        id{},
        name{},
        status{},
        parent_id{},
        priority{},
        type_id{},
        subtype_id{},
        asset_tag{},
        ext{}
    {}

    db_a_elmnt_t (
        a_elmnt_id_t     id,
        std::string      name,
        std::string      status,
        a_elmnt_id_t     parent_id,
        a_elmnt_pr_t     priority,
        a_elmnt_tp_id_t  type_id,
        a_elmnt_stp_id_t subtype_id,
        std::string      asset_tag) :

        id(id),
        name(name),
        status(status),
        parent_id(parent_id),
        priority(priority),
        type_id(type_id),
        subtype_id(subtype_id),
        asset_tag(asset_tag),
        ext{}
    {}
};

/**
 * \brief A type for storing basic information about device.
 *
 * First  -- id
 *              asset element id of the device in database.
 * Second -- device_name
 *              asset element name of the device in database.
 * Third  -- device_type_name
 *              name of the device type in database.
 * Forth  -- device_type_id
 *              id of the device type in database.
 */
typedef std::tuple< uint32_t, std::string, std::string, uint32_t > device_info_t;

inline uint32_t device_info_id(const device_info_t& d) {
    return std::get<0>(d);
}
inline uint32_t device_info_type_id(const device_info_t& d) {
    return std::get<3>(d);
}
inline std::string device_info_name(const device_info_t& d) {
    return std::get<1>(d);
}
inline std::string device_info_type_name(const device_info_t& d) {
    return std::get<2>(d);
}


/**
 * \brief A type for storing basic information about powerlink.
 *
 * First  -- src_id
 *              asset element id of the source device.
 * Second -- src_out
 *              output port on the source device.
 * Third  -- dest_id
 *              asset element id of the destination device.
 * Forth  -- dest_in
 *              input port on the destination device.
 */
typedef std::tuple< a_elmnt_id_t, std::string, a_elmnt_id_t, std::string > powerlink_info_t;


/**
 * \brief This function looks for a device_discovered in a monitor part 
 * which is connected with the specified asset_element in the asset part.
 *
 * Throws exceptions: bios::MonitorCounterpartNotFound
 *                          if apropriate monitor element was not found.
 *                    bios::InternalDBError
 *                          if database error occured.
 *                    bios::ElementIsNotDevice
 *                          if specified element was not a device.
 *                    bios::NotFound
 *                          if specified asset element was not found.
 *
 * \param url              - the connection to database.
 * \param asset_element_id - the id of the asset_element.
 *
 * \return device_discovered_id - of the device connected with the 
 *                                asset_element.
 */
m_dvc_id_t convert_asset_to_monitor_old(const char* url, 
                a_elmnt_id_t asset_element_id);

// the same as previos. but c-style error handling
int convert_asset_to_monitor_safe_old(const char* url, 
                a_elmnt_id_t asset_element_id, m_dvc_id_t *device_id);


/**
 * \brief This function looks for an asset_element in an asset part which 
 * is connected with the specified device_discovered in monitor part.
 *
 * Throws exceptions: bios::NotFound
 *                          if asset element was not found.
 *                    bios::InternalDBError
 *                          if database error occured.
 *
 * \param url                  - the connection to database.
 * \param device_discovered_id - the id of the device_discovered.
 *
 * \return asset_element_id - id of the asset_element connected with the 
 *                                device_discovered.
 */
a_elmnt_id_t convert_monitor_to_asset(const char* url, 
                    m_dvc_id_t discovered_device_id);

int convert_monitor_to_asset_safe(const char* url, 
                    m_dvc_id_t discovered_device_id, a_elmnt_id_t *asset_element_id);

/**
 * \brief Checks if the type is correct
 *
 * Checks against the enum asset_type.
 *
 * \param element_type_id - type to check
 *
 * \return true  - ok
 *         false - not known type
 */
bool is_ok_element_type (a_elmnt_tp_id_t element_type_id);

/**
 * \brief Checks if name is correct 
 *
 * \param name - name to check
 *
 * \return true if name is correct
 *         false if name is not correct
 */
bool is_ok_name (const char* name);

/**
 * \brief Checks if the keytag is correct
 *
 * Checks only the length of the keytag
 * TODO: = is a forbidden sign in zhash.
 *
 * \param keytag - keytag to check
 *
 * \return true  - ok
 *         false - keytag is not correct
 */
bool is_ok_keytag (const char* keytag);


/**
 * \brief Checks if the value (for keytag) is correct
 *
 * Checks only the length of the value
 *
 * \param value - value to check
 *
 * \return true  - ok
 *         false - value is not correct
 */
bool is_ok_value (const char* value);


/**
 * \brief Checks if device type is correct
 *
 * Checks only if device type is specified (!=0)
 *
 * \param asset_device_type_id - device type to check
 *
 * \return true  - ok
 *         false - device type is not correct
 */
bool is_ok_asset_device_type (a_dvc_tp_id_t asset_device_type_id);


/**
 * \brief Checks if the hostname is correct
 *
 * \param hostname - hostname to check
 *
 * \return true  - ok
 *         false - hostname is not correct
 */
bool is_ok_hostname (const char *hostname);


/**
 * \brief Checks if the fullhostname is correct
 *
 * \param fullhostname - fullhostname to check
 *
 * \return true  - ok
 *         false - fullhostname is not correct
 */
bool is_ok_fullhostname (const char *fullhostname);


/**
 * \brief Checks if the mac is correct
 *
 * \param mac - mac address to check
 *
 * \return true  - ok
 *         false - mac address is not correct
 */
bool is_ok_mac (const char *mac);


/**
 * \brief Checks if the link type is correct
 *
 * \param link_type_id - link type to check
 *
 * \return true  - ok
 *         false - link type is not correct
 */
bool is_ok_link_type (a_lnk_tp_id_t link_type_id);


/**
 * \brief Generate the placeholder name
 *
 * \param[in] i - index of a column
 * \param[in] j - index of a row
 *
 * \return name of placeholder
 *
 * sql_plac(2, 3) -> "item2_3";
 */
std::string
sql_plac(
        size_t i,
        size_t j);

/**
 * \brief Generate the SQL string for multivalue insert
 *
 * multi_insert_string("INSERT INTO t_bios_foo", 2, 3, "ON DUPLICATE KEY ....") ->
 * 'INSERT INTO t_bios_foo (foo, bar)
 * VALUES(:item0_0, :item0_1),
 * (:item1_0, :item1_1),
 * (:item2_0, :item2_1)
 *  ON DUPLICATE KEY UPDATE ...'
*/
std::string
multi_insert_string(
        const std::string& sql_header,
        size_t tuple_len,
        size_t items_len,
        const std::string& sql_postfix);

/**
 * \brief Generate the SQL string for multivalue insert
 *
 * multi_insert_string("INSERT INTO t_bios_foo", 2, 3, "ON DUPLICATE KEY ....") ->
 * 'INSERT INTO t_bios_foo (foo, bar)
 * VALUES(:item0_0, :item0_1),
 * (:item1_0, :item1_1),
 * (:item2_0, :item2_1)'
*/
std::string
multi_insert_string(
        const std::string& sql_header,
        size_t tuple_len,
        size_t items_len);

#endif // SRC_DB_DBHELPERS_H_
