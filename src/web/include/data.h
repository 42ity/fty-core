#ifndef WEB_DATA_H
#define WEB_DATA_H

#include <czmq.h>
#include <string>

#include "asset_msg.h"
#include "asset_types.h"

class asset_manager {
    public:
        zmsg_t* get_item(std::string type, std::string id);
        zmsg_t* get_items(std::string type);
        static byte type_to_byte(std::string type);
        static std::string byte_to_type(byte type);
};

class measures_manager {
    public:
        static std::string int_to_type(std::string i);
        static std::string int_to_subtype(std::string i, std::string t);
        static std::string scale(std::string val, std::string i, std::string t);
        static std::string int_to_type(uint16_t i);
        static std::string int_to_subtype(uint16_t i, uint16_t tid);
        static std::string scale(std::string val, uint16_t i, uint16_t tid);
};

#endif // WEB_DATA_H
