#ifndef SRC_SHARED_ASSET_TYPES_H
#define SRC_SHARED_ASSET_TYPES_H

namespace asset_type {

enum asset_type {
    UNKNOWN     = 0,
    GROUP       = 1,
    DATACENTER  = 2,
    ROOM        = 3,
    ROW         = 4,
    RACK        = 5,
    DEVICE      = 6
};

};

#endif //SRC_SHARED_ASSET_TYPES_H
