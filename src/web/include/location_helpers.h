#ifndef SRC_WEB_INCLUDE_LOCATION_HELPERS
#define SRC_WEB_INCLUDE_LOCATION_HELPERS

#include <string>
#include <czmq.h>
#include "asset_msg.h"

int element_id (const std::string& from, int& element_id);
int asset (const std::string& from);
int asset_location_r(asset_msg_t** asset_msg, std::string& json);

#endif // SRC_WEB_INCLUDE_LOCATION_HELPERS

