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

/*! \file assettopology.h
    \brief Functions for getting the topology (location and power) from the
           database.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_ASSETTOPOLOGY_H_
#define SRC_PERSIST_ASSETTOPOLOGY_H_
#include <set>
#include <tuple>
#include "asset_msg.h"

// ===============================================================
// Helper types
// ===============================================================

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
typedef std::tuple< uint32_t, std::string, uint32_t, std::string > powerlink_info_t;

// ===============================================================
// Functions for processing a special message type
// ===============================================================

/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_FROM message.
 *
 * For the correct message processing all message fields should be set up
 * according specification.
 * 
 * To select unlockated elements need to set element_id to 0.
 * For unlockated elements only a non recursive search is provided.
 * To select without the filter need to set a filtertype to 7.
 *
 * In case of success it generates ASSET_MSG_RETURN_LOCATION_FROM message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * It doesn't destroy the getmsg.
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_FROM message.
 */
zmsg_t* get_return_topology_from(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_TO message.
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 *
 * In case of success it generates ASSET_MSG_RETURN_LOCATION_TO messsage.
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_TO 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_TO message.
 */
zmsg_t* get_return_topology_to(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_FROM message.
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 *
 * In case of success it generates ASSET_MSG_RETURN_POWER message.
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * Can not distinguish multiple links from device B to device D if
 * src_out and dest_in are not specified.
 *
 * A single powerchain link is encoded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is SRCOUT_DESTIN_IS_NULL then A or C was not srecified in database (was NULL). 
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER message.
 */ 
zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_TO message
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 *
 * In case of success it generates ASSET_MSG_RETURN_POWER message.
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * Can not distinguish multiple links from device B to device C if
 * src_out and dest_in are not specified.
 *
 * A single powerchain link is encoded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is SRCOUT_DESTIN_IS_NULL then A or C was not srecified in database (was NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_TO
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER message.
 */ 
zmsg_t* get_return_power_topology_to (const char* url, asset_msg_t* getmsg);


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_GROUP message
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 *
 * In case of success it generates ASSET_MSG_RETURN_POWER message.
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * Returns all devices in the group and returns all power links between them.
 * Links that goes outside the group are not returned.
 *
 * Can not distinguish multiple links from device B to device C if
 * src_out and dest_in are not specified.
 *
 * A single powerchain link is encoded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is SRCOUT_DESTIN_IS_NULL then A or C was not srecified in database (was NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_GROUP 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER message.
 */ 
zmsg_t* get_return_power_topology_group(const char* url, asset_msg_t* getmsg);


/*
 * \brief This function processes the ASSET_MSG_GET_POWER_DATACENTER message
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 *
 * In case of success it generates ASSET_MSG_RETURN_POWER message.
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * Returns all devices in datacenter and all powerlinks between them.
 * Links outside the datacenter are not returned.
 *
 * Can not distinguish multiple links from device B to device C if
 * src_out and dest_in are not specified.
 *
 * A single powerchain link is encoded as string according to the 
 * convert_powerchain_powerlink2list function. 
 *
 * \param url    - the connection to database.
 * \param getmsg - the message of the type ASSET_MSG_GET_POWER_DATACENTER
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_DATACENTER message
 */ 
zmsg_t* get_return_power_topology_datacenter(const char* url, 
                                    asset_msg_t* getmsg);

// ===============================================================
// Helper functions and types for testing
// ===============================================================

/**
 * \brief Helper type for testing
 *  
 *  First  -- CHILD:asset element id
 *  Second -- CHILD:device type id
 *  Third  -- CHILD:device name
 *  Forth  -- CHILD:device type name
 *  Fifth  -- PARENT:asset element id
 *  Sixth  -- PARENT:device type id
 *  Sevens -- PARENT:device name
 *  Eighth -- PARENT:device type name
 */
typedef std::set < std::tuple < int, int, std::string, std::string,
                                int, int, std::string, std::string>> edge_lf;


/**
 * \brief Helper function for testing.
 * 
 * Prints a frame of devices (from ASSET_MSG_POWER_FROM message).
 *
 * \param frame - frame to print.
 */
void print_frame_devices (zframe_t* frame);


/**
 * \brief Helper function for testing.
 * 
 * prints a frame (from ASSET_MSG_LOCATION_FROM message).
 *
 * \param frame      - frame to print.
 * \param parent_id  - id (asset_elememt_id) of the parent.
 * 
 */
void print_frame (zframe_t* frame, uint32_t parent_id);


/**
 * \brief Helper function for testing.
 * 
 * Converts a frame (from ASSET_MSG_LOCATION_FROM message) into 
 * the std::set of edges in the tree (child,parent);
 *
 * \param frame      - frame to convert.
 * \param parent_id  - id (asset_elememt_id) of the root.
 * \param id_type    - id of the type of the root.
 * \param name       - name of the root compared to.
 * \param dtype_name - name of the precise element type of root. 
 *                      (available only for groups and devices).
 * 
 * \return std::set of edges.
 */
edge_lf print_frame_to_edges (zframe_t* frame, uint32_t parent_id, 
                uint8_t type, std::string name, std::string dtype_name);


/* \brief Helper function for testing.
 *  
 * Compares the element in the ASSET_MSG_RETURN_LOCATION_FROM or in 
 * the ASSET_MSG_RETURN_LOCATION_TO field by field with specified values.
 *
 * \param rmsg       - message with element.
 * \param id         - id (asset_elememt_id) of the element compared to.
 * \param id_type    - id of the type of the element compared to.
 * \param name       - name of the element compared to.
 * \param dtype_name - name of the precise element type of the element 
 *                      compared to (available only for groups and devices).
 *
 * \return true  - if the element is the same.
 *         false - if elements are different.
 */
bool compare_start_element (asset_msg_t* rmsg, uint32_t id, uint8_t id_type,
                            const char* name, const char* dtype_name);

// ===============================================================
// Helper functions for direct interacting with database
// ===============================================================

/**
 * \brief Selects group elements of specified type for the specified group.
 *  
 *  Paramenters "element_type_id" and "group_name" are used only in the 
 *  returned zmsg_t message.
 * 
 *  In case of success it generates ASEET_MSG_RETURN_LOCATION_FROM message.
 *  In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url             - connection to database.
 * \param element_id      - asset element id of the group.
 * \param element_type_id - asset_element_type_id of the group.
 * \param group_name      - name of the group.
 * \param filter_type     - id of the type of the searched elements
 *                          (from the table t_bios_asset_element_type).
 *
 * \return zmsg_t  - encoded ASEET_MSG_RETURN_LOCATION_FROM or 
 *                           COMMON_MSG_FAIL message.
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
 * \param url             - connection to database.
 * \param element_id      - id of the asset element.
 * \param element_type_id - id of the type of the asset element.
 * \param child_type_id   - type id of the child asset elements.
 * \param is_recursive    - if the search recursive or not.
 * \param current_depth   - a recursion parameter, started from 1.
 * \param filter_type     - id of the type of the searched elements.
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
 * location would be at the bottom level).
 *
 * \param url             - the connection to database.
 * \param element_id      - the element id.
 * \param element_type_id - id of the element's type.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or 
 *                      ASSET_MSG_RETURN_TOPOLOGY_TO message.
 */
zmsg_t* select_parents (const char* url, uint32_t element_id, 
                        uint8_t element_type_id);

/**
 * \brief Selects a name of the specified asset element, in case of device 
 * additionally selects device type name.
 *
 * If the specified asset element is not a device, then a device type name 
 * would be an empty string.
 *
 * Throws exceptions: tntdb::NotFound - in case if no rows were selected.
 *                    std::exception  - in case of any other error.
 *                     
 * \param url              - the connection to database.
 * \param asset_element_id - id of the asset element.
 *
 * \return  A tuple: 
 *              First  - name of the asset element.
 *              Second - device type name.
 *              Third  - device type id.
 */
std::tuple <std::string, std::string, uint32_t>
    select_add_device_info  ( const char* url, 
                              uint32_t asset_element_id);


/**
 * \brief Selects devices and powerlinks for "power topology to"
 * from the specified start element.
 *
 * Throws exceptions: bios::NotFound - in case start element was not found.
 *                    bios::ElementIsNotDevice - in case start lement is 
 *                                      not a device.
 *                    bios::InternalDBError - in case of any database errors.
 *                    std::exception  - in case of any other error.
 *
 * \param url          - the connection to database.
 * \param element_id   - asset element id of the start element.
 * \param linktype     - id of the linktype.
 * \param is_recursive - if the search is recursive (selects all levels)
 *                           or not recursive (selects only 1 level).
 *
 * \return  A pair of sets: 
 *              First  - set of devices.
 *              Second - set of powerlinks.
 */
std::pair < std::set < device_info_t >, std::set < powerlink_info_t > > 
    select_power_topology_to (const char* url, uint32_t element_id, 
                              uint8_t linktype, bool is_recursive);

// ===============================================================
// Helper functions 
// ===============================================================

/**
 * \brief Converts a set of device_info_t elements into matryoshka.
 *
 * \param devices - set of device_info_t elements.
 *
 * \return zmsg_t - an encoded matryoshka of ASSET_MSG_POWERCHAIN_DEVICE 
 *                  messages.
 */
zmsg_t* convert_powerchain_devices2matryoshka (
                        std::set <device_info_t > const &devices);

/**
 * \brief Converts a set of powerlink_info_t elements into list.
 *
 * Every element in the list is a string
 * "A:B:C:D"="src_socket:src_id:dst_socket:dst_id".
 * If A or C is SRCOUT_DESTIN_IS_NULL then A or C was not srecified in database (was NULL). 
 *
 * \param powerlinks - set of powerlink_info_t elements.
 *
 * \return zlist_t - link of strings "A:B:C:D".
 */
zlist_t* convert_powerchain_powerlink2list (
                        std::set < powerlink_info_t > const &powerlinks);

/**
 * \brief Generates an ASSET_MSG_RETURN_POWER message from the given arguments.
 *
 * Both argumens would be left untouched.
 * 
 * \param devices    - a set of device_info_t elements .
 * \param powerlinks - a set of powerlink_info_t elements.
 *
 * \return zmsg_t - an encoded ASSET_MSG_RETURN_POWER message.
 */
zmsg_t* 
    generate_return_power (std::set < device_info_t >    const &devices, 
                           std::set < powerlink_info_t > const &powerlinks);

/**
 * \brief Converts a matryoshka mmsg into frame.
 *
 * Destroyes matryosshka.
 * 
 * \param matryoshka - a matryoshka msg to convert.
 * \param frame      - an output parameter: frame.
 *
 * \return 0  in case of success.
 *         -2 in case of failure.
 */
int matryoshka2frame (zmsg_t **matryoshka, zframe_t **frame );

/**
 * \brief Helper function.
 *
 * Calculates zframe_t size even for NULL value.
 *
 * \param frame - frame.
 *
 * \return size of the frame.
 */
size_t my_size(zframe_t* frame);

// ===============================================================
// Function for processing assettopology messages
// ===============================================================

zmsg_t* 
    process_assettopology (const char *database_url, asset_msg_t **message_p);

#endif // SRC_PERSIST_ASSETTOPOLOGY_H_
