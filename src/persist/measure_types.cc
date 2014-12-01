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
                    "where id = :mt_id "
                    "and id is not null and name is not null");

                row = st.setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }
            break;

        case COMMON_MSG_GET_MEASURE_TYPE_S:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "insert into t_bios_measurement_types (name) "
                    "select :name from dual WHERE NOT EXISTS "
                    "(select id from t_bios_measurement_types where name=:name)"
                );
                st.setString("name", common_msg_mt_name(*msg)).execute();
            } catch (const std::exception &e) {
            }
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, name from t_bios_measurement_types "
                    "where name = :name "
                    "and id is not null and name is not null");

                row = st.setString("name", common_msg_mt_name(*msg)).
                      selectRow();
                ret = common_msg_encode_return_measure_type(
                    row.getInt(0), row.getString(1).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, type_id, scale, name "
                    "from t_bios_measurement_subtypes where id = :mts_id "
                    "and type_id = :mt_id "
                    "and id is not null and type_id is not null and "
                    "name is not null and scale is not null");

                row = st.setInt("mts_id", common_msg_mts_id(*msg)).
                         setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            try {
                tntdb::Statement st = conn.prepareCached(
                    "insert into t_bios_measurement_subtypes (id, name, type_id, scale) "
                    "select "
                    "(select max(id)+1 from t_bios_measurement_subtypes where type_id=:mt_id), "
                    ":name, :mt_id, :scale from dual WHERE NOT EXISTS "
                    "(select id from t_bios_measurement_subtypes where "
                    " name=:name and type_id=:mt_id)"
                );
                st.setString("name", common_msg_mts_name(*msg)).
                   setInt("mt_id", common_msg_mt_id(*msg)).
                   setInt("scale", (int8_t)common_msg_mts_scale(*msg)).
                   execute();
            } catch (const std::exception &e) {
            }
            try {
                tntdb::Statement st = conn.prepareCached(
                    "select id, type_id, scale, name "
                    "from t_bios_measurement_subtypes where name = :name "
                    "and type_id = :mt_id "
                    "and id is not null and type_id is not null and "
                    "name is not null and scale is not null");

                row = st.setString("name", common_msg_mts_name(*msg)).
                         setInt("mt_id", common_msg_mt_id(*msg)).selectRow();
                ret = common_msg_encode_return_measure_subtype(
                    row.getInt(0), row.getShort(1), row.getShort(2),
                    row.getString(3).c_str());
            } catch (const std::exception &e) {
                ret = common_msg_encode_fail(0,0,e.what(),NULL);
            }
            break;
        case COMMON_MSG_GET_MEASURE_SUBTYPE_SS:
            zmsg_t *req;
            zmsg_t *rep;
            common_msg_t *dta;
            req = common_msg_encode_get_measure_type_s(
                      common_msg_mt_name(*msg));
            rep = process_measures_meta(&req);
            dta = common_msg_decode(&rep);
            if(common_msg_id(dta) != COMMON_MSG_RETURN_MEASURE_TYPE) {
                ret = common_msg_encode(&dta);
            } else {
                uint16_t id = common_msg_mt_id(dta);
                req = common_msg_encode_get_measure_subtype_s(id,
                          common_msg_mts_name(*msg),
                          common_msg_mts_scale(*msg) );
                common_msg_destroy(&dta);
                ret = process_measures_meta(&req);
                zmsg_destroy(&req);
            }
            break;

        default:
            ret = common_msg_encode_fail(0,0,"invalid message",NULL);
            break;
    }
    common_msg_destroy(msg);
    return ret;
}

