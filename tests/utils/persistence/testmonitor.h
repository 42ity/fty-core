common_msg_t* _generate_db_fail(uint32_t errorid,  const char* errmsg,  zhash_t* erraux);
common_msg_t* _generate_ok(uint32_t rowid);
common_msg_t* _generate_client(const char* name);
common_msg_t* _generate_return_client(uint32_t clientid, common_msg_t** client);
common_msg_t* select_client(const char* url, const char* name);
common_msg_t* select_client(const char* url, uint32_t id);
common_msg_t* insert_client(const char* url,const char* name);
common_msg_t* delete_client(const char* url, uint32_t id_client);
common_msg_t* update_client(const char* url, uint32_t id, common_msg_t** client);
common_msg_t* _generate_client_info(uint32_t client_id, uint32_t device_id, uint32_t mytime, byte* data, uint32_t datasize);
common_msg_t* _insert_client_info (const char* url, uint32_t device_id, uint32_t client_id, byte* info, uint32_t infolen);
common_msg_t* _delete_client_info (const char* url, uint32_t id);
common_msg_t* _update_client_info (const char* url, common_msg_t* newclientinfo);
common_msg_t* select_client_info_last(const char* url, uint32_t client_id, uint32_t device_id);
common_msg_t* _select_client_info(const char* url, uint32_t id);
common_msg_t* _generate_return_client_info(uint32_t client_info_id, common_msg_t** client_info);

