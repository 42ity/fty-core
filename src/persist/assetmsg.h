#ifndef SRC_PERSIST_ASSETMSG_H_
#define SRC_PERSIST_ASSETMSG_H_

#include "asset_msg.h"

#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5
#define BIOS_ERROR_DB 1

#define MAX_NAME_LENGTH 25 // For now it the maximum length of all fields nam
#define MAX_DESCRIPTION_LENGTH 255

asset_msg_t *asset_msg_process(const char *url, asset_msg_t *msg);
void* void_dup(const void* a); 
#endif // SRC_PERSIST_ASSETMSG_H_

