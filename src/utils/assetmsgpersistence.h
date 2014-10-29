#ifndef UTILS_ASSETMSGPERSISTENCE_H_
#define UTILS_CASSETMSGPERSISTENCE_H_

#include "asset_msg.h"

#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5
#define ERROR_DB 1

#define MAX_NAME_LENGTH 25 // For now it the maximum length of all fields nam
# define MAX_DESCRIPTION_LENGTH 255
asset_msg_t *asset_msg_process(const char *url, asset_msg_t *msg);

#endif
