#ifndef SRC_PERSIST_PERSISTENCELOGIC_H_
#define SRC_PERSIST_PERSISTENCELOGIC_H_

#include "czmq.h"
#include "netdisc_msg.h"
#include "powerdev_msg.h"
#include "common_msg.h"
#include "nmap_msg.h"

namespace persist {

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

// TODO doxy; proxy and destroy the message
void
nmap_msg_process (const char *url, nmap_msg_t *msg);
bool
common_msg_process(const std::string& url, const common_msg_t& msg);

bool insert_new_measurement(const char* url, common_msg_t* msg);

common_msg_t* get_last_measurements(const char* url, common_msg_t* msg);

bool insert_new_client_info(const char* url, common_msg_t* msg);


} // namespace persist

#endif // SRC_PERSIST_PERSISTENCELOGIC_H_

