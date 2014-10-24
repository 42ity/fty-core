#ifndef UTILS_ASSETMSGPERSISTENCE_H_
#define UTILS_CASSETMSGPERSISTENCE_H_

#include "asset_msg.h"

#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5

asset_msg_t *asset_msg_process(const char *url, asset_msg_t *msg);

#endif
