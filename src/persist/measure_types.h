#ifndef SRC_PERSIST_MEASURES_H
#define SRC_PERSIST_MEASURES_H

#include <zmq.h>
#include <czmq.h>

#include "common_msg.h"

/*
 * \brief Processes message regarding measures metadata and creates a reply
 *
 * Main type of requests would be querying type name for id, or getting id for
 * message type names.
 *
 * @param msg Message to process
 * @return Reply to the message, error or NULL
 *
 */
zmsg_t* process_measures_meta(zmsg_t** msg);

/*
 * \brief Processes message regarding measures metadata and creates a reply
 *
 * Main type of requests would be querying type name for id, or getting id for
 * message type names.
 *
 * @param msg Message to process
 * @return Reply to the message, error or NULL
 *
 */
zmsg_t* process_measures_meta(common_msg_t** msg);

#endif // SRC_PERSIST_MEASURES
