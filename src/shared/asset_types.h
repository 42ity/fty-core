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

enum asset_subtype {
    UPS = 1,
    GENSET,
    EPDU,
    PDU,
    SERVER,
    MAIN,
    STS,
    SWITCH,
    STORAGE,
    N_A = 10
};

enum asset_operation {
    INSERT = 1,
    DELETE,
    UPDATE,
    GET
};

};

#endif //SRC_SHARED_ASSET_TYPES_H
