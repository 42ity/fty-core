#include "defs.h"
#include "data.h"
#include "asset_types.h"
#include "measure_types.h"
#include "common_msg.h"
#include "dbpath.h"
#include "monitor.h"

#include <algorithm>
#include <map>

byte asset_manager::type_to_byte(std::string type) {
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);
    byte ret = asset_type::UNKNOWN;
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
    } else if(type == "device") {
        ret = asset_type::DEVICE;
    }
    return ret;
}

std::string asset_manager::byte_to_type(byte type) {
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
        case asset_type::DEVICE:
            return "device";
        default:
            return "unknown";
    }
}

std::string measures_manager::int_to_type(uint16_t i) {
    char buff[16];
    zmsg_t *req = common_msg_encode_get_measure_type_i(i);
    zmsg_t *rep = process_measures_meta(&req);
    common_msg_t *dta = NULL;
    if((rep != NULL) && ((dta = common_msg_decode(&rep)) != NULL) &&
       (common_msg_id(dta) == COMMON_MSG_RETURN_MEASURE_TYPE)) {
        std::string ret = common_msg_mt_name(dta);
        common_msg_destroy(&dta);
        return ret;
    } else {
        sprintf(buff, "%d", i);
        zmsg_destroy(&rep);
        common_msg_destroy(&dta);
        return buff;
    }
}

std::string measures_manager::int_to_type(std::string i) {
    errno = 0;
    uint16_t id = strtol(i.c_str(), NULL, 10);
    if(errno != 0) {
        return i;
    } else {
        return int_to_type(id);
    }
}

std::string measures_manager::int_to_subtype(uint16_t i, uint16_t tid) {
    char buff[16];
    zmsg_t *req = common_msg_encode_get_measure_subtype_i(tid, i);
    zmsg_t *rep = process_measures_meta(&req);
    common_msg_t *dta = NULL;
    if((rep != NULL) && ((dta = common_msg_decode(&rep)) != NULL) &&
       (common_msg_id(dta) == COMMON_MSG_RETURN_MEASURE_SUBTYPE)) {
        std::string ret = common_msg_mts_name(dta);
        common_msg_destroy(&dta);
        return ret;
    } else if((dta != NULL) && (common_msg_id(dta) == COMMON_MSG_FAIL)) {
        common_msg_print(dta); 
    }
    sprintf(buff, "%d", i);
    zmsg_destroy(&rep);
    common_msg_destroy(&dta);
    return buff;
}

std::string measures_manager::int_to_subtype(std::string i, std::string t) {
    errno = 0;
    uint16_t id =  strtol(i.c_str(), NULL, 10);
    uint16_t tid = strtol(t.c_str(), NULL, 10);
    if(errno != 0) {
        return i;
    } else {
        return int_to_subtype(id, tid);
    }
}

std::string measures_manager::scale(std::string val, uint16_t i, uint16_t tid) {
    char buff[16];
    zmsg_t *req = common_msg_encode_get_measure_subtype_i(tid, i);
    zmsg_t *rep = process_measures_meta(&req);
    common_msg_t *dta = NULL;
    if((rep != NULL) && ((dta = common_msg_decode(&rep)) != NULL) &&
       (common_msg_id(dta) == COMMON_MSG_RETURN_MEASURE_SUBTYPE)) {
	/* The message passes an unsigned byte which we know to be
	 * really a _signed_ 8-bit value */
        int sc = (int8_t)common_msg_mts_scale(dta);

        common_msg_destroy(&dta);
        while(sc > 0) {
            val += "0";
            sc--;
        }
	/* Prepend zeroes - if needed - and add the period separator.
	 * Locale does not matter, this is for programmatic consumption */
        if(sc < 0) {
            sc += (int)(val.length());
            // We need to add some leading zeros
            if(sc <= 0) {
                while(sc < 0) {
                    val = "0" + val;
                    sc++;
                }
                val = "0." + val;
            // We just need to insert decimal point to the correct possition
            // -2 as letters are indexed from zero and insert adds after the index
            } else {
                val.insert(val.length()+sc-2, ".");
            }
        }
        return val;
    } else if((dta != NULL) && (common_msg_id(dta) == COMMON_MSG_FAIL)) {
        common_msg_print(dta); 
    }
    sprintf(buff, "%d", i);
    zmsg_destroy(&rep);
    common_msg_destroy(&dta);
    return val;
}


