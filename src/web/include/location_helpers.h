#ifndef SRC_WEB_INCLUDE_LOCATION_HELPERS
#define SRC_WEB_INCLUDE_LOCATION_HELPERS

#include <string>
#include <czmq.h>
#include "asset_msg.h"

// 
int process_return_location_from (asset_msg_t **message_p, std::string& json);
bool build_return_location_from (asset_msg_t *message, std::string& json);

int element_id (const std::string& from, int& element_id);
int asset (const std::string& from);

#endif // SRC_WEB_INCLUDE_LOCATION_HELPERS

