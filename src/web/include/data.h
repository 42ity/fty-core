#include <czmq.h>
#include <string>

#include "asset_msg.h"
#include "asset_types.h"

class asset_manager {
    public:
        asset_msg_t* get_item(std::string type, std::string id);
        asset_msg_t* get_items(std::string type);
        asset_type type_to_byte(std::string type);
        std::string byte_to_type(asset_type type);
};
