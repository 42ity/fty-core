/*
Copyright (C) 2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   monitor.h
    \brief  Functions for manipulating with elements in database monitor part.
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/

#ifndef SRC_PERSIST_MONITOR_H_
#define SRC_PERSIST_MONITOR_H_

#include <string>

#include "common_msg.h"
#include <tntdb/connect.h>
#include "dbtypes.h"
#include "dbhelpers.h"

#define GEN_MEASUREMENTS_MAX 10

// !!!!!!!!!                ATTENTION           !!!!!!!!!!!!!!!!!!
// Date is always UTC time as UNIX_TIMESTAMP.

void generate_measurements (const char         *url, 
                            m_dvc_id_t         device_id, 
                            uint32_t           max_seconds, 
                            m_msrmnt_value_t   last_value);

// ===============================================================
// Helper common functions
// ===============================================================

/**
 * \brief Generates a COMMON_MSG_FAIL message.
 * 
 * Generates a COMMON_MSG_FAIL with error type BIOS_ERROR_DB , with 
 * a specified code, errormessage and optional parameters.
 * 
 * If the parameter errmsg was NULL, then result error message would be "".
 *
 * \param errorid - an id of the error.
 * \param errmsg  - a detailed message about the error.
 * \param erraux  - optional information.
 *
 * \return a COMMON_MSG_FAIL message.
 */
common_msg_t* generate_db_fail(uint32_t errorid, const char* errmsg, 
                               zhash_t** erraux);


/**
 * \brief Generates an COMMON_MSG_DB_OK message and specifies an id 
 * of the row that was processed.
 *
 * \param rowid - an id of the processed row.
 *
 * \return a COMMON_MSG_DB_OK message.
 */
common_msg_t* generate_ok(uint64_t rowid, zhash_t **aux);


// ===============================================================
// CLIENT
// ===============================================================


/**
 * \brief A helper function analogue for common_msg_client_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * \param client_name - name of the client.
 *
 * \return - a COMMON_MSG_CLIENT message.
 */
common_msg_t* generate_client(const char* client_name);


/**
 * \brief A helper function analogue for common_msg_return_client_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It does destriy the "client" message.
 *
 * \param client_id - an id of the client.
 * \param client    - a COMMON_MSG_CLIENT message.
 *
 * \return - a COMMON_MSG_RETURN_CLIENT message.
 */
common_msg_t* generate_return_client(m_clnt_id_t client_id, 
                                     common_msg_t** client);


/**
 * \brief Selects a client from t_bios_client by its name.
 *
 * In case of success it generates COMMON_MSG_CLIENT message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url         - connection url to database
 * \param client_name - name of the client.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* select_client(const char* url, const char* client_name);


/**
 * \brief Selects a client from t_bios_client by its id.
 *
 * In case of success it generates COMMON_MSG_CLIENT message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database
 * \param client_id - id of the client.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* select_client(const char* url, m_clnt_id_t client_id);


/**
 * \brief Inserts into the table t_bios_client a new client.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url         - connection url to database
 * \param client_name - name of the new client.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_client(const char* url, const char* client_name);


/**
 * \brief Deletes from the table t_bios_client a row by its id.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database
 * \param client_id - id of the row to be deleted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* delete_client(const char* url, m_clnt_id_t client_id);


/**
 * \brief Updates in the table t_bios_client a row by its id.
 *
 * It does destroy the "client" message.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database
 * \param client_id - id of the row to be updated.
 * \param client    - an new client.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* update_client(const char* url, m_clnt_id_t client_id, 
                            common_msg_t** client);


// ===============================================================
// CLIENT INFO
// ===============================================================


/**
 * \brief A helper function analogue for common_msg_client_info_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * \param client_id - an id of the client.
 * \param device_id - an id of the device in monitor part.
 * \param mytime    - time as UNIX_TIMESTAMP.
 * \param data      - information.
 * \param datasize  - size of information.
 *
 * \return - a COMMON_MSG_CLIENT_INFO message.
 */
common_msg_t* generate_client_info
    (m_clnt_id_t client_id, m_dvc_id_t device_id, uint32_t mytime, 
    byte* data, uint32_t datasize);


