#include "data.h"
#include "asset_types.h"

#include <algorithm>

byte asset_manager::type_to_byte(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    if(type == "datacenter") {
        return DATACENTER;
    } else if(type == "room") {
        return ROOM;
    } else if(type == "row") {
        return ROW;
    } else if(type == "rack") {
        return RACK;
    }
}

std::string asset_manager::byte_to_type(byte type) {
    switch(type) {
        case DATACENTER:
            return "datacenter";
        case ROOM:
            return "room";
        case ROW:
            return "row";
        case RACK:
            return "rack";
    }
}
