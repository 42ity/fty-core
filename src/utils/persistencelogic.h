#ifndef UTILS_PERSISTENCELOGIC_H_
#define UTILS_PERSISTENCELOGIC_H_

#include "czmq.h"
#include "netdisc_msg.h"
#include "powerdev_msg.h"
#include "common_msg.h"

namespace utils {

namespace db {

/**
 * \brief Processes the network discovery messages.
 *
 * \return true if processing was successful and false if message was bad
 */
bool
process_message(const std::string& url, zmsg_t *msg);

bool
netdisc_msg_process(const std::string& url, const netdisc_msg_t& msg);

bool
powerdev_msg_process(const std::string& url, const powerdev_msg_t& msg);

} // namespace db

} // namespace utils

#endif // UTILS_PERSISTENCELOGIC_H_

