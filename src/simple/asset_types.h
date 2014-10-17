#ifndef ASSET_TYPES_H
#define ASSET_TYPES_H

enum class asset_type : byte {
    UNKNOWN = 0,
    DATACENTER,
    ROOM,
    ROW,
    RACK,
    GROUP,
    DEVICE
};

#endif //ASSET_TYPES_H
