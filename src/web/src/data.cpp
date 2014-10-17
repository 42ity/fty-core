#include "data.h"
#include "asset_types.h"

#include <algorithm>

byte asset_manager::type_to_byte(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    asset_type ret = asset_type::UNKNOWN;
    if(type == "datacenter") {
        ret = asset_type::DATACENTER;
    } else if(type == "room") {
        ret = asset_type::ROOM;
    } else if(type == "row") {
        ret = asset_type::ROW;
    } else if(type == "rack") {
        ret = asset_type::RACK;
    } else if(type == "group") {
        ret = asset_type::GROUP;
    }
    return (byte)ret;
}

std::string asset_manager::byte_to_type(byte type) {
    asset_type conv = (asset_type)type;
    return asset_manager::byte_to_type(conv);
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
