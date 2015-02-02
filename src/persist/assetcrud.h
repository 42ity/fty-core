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

#ifndef SRC_PERSIST_ASSETMSG_H_
#define SRC_PERSIST_ASSETMSG_H_

#include "asset_msg.h"
#include "dbtypes.h"
#include "defs.h"

// For now it the maximum length of all fields 
// called "name"
#define MAX_NAME_LENGTH 25
#define MAX_DESCRIPTION_LENGTH 255

/**
 * \brief Processes message of type asset_msg_t
 *
 * Broken down processing of generic database zmsg_t, this time asset message
 * case.
 */
zmsg_t *asset_msg_process(zmsg_t **msg);

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
zmsg_t* select_asset_element(const char* url, a_elmnt_id_t element_id, 
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
zmsg_t* select_asset_device(const char* url, asset_msg_t** element, 
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
zhash_t* select_asset_element_attributes(const char* url, 
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
zlist_t* select_asset_device_link(const char* url, 
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

/**
 * \brief This function looks for a device_discovered from a monitor part 
 * connected with the specified asset_element in the asset part.
 *
 * Throws exceptions: bios::MonitorCounterpartNotFound - in case apropriate 
 *                              monitor element was not found.
 *                    bios::InternalDBError - in case of any database errors.
 *                    bios::ElementIsNotDevice - if specified element was 
 *                              not a device.
 *                    bios::NotFound - if specified asset element was not found.
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
 * \brief This function looks for an asset_element form asset part connected 
 * with the specified device_discovered from monitor part.
 *
 * Throws exceptions: bios::NotFound - in case apropriate asset element 
 *                                     was not found.
 *                    bios::InternalDBError - in case of any database errors.
 *
 * \param url                  - the connection to database.
 * \param device_discovered_id - the id of the device_discovered.
 *
 * \return asset_element_id - id of the asset_element connected with the 
 *                                device_discovered.
 */
a_elmnt_id_t convert_monitor_to_asset(const char* url, 
                    m_dvc_id_t discovered_device_id);

#endif // SRC_PERSIST_ASSETMSG_H_
