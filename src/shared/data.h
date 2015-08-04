#ifndef SRC_SHARED_DATA_H
#define SRC_SHARED_DATA_H

#include <czmq.h>
#include <string>

#include "defs.h"
#include "asset_types.h"
#include "dbhelpers.h"
#include "common_msg.h"
#include "db/assets.h"

/**
 * \brief extract error, msg and HTTP error code from common_msg instance
 *
 */
void common_msg_to_rest_error(common_msg_t* cm_msg, std::string& error, std::string& msg, unsigned* code);

class asset_manager {
    public:
        // new functionality
        db_reply <db_web_element_t>  get_item1(const std::string &id);
        db_reply <std::map <uint32_t, std::string> > get_items1(const std::string &typeName);
        
        // to support old style
        db_reply <db_web_element_t>  get_item1(const std::string &id, const std::string &type);

        static byte type_to_byte(std::string type);
        static std::string byte_to_type(byte type);
};

class measures_manager {
    public:
        std::string map_names(std::string name);
        std::string map_values(std::string name, std::string value);
        std::string apply_scale(const std::string &val, const std::string &scale);
};

class ui_props_manager {
    public:
        std::string get(std::string& result);
        std::string put(const std::string& props);
};

#endif // SRC_SHARED_DATA_H
