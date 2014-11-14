#include <exception>
#include <assert.h>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>

#include "log.h"
#include "dbpath.h"
#include "common_msg.h"
#include "assetmsg.h"

zmsg_t* _get_last_measurements(common_msg_t *msg);

zmsg_t* _generate_return_measurements (uint32_t device_id, zlist_t** measurements)
{
    zmsg_t* resultmsg = common_msg_encode_return_last_measurements(device_id,
                                                                *measurements);
    assert ( resultmsg );
    zlist_destroy (measurements);
    return resultmsg;
}

/**
 * \brief Takes all last measurements of the spcified device.
 *
 * \param url       - connection url to database.
 * \param device_id - id of the monitor device that was measured.
 *
 * \return NULL            in case of errors.
 *         empty zlist_t   in case of nothing was found
 *         filled zlist_t  in case of succcess.
 */
zlist_t* select_last_measurements(uint32_t device_id)
{
    assert ( device_id ); // is required
    
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " id_key, value, id_subkey, scale"
            " FROM"
            " v_bios_client_info_measurements_last"
            " WHERE id_discovered_device=:deviceid"
        );

        tntdb::Result result = st.setInt("deviceid", device_id).
                                  select();

        uint32_t rsize = result.size();

        zlist_t* measurements = zlist_new();
        assert ( measurements );
        // in older versions this function is called 
        // zhash_set_item_duplicator
        zlist_set_duplicator (measurements, void_dup);

        char buff[42];     // 10+10+10+10 2
        if ( rsize > 0 )
        {
            // There are some measurements for the specified device
            // Go through the selected measurements
            for ( auto &row: result )
            {
                // keytag_id, required
                uint32_t keytag_id = 0;
                row[0].get(keytag_id);
                assert ( keytag_id );      // database is corrupted
                
                // subkeytag_id, required
                uint32_t subkeytag_id = 0;
                row[0].get(subkeytag_id);
                assert ( subkeytag_id );   // database is corrupted

                // value, required
                uint32_t value = 0;
                bool isNotNull = row[1].get(value);
                assert ( isNotNull );      // database is corrupted

                // scale, required
                uint32_t scale = 0;
                row[0].get(scale);
                assert ( scale );          // database is corrupted
                
                sprintf(buff, "%d:%d:%d:%d", keytag_id, subkeytag_id, 
                              value, scale);
                zlist_push (measurements, buff);
            }
        }
        return measurements;
    }
    catch (const std::exception &e) {
        return NULL;
    }
}

zmsg_t* _get_last_measurements(zmsg_t** msg) {
    common_msg_t *req = common_msg_decode(msg);
    zmsg_t *rep = _get_last_measurements(req);
    common_msg_destroy(&req);
    return rep;
}

zmsg_t* _get_last_measurements(common_msg_t* msg)
{
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_LAST_MEASUREMENTS );
    uint32_t device_id = common_msg_element_id (msg);
    assert ( device_id );
    uint32_t device_id_monitor = convert_asset_to_monitor(url.c_str(), device_id);
    assert ( device_id_monitor > 0 );

    zlist_t* last_measurements = 
            select_last_measurements(device_id_monitor);
    if ( last_measurements == NULL )
        return common_msg_encode_fail(0,0,"internal error",NULL);
    else if ( zlist_size(last_measurements) == 0 )
        return common_msg_encode_fail(0,0,"not found",NULL);
    else
    {
        return _generate_return_measurements(device_id, &last_measurements);
    }
};

