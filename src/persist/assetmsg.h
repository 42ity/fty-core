#ifndef SRC_PERSIST_ASSETMSG_H_
#define SRC_PERSIST_ASSETMSG_H_

#include "defs.h"
#include "asset_msg.h"

#define MAX_NAME_LENGTH 25 // For now it the maximum length of all fields nam
#define MAX_DESCRIPTION_LENGTH 255

/**
 * \brief Processes message of type asset_msg_t
 *
 * Broken down processing of generic database zmsg_t, this time asset message
 * case.
 */
zmsg_t *asset_msg_process(zmsg_t **msg);

//---------------
//internal functions for processing the asset_messages

zlist_t* select_asset_element_groups(const char* url, unsigned int element_id);

zlist_t* select_asset_device_link(const char* url, unsigned int device_id, unsigned int link_type_id);

zhash_t* select_asset_element_attributes(const char* url, unsigned int element_id);
zmsg_t* select_asset_device(const char* url, asset_msg_t** element, uint32_t element_id);
zmsg_t* select_asset_element(const char* url, unsigned int element_id, unsigned int element_type_id);
zmsg_t* get_asset_element(const char *url, asset_msg_t *msg);
zmsg_t* get_asset_elements(const char *url, asset_msg_t *msg);
uint32_t convert_asset_to_monitor(const char* url, uint32_t asset_element_id);
uint32_t convert_monitor_to_asset(const char* url, uint32_t discovered_device_id);
//
#endif // SRC_PERSIST_ASSETMSG_H_

