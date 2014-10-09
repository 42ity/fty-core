#ifndef UTILS_PERSISTENCELOGIC_H_
#define UTILS_PERSISTENCELOGIC_H_

#include "netdisc_msg.h"

namespace utils {

namespace db {

bool
process_message(const std::string& url, const netdisc_msg_t& msg);

} // namespace db

} // namespace utils

#endif // UTILS_PERSISTENCELOGIC_H_

