#include <cstdio>
#include <cstring>
#include <exception>
#include <algorithm>

#include "asset_types.h"

#include "location_helpers.h"

int process_return_location_from (asset_msg_t **message_p, std::string& json) {
        /*
    log_open ();
    log_set_level (LOG_DEBUG);

    assert (message_p);
    if (message_p == NULL) {
        return -1;
    }
    if (*message_p) {
        asset_msg_t *message = *message_p;
        if (message == NULL) {
            return -1;
        }


        

    }
    else {
        log_error ("Pointer to null pointer passed as second argument  'asset_msg_t **message_p'.");
        return_msg = common_msg_encode_fail (0, 0,"Invalid asset message: Pointer to null pointer passed as second argument.", NULL);
        assert (return_msg);
        assert (is_common_msg (return_msg));
    }
    assert (return_msg); // safeguard non-NULL return value
    log_close ();
    */
    return true;    
}

bool build_return_location_from (asset_msg_t *message, std::string& json) {
        return true;
}


// Note: in the future, if more than one operation requires parsing these strings on input,
//       we should consider moving this to general helpers.cpp file and rewrite these fun-
//       ctions for a more general case
//       e.g. namespace shared:: src/include/utils.h src/shared/utils.c and have asset_types
//       in src/include. There is no need to wrap a class around simple functions.

// TODO:case "api/v1/asset/5" (missing element type, i.e. room, datacenter...) not covered
int element_id (const std::string& from, int& element_id) {
    if (from.empty()) {
        return -1;
    }
    if (from.find ("api/v1/asset/") != 0) {
        return -1;
    }
    std::size_t last = from.find_last_of ("/");
    std::string number = from.substr (last + 1, from.length() - last);
    if (number.length () == 0) {
        return -1;
    }
    try {
        element_id = std::stoi (number);
    } catch (std::exception& e) {
        return -1;
    }
    return 0;
}


int asset (const std::string& from) {
    if (from.empty()) {
        return -1;
    }
    if (from.find ("api/v1/asset/") != 0) {
        return -1;
    }
    std::size_t pos = strlen ("api/v1/asset/");   
    std::string cut = from.substr (pos, from.length () - pos);
    if (cut.empty()) {
        return -1;
    }
    pos = cut.find_first_of ("/");
    if (pos == 0) {
        return -1;
    }
    cut = cut.substr(0, pos);

    int ret = asset_type::UNKNOWN;
    if (cut == "datacenter") {
        ret = asset_type::DATACENTER;
    } else if (cut == "room") {
        ret = asset_type::ROOM;
    } else if (cut == "row") {
        ret = asset_type::ROW;
    } else if (cut == "rack") {
        ret = asset_type::RACK;
    } else if (cut == "group") {
        ret = asset_type::GROUP;
    } else if (cut == "device") {
        ret = asset_type::DEVICE;
    }
    return ret;
}

