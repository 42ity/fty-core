#include "ymsg.h"
#include "log.h"
#include "dbpath.h"
#include "bios_agent.h"

#include <tntdb.h>
#include <errno.h>
#include <zchunk.h>

void get_measurements(ymsg_t* out, char** out_subj,
                      ymsg_t* in, const char* in_subj) {
    std::string json;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            "SELECT topic,value,scale,UNIX_TIMESTAMP(timestamp),units "
            "FROM v_bios_measurement "
            "WHERE "
            "topic_id IN "
            "   (SELECT t.id FROM t_bios_measurement_topic AS t, "
            "                     t_bios_monitor_asset_relation AS rel "
            "    WHERE rel.id_asset_element = :id AND "
            "          t.device_id = id_discovered_device AND "
            "          t.topic LIKE :topic) AND "
            "timestamp > FROM_UNIXTIME(:time_st) AND "
            "timestamp < FROM_UNIXTIME(:time_end) "
            "ORDER BY timestamp ASC "
        );
        std::string topic = ymsg_get_string(in,"source");
        topic += "@%";
        errno = 0;
        st.set("id", ymsg_get_int64(in,"element_id"))
          .set("topic", topic)
          .set("time_st", ymsg_get_int64(in,"time_st"))
          .set("time_end", ymsg_get_int64(in,"time_end"));
        if(errno != 0)
            throw std::invalid_argument("Message decoding error");
        tntdb::Result result = st.select();
        std::string units;
        for(auto &row: result) {
            if(!json.empty()) {
                json += ",\n";
            }
            if(units.empty())
                units = row[4].getString();
            json += " {";
            json += "   \"value\": " + std::to_string(row[1].getInt32()) +  ",";
            json += "   \"scale\": " + std::to_string(row[2].getInt32()) +  ",";
            json += "   \"timestamp\": " + std::to_string(row[3].getInt64());
            json += " }";
        }
        json = "{ \"units\": \"" + units + "\",\n" +
               "  \"source\": \"" + ymsg_get_string(in,"source") + "\",\n" +
               "  \"element_id\": " + ymsg_get_string(in,"element_id") + ",\n" +
               "\"data\": [\n" + json + "\n] }";
    } catch(const std::exception &e) {
        log_error("Run into '%s' while getting measurements", e.what());
        return;
    }
    zchunk_t *ch = zchunk_new(json.c_str(), json.length());
    if(ch != NULL) {
        ymsg_set_response(out, &ch);
        (*out_subj) = strdup("return_measurements");
    }
}
