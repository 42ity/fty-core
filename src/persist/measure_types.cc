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
#include "defs.h"
#include "log.h"

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
        ret = common_msg_encode_fail(DB_ERR,DB_ERROR_CANTCONNECT,e.what(),NULL);
    }
    tntdb::Row row;

    if(ret == NULL)
    switch(common_msg_id(*msg)) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, name, unit from t_bios_measurement_types "
                    "where id = :mt_id "
                    "and id is not null and name is not null");

                row = st.setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str(),
                    row.getString(2).c_str());
            }
            catch (const tntdb::NotFound &e) {
                log_warning("Type '%d' not dound", common_msg_mt_id(*msg));
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_NOTFOUND,e.what(),NULL);
            }
            catch (const std::exception &e) {
                log_error("While getting type '%d' encountered '%s'",
                          common_msg_mt_id(*msg), e.what());
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_INTERNAL,e.what(),NULL);
            }
            break;

        case COMMON_MSG_GET_MEASURE_TYPE_S: {
            try {
                tntdb::Statement st = conn.prepareCached(
                    "insert into t_bios_measurement_types (name, unit) "
                    "select :name, :unit from dual WHERE NOT EXISTS "
                    "(select id from t_bios_measurement_types where name=:name)"
                );
                st.setString("name", common_msg_mt_name(*msg)).
                   setString("unit", common_msg_mt_unit(*msg)).
                   execute();
                st = conn.prepareCached(
                    "select id, name, unit from t_bios_measurement_types "
                    "where name = :name "
                    "and id is not null and name is not null");

                row = st.setString("name", common_msg_mt_name(*msg)).
                      selectRow();
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str(),
                    row.getString(2).c_str());
            } catch (const std::exception &e) {
                log_error("While getting type '%s' encountered '%s'",
                          common_msg_mt_name(*msg), e.what());
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_INTERNAL,e.what(),NULL);
            }}
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, id_type, scale, name "
                    "from t_bios_measurement_subtypes where id = :mts_id "
                    "and id_type = :mt_id "
                    "and id is not null and id_type is not null and "
                    "name is not null and scale is not null");

                row = st.setInt("mts_id", common_msg_mts_id(*msg)).
                         setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            }
            catch (const tntdb::NotFound &e) {
                log_warning("Subtype %d/%d not dound", common_msg_mt_id(*msg),
                            common_msg_mts_id(*msg));
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_NOTFOUND,e.what(),NULL);
            }
            catch (const std::exception &e) {
                log_error("While getting subtype %d/%d encountered '%s'",
                          common_msg_mt_id(*msg), common_msg_mts_id(*msg), e.what());
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_INTERNAL,e.what(),NULL);
            }
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S: {
            try {
                tntdb::Statement st = conn.prepareCached(
                    "insert into t_bios_measurement_subtypes (id, name, id_type, scale) "
                    "select "
                    "(select COALESCE(max(id),0)+1 from "
                    "t_bios_measurement_subtypes where id_type=:mt_id), "
                    ":name, :mt_id, :scale from dual WHERE NOT EXISTS "
                    "(select id from t_bios_measurement_subtypes where "
                    " name=:name and id_type=:mt_id)"
                );
                st.setString("name", common_msg_mts_name(*msg)).
                   setInt("mt_id", common_msg_mt_id(*msg)).
                   setInt("scale", (int8_t)common_msg_mts_scale(*msg)).
                   execute();
                st = conn.prepareCached(
                    "select id, id_type, scale, name "
                    "from t_bios_measurement_subtypes where name = :name "
                    "and id_type = :mt_id "
                    "and id is not null and id_type is not null and "
                    "name is not null and scale is not null");

                row = st.setString("name", common_msg_mts_name(*msg)).
                         setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            } catch (const std::exception &e) {
                log_error("While getting sutype %d/'%s' encountered '%s'",
                          common_msg_mt_id(*msg), common_msg_mt_name(*msg), e.what());
                ret = common_msg_encode_fail(DB_ERR,DB_ERROR_INTERNAL,e.what(),NULL);
            }}
            break;

        default:
            log_warning("Got wrong measurements meta message!");
            ret = common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                         "Wrong meassurements meta message",
                                         NULL);
            break;
    }
    common_msg_destroy(msg);
    return ret;
}

