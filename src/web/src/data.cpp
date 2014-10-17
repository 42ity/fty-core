#include "data.h"
#include "asset_types.h"

#include <algorithm>

asset_type asset_manager::type_to_byte(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if(type == "datacenter") {
        return asset_type::DATACENTER;
    } else if(type == "room") {
        return asset_type::ROOM;
    } else if(type == "row") {
        return asset_type::ROW;
    } else if(type == "rack") {
        return asset_type::RACK;
    } else if(type == "group") {
        return asset_type::GROUP;
    }
}

std::string asset_manager::byte_to_type(asset_type type) {
    switch(type) {
        case asset_type::DATACENTER:
            return "datacenter";
        case asset_type::ROOM:
            return "room";
        case asset_type::ROW:
            return "row";
        case asset_type::RACK:
            return "rack";
        case asset_type::GROUP:
            return "group";
    }
}
