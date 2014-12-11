#ifndef SRC_PERSIST_ASSETTOPOLOGY_H_
#define SRC_PERSIST_ASSETTOPOLOGY_H_

#include "asset_msg.h"

typedef std::set<std::tuple<int,int,std::string,std::string,int,int,std::string,std::string>> edge_lf;

zframe_t* select_childs ( const char* url,          uint32_t element_id, 
                          uint32_t element_type_id, uint32_t child_type_id, 
                          bool is_recursive,        uint32_t current_depth, 
                          uint32_t filtertype);

zmsg_t* select_parents (const char* url, uint32_t element_id, uint32_t element_type_id);

zmsg_t* select_group_elements(const char* url, uint32_t element_id, 
        uint8_t element_type_id, const char* group_name, 
        const char* dtype_name, uint8_t filtertype);

edge_lf print_frame_to_edges (zframe_t* frame, uint32_t parent_id, int type, std::string name, std::string dtype_name);

void print_frame (zframe_t* frame,uint32_t parent_id);
void print_frame_devices (zframe_t* frame);
bool compare_start_element (asset_msg_t* rmsg, uint32_t id, uint8_t id_type, const char* name, const char* dtype_name);

zmsg_t *process_assettopology (const char *database_url, asset_msg_t **message_p);

zmsg_t* get_return_topology_to (const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_topology_from (const char* url, asset_msg_t* getmsg);


zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_power_topology_to(const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_power_topology_group(const char* url, asset_msg_t* getmsg);
zmsg_t* get_return_power_topology_datacenter(const char* url, asset_msg_t* getmsg);


#endif // SRC_PERSIST_ASSETTOPOLOGY_H_
