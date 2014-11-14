#ifndef SRC_PERSIST_ASSETMSG_H_
#define SRC_PERSIST_ASSETMSG_H_

#include "asset_msg.h"

#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5
#define BIOS_ERROR_DB 1

#define MAX_NAME_LENGTH 25 // For now it the maximum length of all fields nam
#define MAX_DESCRIPTION_LENGTH 255

asset_msg_t *asset_msg_process(const char *url, asset_msg_t *msg);
void* void_dup(const void* a); 


//---------------
//internal functions for processing the asset_messages

zlist_t* _select_asset_element_groups(const char* url, unsigned int element_id);

zlist_t* _select_asset_device_link(const char* url, unsigned int device_id, unsigned int link_type_id);
asset_msg_t* generate_fail(unsigned int errorid);

zhash_t* _select_asset_element_attributes(const char* url, unsigned int element_id);
asset_msg_t* _select_asset_device(const char* url, asset_msg_t** element);
asset_msg_t* _select_asset_element(const char* url, unsigned int element_id, unsigned int element_type_id);
asset_msg_t* _get_asset_element(const char *url, asset_msg_t *msg);
asset_msg_t* _get_asset_elements(const char *url, asset_msg_t *msg);
uint32_t convert_asset_to_monitor(const char* url, uint32_t asset_element_id);
uint32_t convert_monitor_to_asset(const char* url, uint32_t discovered_device_id);
//
#endif // SRC_PERSIST_ASSETMSG_H_