std::string measures_manager::scale(std::string val, std::string i, std::string t) {
    errno = 0;
    uint16_t id =  strtol(i.c_str(), NULL, 10);
    uint16_t tid = strtol(t.c_str(), NULL, 10);
    if(errno != 0) {
        return val;
    } else {
        return scale(val, id, tid);
    }
}

std::string measures_manager::map_names(std::string name) {
    static std::map<std::string, std::string> map = {
        { "temperature.ups", "ups.temperature" },
        { "status.ups", "ups.status" },
        { "load.ups", "ups.load" },
        { "realpower.default", "ups.realpower" },
        { "realpower.L1", "output.L1.realpower" },
        { "realpower.L2", "output.L2.realpower" },
        { "realpower.L3", "output.L3.realpower" },
        { "voltage.default", "output.voltage" },
        { "voltage.L1", "output.L1-N.voltage" },
        { "voltage.L2", "output.L2-N.voltage" },
        { "voltage.L3", "output.L3-N.voltage" },
    };
    
    auto it = map.find(name);
    if(it != map.end())
        return it->second;
    return name;
}

std::string measures_manager::map_values(std::string name, std::string value) {
    static std::map<std::string, std::map<std::string, std::string>> map = {
        { "status.ups", {
                { "0", "OFF" },
                { "1", "OL"  },
                { "2", "OL CHRG"  },
                { "3", "RB OL OFF"  },
                { "4", "OFF DISCHRG"  },
                { "5", "ALAM OL RB OFF"  },
            }
        }
    };
    
    auto it = map.find(name);
    if(it != map.end()) {
        auto it2 = it->second.find(value);
        if(it2 != it->second.end())
            return "\"" + it2->second + "\"";
    }
    return value;
}

std::string ui_props_manager::get(std::string& result) {

    //FIXME: where to put the constant?
    common_msg_t *reply = select_ui_properties(url.c_str());
    if (!reply)
        return std::string("{\"error\" : \"Can't load ui/properties from database!\"}");

    uint32_t msg_id = common_msg_id(reply);

    if (msg_id == COMMON_MSG_FAIL) {
        auto ret = std::string{"{\"error\" : \""};
        ret.append(common_msg_errmsg(reply));
        ret.append("\"}");
        common_msg_destroy(&reply);
        return ret;
    }
    else if (msg_id != COMMON_MSG_RETURN_CINFO) {
        common_msg_destroy(&reply);
        return std::string("{\"error\" : \"Unexpected msg_id delivered, expected COMMON_MSG_RETURN_CINFO\"}");
    }

    zmsg_t *zmsg = common_msg_get_msg(reply);
    if (!zmsg) {
        common_msg_destroy(&reply);
        return std::string("{\"error\" : \"Can't extract inner message from reply!\"}");
    }

    common_msg_t *msg = common_msg_decode(&zmsg);
    common_msg_destroy(&reply);
    if (!msg)
        return std::string("{\"error\" : \"Can't decode inner message from reply!\"}");
    
    zchunk_t *info = common_msg_get_info(msg);
    common_msg_destroy(&msg);
    if (!info)
        return std::string("{\"error\" : \"Can't get chunk from reply!\"}");

    char *s = zchunk_strdup(info);
    zchunk_destroy(&info);
    if (!s)
        return std::string("{\"error\" : \"Can't get string from reply!\"}");
    
    result = s;
    free(s);

    return std::string{};
}

std::string ui_props_manager::put(const std::string& ext) {

    const char* s = ext.c_str();

    zchunk_t *chunk = zchunk_new(s, strlen(s));
    if (!chunk)
        return std::string("fail to create zchunk");

    //FIXME: where to store client_id?
    common_msg_t *reply = update_client_info(url.c_str(), UI_PROPERTIES_CLIENT_ID, &chunk);
    uint32_t msg_id = common_msg_id(reply);
    common_msg_destroy(&reply);

    if (msg_id != COMMON_MSG_DB_OK)
        return std::string("msg_id != COMMON_MSG_DB_OK");

    return "";
}

