#include <czmq.h>

#include <stdlib.h>

#include "data.h"
#include "asset_msg.h"
#include "common_msg.h"
#include "log.h"

#include <cxxtools/directory.h>
#include <cxxtools/regex.h>
#include "persistencelogic.h"

zmsg_t *asset_manager::get_item(std::string type, std::string id) {
    log_debug("Trying to get element %s of type %s", id.c_str(), type.c_str());
    byte real_type = asset_manager::type_to_byte(type);
    if(real_type == (byte)asset_type::UNKNOWN) {
        return NULL;
    }
    uint32_t real_id = atoi(id.c_str());
    if(real_id == 0) {
        return NULL;
    }
    zmsg_t *get_element = asset_msg_encode_get_element(real_id, real_type);
    zmsg_t *ret = persist::process_message(&get_element);
    zmsg_destroy(&get_element);
    assert(ret != NULL);
    if (is_common_msg(ret) ) {
        return ret;          // it can be only COMMON_MSG_FAIL
    }
    // Return directly element message which is packed inside return element
    asset_msg_t* msg = asset_msg_decode(&ret);
    if(msg == NULL) {
        log_error("Decoding reply from persistence failed!");
        return NULL;
    }

    ret = asset_msg_get_msg(msg);
    asset_msg_destroy(&msg);
    return ret;
}

zmsg_t *asset_manager::get_items(std::string type) {
    log_debug("Trying to get elements of type %s", type.c_str());
    byte real_type = asset_manager::type_to_byte(type);
    if ( real_type == (byte)asset_type::UNKNOWN ) {
        return NULL;
    }

    zmsg_t *get_elements = asset_msg_encode_get_elements(real_type);
    zmsg_t *ret = persist::process_message(&get_elements);
    zmsg_destroy(&get_elements);

    return ret;
}
