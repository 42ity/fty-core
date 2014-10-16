#include <czmq.h>

#include "data.h"
#include "asset_msg.h"

#include <cxxtools/directory.h>
#include <cxxtools/regex.h>

asset_msg_t *asset_manager::get_item(std::string type, std::string id) {
    FILE *fl = fopen(("data/" + type + "/" + id).c_str(), "r");
    asset_msg_t *ret;

    if (fl == NULL) {
        return NULL;
    }
    zmsg_t *msg = zmsg_load(NULL, fl);
    fclose(fl);

    if (msg == NULL) {
        return NULL;
    }

    ret = asset_msg_decode(&msg);
    zmsg_destroy(&msg);
    msg = asset_msg_get_msg(ret);
    asset_msg_destroy(&ret);
    ret = asset_msg_decode(&msg);
    zmsg_destroy(&msg);

    if (ret != NULL) {
        return ret;
    }

    zmsg_destroy(&msg);
    return NULL;
}

asset_msg_t *asset_manager::get_items(std::string type) {
    asset_msg_t *ret = asset_msg_new(ASSET_MSG_RETURN_ELEMENTS);
    cxxtools::Directory dir("data/" + type);
    cxxtools::Regex reg("^[0-9]+$");

    for (auto it = dir.begin(); it != dir.end(); ++it) {
        if (reg.match(*it)) {
            asset_msg_elemenet_ids_insert(ret, it->c_str(), "TBD");
        }
    }
    return ret;
}
