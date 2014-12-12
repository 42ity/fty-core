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

/*! \file assettopology.h
    \brief Functions for getting the topology (location andpower) from the
           database
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_ASSETTOPOLOGY_H_
#define SRC_PERSIST_ASSETTOPOLOGY_H_

#include "asset_msg.h"

// ===============================================================
// Functions for processing a special message type
// ===============================================================

/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_FROM message
 *
 * To correct processing all fields of the message should be set up 
 * according specification.
 * 
 * To select unlockated elements need to set element_id to 0.
 * For unlockated elements only a non recursive search is provided.
 * To select without the filter need to set a filtertype to 7.
 *
 * In case of success it generates the ASSET_MSG_RETURN_LOCATION_FROM. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * It doesn't destroy the getmsg.
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_FROM
 */
zmsg_t* get_return_topology_from(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_TO message
 *
 * In case of success it generates the ASSET_MSG_RETURN_LOCATION_TO. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_TO 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_TO
 */
zmsg_t* get_return_topology_to(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_FROM message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero than A or C it was not srecified in database 
 * (it was NULL). 
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_TO message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero than A or C it was not srecified in database 
 * (it was NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_TO
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_to (const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_GROUP message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * Returns all devices in the group and returns all power links between them.
 * Links that goes outside the group are not returned.
 *
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero then A or C were not srecified in database (were NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_GROUP 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_group(const char* url, asset_msg_t* getmsg);


/*
 * \brief This function processes the ASSET_MSG_GET_POWER_DATACENTER message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * Returns all devices in datacenter and all powerlinks between them.
 * Links outside the datacenter are not returned.
 *
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero then A or C were not srecified in database (were NULL). 
 *
 * \param url    - the connection to database.
 * \param getmsg - the message of the type ASSET_MSG_GET_POWER_DATACENTER
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_DATACENTER
 */ 
zmsg_t* get_return_power_topology_datacenter(const char* url, 
                                    asset_msg_t* getmsg);

// ===============================================================
// Helper function for testing
// ===============================================================

typedef std::set<std::tuple<int,int,std::string,std::string,int,int,std::string,std::string>> edge_lf;

/**
 * \brief Helper function for testing
 * 
 * Prints a frame of devices (from ASSET_MSG_POWER_FROM message)
 *
 * \param frame - frame to print
 */
void print_frame_devices (zframe_t* frame);


/**
 * \brief Helper function for testing
 * 
 * prints a frame (from ASSET_MSG_LOCATION_FROM message)
 *
 * \param frame      - frame to print
 * \param parent_id  - id (asset_elememt_id) of the parent
 * 
 */
void print_frame (zframe_t* frame, uint32_t parent_id);


/**
 * \brief Helper function for testing
 * 
 * Converts a frame (from ASSET_MSG_LOCATION_FROM message) into 
 * the std::set of edges in the tree (child,parent);
 *
 * \param frame      - frame to convert
 * \param parent_id  - id (asset_elememt_id) of the root
 * \param id_type    - id of the type of the root
 * \param name       - name of the root compared to
 * \param dtype_name - name of the precise element type of root 
 *                      (available only for groups and devices).
 * 
 * \return std::set of edges
 */
edge_lf print_frame_to_edges (zframe_t* frame, uint32_t parent_id, 
                uint8_t type, std::string name, std::string dtype_name);


/* \brief Helper function for testing 
 *  
 * Compares the element in the ASSET_MSG_RETURN_LOCATION_FROM or in 
 * the ASSET_MSG_RETURN_LOCATION_TO field by field with specified values.
 *
 * \param rmsg       - message with element
 * \param id         - id (asset_elememt_id) of the element compared to
 * \param id_type    - id of the type of the element compared to
 * \param name       - name of the element compared to
 * \param dtype_name - name of the precise element type of the element 
 *                      compared to (available only for groups and devices).
 *
 * \return true  - if the element is the same
 *         false - if elements are different
 */
bool compare_start_element (asset_msg_t* rmsg, uint32_t id, uint8_t id_type,
                            const char* name, const char* dtype_name);

// ===============================================================
// Functions for direct interacting with  database
// ===============================================================

/**
 * \brief Selects group elements of specified type for the specified group.
 *  
 *  Paramenters "element_type_id" and "group_name" are used only in the 
 *  returned zmsg_t message.
 * 
 *  In case of success it generates the ASEET_MSG_RETURN_LOCATION_FROM. 
 *  In case of failure returns COMMON_MSG_FAIL.

 * \param url             - connection to database
 * \param element_id      - asset element id of group
 * \param element_type_id - asset_element_type_id of the group
 * \param group_name      - name of the group
 * \param filter_type     - id of the type of the searched elements
 *                          (from the table t_bios_asset_element_type)
 *
 * \return zmsg_t  - encoded ASEET_MSG_RETURN_LOCATION_FROM or 
 *                           COMMON_MSG_FAIL.
 */
zmsg_t* select_group_elements(
            const char* url             , uint32_t    element_id, 
            uint8_t     element_type_id , const char* group_name, 
            const char* dtype_name      , uint8_t     filtertype
        );


/**
 * \brief Select childs of specified type for the specified element 
 *  (element_id + element_type_id). 
 *  
 *  To select unlockated elements need to set element_id to 0.
 *  To select without the filter need to set a filtertype to 7.
 *
 * \param url             - connection to database
 * \param element_id      - id of the asset element
 * \param element_type_id - id of the type of the asset element
 * \param child_type_id   - type id of the child asset elements
 * \param is_recursive    - if the search recursive or not
 * \param current_depth   - a recursion parameter, started from 1
 * \param filter_type     - id of the type of the searched elements
 *
 * \return zframe_t - list of the childs of secified type according 
 *                    to the filter filter_type. (it is a Matryoshka).
 */
zframe_t* select_childs(
    const char* url             , uint32_t element_id, 
    uint8_t     element_type_id , uint8_t child_type_id, 
    bool        is_recursive    , uint32_t current_depth, 
    uint8_t     filtertype);


/*
 \brief Recursivly selects the parents of the element until the top 
 *  unlocated element.
 *
 * Generates the ASSET_MSG_RETURN_TOPOLOGY_TO message, but in inverse 
 * order (the specified element would be on the top level, but the top 
 * location would be at the bottom level);
 *
 * \param url             - the connection to database.
 * \param element_id      - the element id
 * \param element_type_id - id of the element's type
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or 
 *                      ASSET_MSG_RETURN_TOPOLOGY_TO
 */
zmsg_t* select_parents (const char* url, uint32_t element_id, 
                        uint8_t element_type_id);


zmsg_t *process_assettopology (const char *database_url, asset_msg_t **message_p);

#endif // SRC_PERSIST_ASSETTOPOLOGY_H_
