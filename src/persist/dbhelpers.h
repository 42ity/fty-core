/*
Copyright (C) 2015 Eaton
 
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

/*! \file dbhelpers.h
    \brief Helper function for direct interact with DB
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_DBHELPERS_H_
#define SRC_PERSIST_DBHELPERS_H_

#include "dbtypes.h"

// all fields called name
#define MAX_NAME_LENGTH         25
// t_bios_asset_ext_attributes.keytag
#define MAX_KEYTAG_LENGTH       40
// t_bios_asset_ext_attributes.value
#define MAX_VALUE_LENGTH        255
// t_bios_asset_device.mac   and t_bios_net_history.mac
#define MAX_MAC_LENGTH          17
// t_bios_asset_device.ip    and t_bios_discovered_ip.ip
#define MAX_IP_LENGTH           45
// t_bios_asset_device.hostname
#define MAX_HOSTNAME_LENGTH     25
// t_bios_asset_device.fullhostname
#define MAX_FULLHOSTNAME_LENGTH 45

#define MAX_DESCRIPTION_LENGTH  255
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
m_dvc_id_t convert_asset_to_monitor(const char* url, 
                a_elmnt_id_t asset_element_id);

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
 * \brief Checks if device type is is correct
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
 * \brief Checks if the ip is correct
 *
 * \param ip - ip to check
 *
 * \return true  - ok
 *         false - ip is not correct
 */
bool is_ok_ip (const char *ip);


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


#endif // SRC_PERSIST_DBHELPERS_H_
