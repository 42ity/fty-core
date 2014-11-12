#ifndef SRC_PERSIST_MONITOR_H_
#define SRC_PERSIST_MONITOR_H_

common_msg_t* _generate_db_fail(uint32_t errorid, const char* errmsg, zhash_t** erraux);

common_msg_t* _generate_ok(uint32_t rowid);
////////////////////////////////////////////////////////////////////
///////            CLIENT                    ///////////////////////
////////////////////////////////////////////////////////////////////

common_msg_t* _generate_client(const char* name);

common_msg_t* _generate_return_client(uint32_t clientid, common_msg_t** client);

common_msg_t* select_client(const char* url, const char* name);

common_msg_t* select_client(const char* url, uint32_t id);

common_msg_t* insert_client(const char* url,const char* name);

common_msg_t* delete_client(const char* url, uint32_t id_client);
// should destroy client
common_msg_t* update_client(const char* url, uint32_t id, common_msg_t** client);

////////////////////////////////////////////////////////////////////////
/////////////////           CLIENT INFO              ///////////////////
////////////////////////////////////////////////////////////////////////

common_msg_t* _generate_client_info
    (uint32_t client_id, uint32_t device_id, uint32_t mytime, byte* data, uint32_t datasize);

common_msg_t* _generate_return_client_info(uint32_t client_info_id, common_msg_t** client_info);

/**
 * \brief Inserts into the table t_bios_client_info new row.
 *
 * blob would be destroyed.
 *
 * \param device_id - id of the device information is about
 * \param client_id - id of the module that gathered this information
 * \param blob      - an information as a flow of bytes
 *
 * \return COMMON_MSG_DB_FAIL if inserting failed
 *         COMMON_MSG_DB_OK   if inserting was successful
 */
common_msg_t* insert_client_info
    (const char* url, uint32_t device_id, uint32_t client_id, zchunk_t** blob);

common_msg_t* insert_client_info
    (const char* url, uint32_t device_id, uint32_t client_id, byte* blobdata, uint32_t blobsize);

/**
 * \brief Delets from the table t_bios_client_info row by id.
 *
 * \param id - id of the row to be deleted
 *
 * \return COMMON_MSG_DB_FAIL if delete failed
 *         COMMON_MSG_DB_OK   if delete was successful
 */
common_msg_t* delete_client_info (const char* url, uint32_t id);

/**
 * \brief Updates in the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information
 *
 * \return COMMON_MSG_DB_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* update_client_info (const char* url, common_msg_t** newclientinfo);

/**
 * \brief Selects the last row the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information
 *
 * \return COMMON_MSG_DB_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* select_client_info_last(const char* url, uint32_t client_id, uint32_t device_id);

common_msg_t* select_client_info(const char* url, uint32_t id_client_info);

///////////////////////////////////////////////////////////////////
///////            DEVICE TYPE              ///////////////////////
///////////////////////////////////////////////////////////////////

common_msg_t* _generate_device_type(const char* name);

common_msg_t* _generate_return_device_type(uint32_t devicetype_id, common_msg_t** device_type);

common_msg_t* select_device_type(const char* url, const char* name);

common_msg_t* select_device_type(const char* url, uint32_t devicetype_id);

common_msg_t* insert_device_type(const char* url, const char* name);

common_msg_t* delete_device_type(const char* url, uint32_t devicetype_id);

// should destroy device_type
common_msg_t* update_device_type(const char* url, uint32_t id, common_msg_t** devicetype);
////////////////////////////////////////////////////////////////////////
/////////////////           DEVICE                   ///////////////////
////////////////////////////////////////////////////////////////////////

common_msg_t* _generate_device(const char* name, uint32_t devicetype_id);

common_msg_t* _generate_return_device(uint32_t device_id, common_msg_t** device);

common_msg_t* _generate_device(const char* name, uint32_t devicetype_id);
//it shoud destroy the device
common_msg_t* _generate_return_device(uint32_t device_id, common_msg_t** device);
/**
 * \brief Inserts into the table t_bios_discovered_device new row.
 *
 * \param url           - connection url to database
 * \param devicetype_id - id of the device type
 * \param name          - name of the device
 *
 * \return COMMON_MSG_DB_FAIL if inserting failed
 *         COMMON_MSG_DB_OK   if inserting was successful
 */
common_msg_t* insert_device(const char* url, uint32_t devicetype_id, const char* name);

/**
 * \brief Deletes from the table t_bios_discovered_device row by id.
 *
 * \param id - id of the row to be deleted
 *
 * \return COMMON_MSG_DB_FAIL if delete failed
 *         COMMON_MSG_DB_OK   if delete was successful
 */
common_msg_t* delete_device (const char* url, uint32_t device_id);
/**
 * \brief Updates in the table t_bios_discovered_device row by id.
 *
 * \param newdevice - message with new information
 *
 * \return COMMON_MSG_DB_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* update_device (const char* url, common_msg_t** newdevice);

common_msg_t* select_device (const char* url, uint32_t devicetype_id, const char* name);

common_msg_t* select_device (const char* url, const char* devicetype_name, const char* name);

common_msg_t* insert_device(const char* url, const char* devicetype_name, const char* name);

common_msg_t* select_key (const char* url, const char* keytagname);
common_msg_t* insert_measurement(const char* url,uint32_t client_id,uint32_t device_id,uint32_t keytag_id,uint32_t subkeytag, uint64_t value);
#endif // SRC_PERSIST_MONITOR_H_

