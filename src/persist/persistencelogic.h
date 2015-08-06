#ifndef SRC_PERSIST_PERSISTENCELOGIC_H_
#define SRC_PERSIST_PERSISTENCELOGIC_H_

#include <czmq.h>
#include <zmq.h>

// To be deleted - we should be fine with just zmsg_t
#include "ymsg.h"
#include "measurement.h"

#include <string>

namespace persist {

/**
 * \brief process MAILBOX DELIVER
 *
 * \param[out] output message - reply
 * \param[out] output subject - reply
 * \param[in]  input message
 * \param[in]  input subject
 * */
void process_mailbox_deliver(ymsg_t** out, char** out_subj, ymsg_t* in, const char* in_subj);

/* \brief process message on main/measurement stream */
void process_measurement(const std::string &topic, ymsg_t **ymsg, TopicCache& c);

/* \brief process message on networks stream */
void
process_networks(
        ymsg_t** in_p);

void process_inventory (ymsg_t **msg);

//#################### OLD PROTOCOL TO BE REMOVED! ####################

/**
 * \brief Processes message of type asset_msg_t
 *
 * Broken down processing of generic database zmsg_t, this time asset message
 * case.
 */
zmsg_t *asset_msg_process(zmsg_t **msg);

} // namespace persist

#endif // SRC_PERSIST_PERSISTENCELOGIC_H_

