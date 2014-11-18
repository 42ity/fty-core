#include <czmq.h>

#include <stdlib.h>

#include "data.h"
#include "asset_msg.h"
#include "common_msg.h"
#include "log.h"

#include <cxxtools/directory.h>
#include <cxxtools/regex.h>
#include "dbpath.h"
#include "assetmsg.h"

zmsg_t *asset_manager::get_item(std::string type, std::string id) {
    byte real_type = asset_manager::type_to_byte(type);
    if(real_type == (byte)asset_type::UNKNOWN) {
        return NULL;
    }
    uint32_t real_id = atoi(id.c_str());
    if(real_id == 0) {
        return NULL;
    }
    asset_msg_t *get_element = asset_msg_new(ASSET_MSG_GET_ELEMENT);
    asset_msg_set_type(get_element, real_type);
    asset_msg_set_element_id(get_element, real_id);
    asset_msg_print(get_element);

    // Currently not needed
    zmsg_t *ret = asset_msg_process (url.c_str(), get_element);
    asset_msg_destroy (&get_element);
    /*
        FILE *fl = fopen(("data/" + type + "/" + id).c_str(), "r");
        asset_msg_t *ret;

        if(fl == NULL) {
            return NULL;
        }
        zmsg_t *msg = zmsg_load(NULL, fl);
        fclose(fl);

        if(msg == NULL) {
            return NULL;
        }

        ret = asset_msg_decode(&msg);
    */
    // vratim return_element anebo fail
    assert(ret != NULL);
    if (is_common_msg(ret) ) {  
        return ret;          // it can be only COMMON_MSG_FAIL
    }
    // ASSET_RETURN_ELEMENT
    asset_msg_t* msg = asset_msg_decode (&ret);
    assert ( msg != NULL );
    asset_msg_print (msg);

    ret = asset_msg_get_msg (msg);
    assert ( ret != NULL );
    asset_msg_destroy (&msg);
    assert ( ret != NULL );
    return ret;
}

zmsg_t *asset_manager::get_items(std::string type) {
    byte real_type = asset_manager::type_to_byte(type);
    if ( real_type == (byte)asset_type::UNKNOWN ) {
        return NULL;
    }

    asset_msg_t *get_elements = asset_msg_new (ASSET_MSG_GET_ELEMENTS);
    asset_msg_set_type (get_elements, real_type);
    asset_msg_print (get_elements);

    // Currently not needed
    zmsg_t *ret = asset_msg_process (url.c_str(), get_elements);
    asset_msg_destroy (&get_elements);
    /*
        asset_msg_t *ret = asset_msg_new(ASSET_MSG_RETURN_ELEMENTS);
        cxxtools::Directory dir("data/" + type);
        cxxtools::Regex reg("^[0-9]+$");

        for(auto it = dir.begin(); it != dir.end(); ++it) {
            if(reg.match(*it)) {
                asset_msg_element_ids_insert(ret, it->c_str(), "TBD");
            }
        }
    */

    zmsg_print(ret);
    return ret;
}
