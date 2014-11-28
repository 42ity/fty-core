#ifndef SRC_PERSIST_ASSETTOPOLOGY_H_
#define SRC_PERSIST_ASSETTOPOLOGY_H_

#include "asset_msg.h"

zframe_t* select_childs ( const char* url,          uint32_t element_id, 
                          uint32_t element_type_id, uint32_t child_type_id, 
                          bool is_recursive,        uint32_t current_depth, 
                          uint32_t filtertype);

zmsg_t* select_parents (const char* url, uint32_t element_id, uint32_t element_type_id);

void print_frame (zframe_t* frame);
void print_frame_devices (zframe_t* frame);

zmsg_t* get_return_topology_to (const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_topology_from (const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_power_topology_to(const char* url, asset_msg_t* getmsg);
#endif // SRC_PERSIST_ASSETTOPOLOGY_H_
