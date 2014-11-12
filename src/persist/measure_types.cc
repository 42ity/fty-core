#include <zmq.h>
#include <czmq.h>
#include <exception>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb.h>

#include "measure_types.h"
#include "common_msg.h"
#include "dbpath.h"

zmsg_t* process_measures_meta(zmsg_t** zmsg) {
    if(is_common_msg(*zmsg) != true)
        return common_msg_encode_fail(0,0,"invalid message",NULL);

    common_msg_t *msg = common_msg_decode(zmsg);
    return process_measures_meta(&msg);
}

zmsg_t* process_measures_meta(common_msg_t** msg) {
    zmsg_t* ret = NULL;
    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
    } catch (const std::exception &e) {
        ret = common_msg_encode_fail(0,0,e.what(),NULL);
    }
    tntdb::Row row;

    if(ret == NULL)
    switch(common_msg_id(*msg)) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, name from t_bios_measurement_types "
                    "where id = :mt_id");

                row = st.setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                common_msg_destroy(msg);
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }

        case COMMON_MSG_GET_MEASURE_TYPE_S:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, name from t_bios_measurement_types "
                    "where name = :name");

                row = st.setString("name", common_msg_mt_name(*msg)).
                      selectRow();
                common_msg_destroy(msg);
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, mt_id, name, scale "
                    "from t_bios_measurement_subtypes where id = :mts_id");

                row = st.setInt("mts_id", common_msg_mts_id(*msg)).selectRow();
                common_msg_destroy(msg);
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, mt_id, name, scale "
                    "from t_bios_measurement_subtypes where name = :name");

                row = st.setString("name", common_msg_mts_name(*msg)).
                      selectRow();
                common_msg_destroy(msg);
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }

        default:
            ret = common_msg_encode_fail(0,0,"invalid message",NULL);
    }

    common_msg_destroy(msg);
    return ret;
}