/**
 * \brief A helper function analogue for common_msg_device_type_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It does destroy the "client_info" message.
 *
 * \param client_info_id - an id of the device type in monitor part.
 * \param client_info    - a COMMON_MSG_CLIENT_INFO message.
 *
 * \return - a COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* generate_return_client_info(m_clnt_info_id_t client_info_id, 
                                          common_msg_t** client_info);


/**
 * \brief Inserts information about device gathered by client into 
 * t_bios_client_info.
 *
 * In case of success it generates COMMON_MSG_RETURN_CINFO message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database.
 * \param device_id - id of the device in monitor part we searching for.
 * \param client_id - id of the client.
 * \param blob      - information to be inserted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* insert_client_info (const char* url, m_dvc_id_t device_id, 
                                  m_clnt_id_t client_id, zchunk_t** blob);


/** 
 * \brief Inserts information about device gathered by client.
 *
 * In case of success it generates COMMON_MSG_RETURN_CINFO message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database.
 * \param device_id - id of the device in monitor part we searching for.
 * \param client_id - id of the client.
 * \param blobdata  - information to be inserted (array of bytes).
 * \param blobsize  - soze of information to be nserted
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* insert_client_info  (const char* url, m_dvc_id_t device_id, 
                m_clnt_id_t client_id, byte* blobdata, uint32_t blobsize);


/**
 * \brief Deletes from the table t_bios_client_info a row by id.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param client_info_id - id of the row to be deleted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* delete_client_info (const char* url, 
                                  m_clnt_info_id_t client_info_id);


/**
 * \brief Updates in the table t_bios_client_info a row by its id.
 *
 * Blob would be destroyed.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database
 * \param client_info_id - id of the row to be updated.
 * \param blob           - an information as a flow of bytes.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* update_client_info
    (const char* url, m_clnt_info_id_t client_info_id, zchunk_t** blob);


/**
 * \brief Updates the ui properties.
 *
 * Blob would be destroyed.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database
 * \param blob           - an information as a flow of bytes.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* update_ui_properties
    (const char* url, zchunk_t** blob);

/** 
 * \brief Selects last information about device gathered by client.
 *
 * In case of success it generates COMMON_MSG_RETURN_CINFO message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database.
 * \param client_id - id of the client.
 * \param device_id - id of the device in monitor part we searching for.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* select_client_info_last(const char  * url, 
                                      m_clnt_id_t client_id, 
                                      m_dvc_id_t  device_id);


/** 
 * \brief Selects information by its id.
 *
 * In case of success it generates COMMON_MSG_RETURN_CINFO message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param client_info_id - id of the row with information.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* select_client_info(const char* url, 
                                 m_clnt_info_id_t client_info_id);


// ===============================================================
// UI PROPERTIES
// ===============================================================


/** 
 * \brief Selects a UI properties from database.
 *
 * Due to unusual nature of UI properties entry - we suppose there will be
 * the only one in t_bios_client_info.
 * 
 * In case of success it generates COMMON_MSG_RETURN_CINFO message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url - connection url to database.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_CINFO message.
 */
common_msg_t* select_ui_properties(const char* url);


// ===============================================================
// DEVICE TYPE
// ===============================================================


/**
 * \brief A helper function analogue for common_msg_device_type_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * \param device_type_id - an id of the device type in monitor part.
 * \param device_type    - a COMMON_MSG_DEVICE_TYPE message.
 *
 * \return - a COMMON_MSG_DEVICE_TYPE message.
 */
common_msg_t* generate_device_type(const char* device_type_name);


/**
 * \brief A helper function analogue for common_msg_return_devtype_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It does destroy the "device_type" message.
 *
 * \param device_type_id - an id of the device type in monitor part.
 * \param device_type    - a COMMON_MSG_DEVICE_TYPE message.
 *
 * \return - a COMMON_MSG_RETURN_DEVTYPE message.
 */
common_msg_t* generate_return_device_type(m_dvc_tp_id_t device_type_id, 
                                          common_msg_t** device_type);


/**
 * \brief Selects a device type by device type name in monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVTYPE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - a device type name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVTYPE message.
 */
common_msg_t* select_device_type(const char* url, 
                                 const char* device_type_name);


/**
 * \brief Selects a device type by device type id in monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVTYPE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - a device type id.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVTYPE message.
 */
common_msg_t* select_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id);


/**
 * \brief Inserts into the table t_bios_device_type a new row.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - name of the device type.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_device_type(const char* url, 
                                 const char* device_type_name);


/**
 * \brief Deletes from the table t_bios_device_type a row by id.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - id of the row to be deleted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* delete_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id);


/**
 * \brief Updates a row in a table t_bios_device_type by id with
 * specified object.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * It destroyes the "device_type" message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - id of the device type.
 * \param device_type    - a message COMMON_MSG_DEVICE_TYPE with new values.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* update_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id, 
                                 common_msg_t** device_type);

db_reply_t
    select_monitor_device_type_id
        (tntdb::Connection &conn, 
         const char *device_type_name);

// ===============================================================
// DEVICE
// ===============================================================


db_reply_t 
    select_device (tntdb::Connection &conn, 
                   const char* device_name);


/**
 * \brief A helper function analogue for common_msg_device_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * \param device_name    - name of the device.
 * \param device_type_id - an id of the device type.
 *
 * \return - a COMMON_MSG_DEVICE message.
 */
common_msg_t* generate_device (const char* device_name, 
                               m_dvc_tp_id_t device_type_id);


/**
 * \brief A helper function analogue for common_msg_return_device_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It does destroy the "device" message.
 *
 * \param device_id - an id of the device in monitor part.
 * \param device    - a COMMON_MSG_DEVICE message.
 *
 * \return - a COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* generate_return_device(m_dvc_id_t device_id, 
                                     common_msg_t** device);


/**
 * \brief Inserts into the table t_bios_discovered_device a new row.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - id of the device type.
 * \param device_name    - name of the device.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_disc_device(const char* url, m_dvc_tp_id_t device_type_id, 
                            const char* device_name);


/**
 * \brief Deletes from the table t_bios_discovered_device a row by id.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url       - connection url to database.
 * \param device_id - id of the row to be deleted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* delete_disc_device (const char* url, m_dvc_id_t device_id);


/**
 * \brief Updates in the table t_bios_discovered_device a row by id.
 *
 * \param newdevice - message with new information.
 *
 * \return COMMON_MSG_FAIL if update failed.
 *         COMMON_MSG_DB_OK   if update was successful.
 */
common_msg_t* update_device (const char* url, common_msg_t** new_device);


/**
 * \brief Selects a device by device name in monitor part and device type id 
 * in monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVICE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 *  There are discovered devices with such name.
 *  TODO for now take first in the list.
 *
 * \param url            - connection url to database.
 * \param device_type_id - a device type name.
 * \param device_name    - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* select_device (const char* url, m_dvc_tp_id_t device_type_id, 
                             const char* device_name);


/**
 * \brief Selects a device by device name and device type name in 
 * monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVICE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - a device type name.
 * \param device_name      - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* select_device (const char* url, const char* device_type_name, 
                             const char* device_name);


/**
 * \brief Inserts a new device by device name and device type name into 
 * monitor part.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - a device type name.
 * \param device_name      - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_disc_device(const char* url, const char* device_type_name, 
                            const char* device_name);


db_reply_t 
    insert_into_monitor_device
        (tntdb::Connection &conn,
         m_dvc_tp_id_t device_type_id,
         const char* device_name);

db_reply_t 
    insert_into_monitor_device
        (tntdb::Connection &conn,
         const char* device_type_name,
         const char* device_name);


// ===============================================================
// MEASUREMENT
// ===============================================================


/**
 * \brief Inserts into the table t_bios_client_measurements a new row.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url        - connection url to database.
 * \param client_id  - id of the client that gathered measure.
 * \param device_id  - id of the device that was measured.
 * \param type_id    - id of the measure type.
 * \param subtype_id - id of the measure subtype.
 * \param value      - value of measurement.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_DB_OK message.
 */
common_msg_t* insert_measurement(const char         *url, 
                                 m_clnt_id_t        client_id, 
                                 m_dvc_id_t         device_id, 
                                 m_msrmnt_tp_id_t   type_id, 
                                 m_msrmnt_sbtp_id_t subtype_id, 
                                 m_msrmnt_value_t   value);


/**
 * \brief A helper function analogue for 
 *                              common_msg_return_last_measurements_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It does destroy the "measurements" message.
 *
 * \param device_id    - an id_asset_element for the device.
 * \param measurements - last measurements for the specified device.
 *
 * \return - a COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
common_msg_t* generate_return_last_measurements (a_elmnt_id_t device_id, 
                                                zlist_t** measurements);


/**
 * \brief Takes all last measurements of the specified device.
 *
 * Device is specified by its t_bios_discovered_device.id_discovered_device.
 *
 * Returns a list of strings. One string for one measurement.
 * Every string has the followng format: "keytag_id:subkeytag_id:value:scale".
 *
 * \param[in]  url         - connection url to database.
 * \param[in]  device_id   - id of the monitor device that was measured.
 * \param[out] device_name - name of measured device.
 *
 * \return NULL            in case of errors.
 *         empty zlist_t   in case of nothing was found.
 *         filled zlist_t  in case of succcess.
 */
zlist_t* select_last_measurements(tntdb::Connection &conn, m_dvc_id_t device_id,
                                                std::string &device);


/**
 * \brief This function processes the COMMON_MSG_GET_LAST_MEASUREMENTS 
 * message.
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 * 
 * In case of success it generates COMMON_MSG_RETURN_LAST_MEASUREMENTS 
 * message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * It doesn't destroy the "getmsg" message.
 *
 * \param url    - the connection to database.
 * \param getmsg - the message of the type COMMON_MSG_GET_LAST_MEASUREMENTS 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
zmsg_t* _get_last_measurements(const char* url, common_msg_t* getmsg);


/**
 * \brief A wrapper for _get_last_measurements.
 *
 * It does destroy the "getmsg" message.
 *
 * \param getmsg - a filled COMMON_MSG_GET_LAST_MEASUREMENTS message.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
zmsg_t* get_last_measurements(zmsg_t** getmsg);


#endif // SRC_PERSIST_MONITOR_H_
