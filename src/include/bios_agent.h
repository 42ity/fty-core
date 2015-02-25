/*
Copyright (C) 2014 - 2015 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
 \file   bios_agent.h
 \brief  Thin mlm_client.h wrapper to incorporate ymsg_t transport message 
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#ifndef SRC_INCLUDE_BIOS_AGENT__
#define SRC_INCLUDE_BIOS_AGENT__

#include <czmq.h>
#include <malamute.h>

#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BIOS_EXPORT 

struct _bios_agent_t {
    mlm_client_t *client;   // malamute client instance
    void* seq;              // message sequence number 
};

typedef struct _bios_agent_t bios_agent_t;

/*!
 \brief Create a new bios_agent.

 Connect to malamute on specified endpoint, with specified timeout in msecs (zero means wait forever). Name of the agent can have form: user/password in which case client logins to the broker via PLAIN authentication.
 \param[in] endpoint    Server endpoint to which to connect
 \param[in] timeout     Number of msecs to wait for successfull connection to specified endpoint. Zero (0) means wait indefinitely.
 \param[in] name        Name of the agent. If it is in form user/password, agent connects to the broker via PLAIN authentication.
 \note Please note that you have to call bios_agent_destroy() when you are done working with bios_agent to free up allocated resources. 
 \return Newly allocated bios agent on successfull connection, NULL otherwise.
*/
BIOS_EXPORT bios_agent_t*
    bios_agent_new (const char* endpoint, uint32_t timeout, const char* name);

/*!
 \brief Destroy bios_agent, free up resources and nullify the pointer.
 \param[in,out] self_p  bios agent to be destroyed and nullified
*/
BIOS_EXPORT void
    bios_agent_destroy (bios_agent_t **self_p);

/*!
 \brief Send STREAM SEND message to malamute.

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] subject Message subject
 \param[in] msg_p   Message to be sent. Gets destroyed, nullified upon send. 
 \note Message is destroyed and nullified when sent.
 \note Setting bios agent as producer/consumer of STREAM pattern is done by directly manipulating the 'client' member of bios agent using mlm_client_set_producer(), mlm_client_set_consumer().
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_send (bios_agent_t *self, const char *subject, ymsg_t **msg_p);

/*!
 \brief Send confirmed ROZP SEND message using pattern MAILBOX SEND to malamute

 Takes ownership of message and destroys it when done sending it.  
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] tracker Tracker for confirmed MAILBOX SEND
 \param[in] timeout Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.  
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendto_track (bios_agent_t *self, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p);

/*!
 \brief Send ROZP SEND message using pattern MAILBOX SEND to malamute

 Takes ownership of message and destroys it when done sending it.  
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] timeout Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.  
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, uint32_t timeout, ymsg_t **send_p);

/*!
 \brief Send confirmed ROZP REPLY message using pattern MAILBOX SEND to malamute.

 Message sequence number of supplied 'send' message (which must be of type ROZP SEND) is copied to 'rep' field of 'reply_p' message (which must be of type ROZP REPLY). Message sequence number of 'self' bios agent is assigned to field 'seq' of 'reply_p' message. Depending on presence/value of "repeat" key in field 'aux' of message 'send_p' field 'request' is copied to 'reply_p' message. Takes ownership of message 'reply_p' and destroys it when done sending it.  
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] tracker  Tracker for confirmed MAILBOX SEND
 \param[in] timeout  Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] send     ROZP SEND message from  
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
int
    bios_agent_replyto_track (bios_agent_t *self, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send);

/*!
 \brief Send ROZP REPLY message using patter MAILBOX SEND to malamute.

 Message sequence number of supplied 'send' message (which must be of type ROZP SEND) is copied to 'rep' field of 'reply_p' message (which must be of type ROZP REPLY). Message sequence number of 'self' bios agent is assigned to field 'seq' of 'reply_p' message. Depending on presence/value of "repeat" key in field 'aux' of message 'send_p' field 'request' is copied to 'reply_p' message. Takes ownership of message 'reply_p' and destroys it when done sending it.
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] timeout  Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] send     ROZP SEND message from  
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
int
    bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send);

/*!
 * Either we find a name OR we throw this out until someone says: i don't want to input message, but just data and finds a better name :)
 *
 \brief Send confirmed ROZP REPLY message using patter MAILBOX SEND to malamute.

 Takes ownership of message 'reply_p' and destroys it when done sending it.  
 This is an overload of bios_agent_replyto_track (bios_agent_t *self, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send) where the values that would have been taken from 'send' parameter are explicitely exposed.
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] tracker  Tracker for confirmed MAILBOX SEND
 \param[in] timeout  Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] rep      Value to be assigned to field 'rep' of 'reply_p' message 
 \param[in] request  Value to be assigned to field 'request' of 'reply_p' message. Can be NULL.   
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason

int
    bios_agent_replyto_track (bios_agent_t *self, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, uint16_t rep, zchunk_t **request);
*/

/*!
 * Either we find a name OR we throw this out until someone says: i don't want to input message, but just data and finds a better name :)
 *
 \brief Send ROZP REPLY message using patter MAILBOX SEND to malamute.

 Takes ownership of message 'reply_p' and destroys it when done sending it.
 This is an overload of bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send) where the values that would have been taken from 'send' parameter are explicitely exposed.
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] timeout  Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] rep      Value to be assigned to field 'rep' of 'reply_p' message 
 \param[in] request  Value to be assigned to field 'request' of 'reply_p' message. Can be NULL.   
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason

int
    bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, uint32_t timeout, ymsg_t **reply_p, uint16_t rep, zchunk_t **request);
*/

/*!
 \brief Send confirmed ROZP SEND message using patter SERVICE SEND to malamute

 Takes ownership of message and destroys it when done sending it.  
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] tracker Tracker for confirmed SERVICE SEND
 \param[in] timeout Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.  
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
int
    bios_agent_sendfor_track (bios_agent_t *self, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p);

/*!
 \brief Send ROZP SEND message using patter SERVICE SEND to malamute

 Takes ownership of message and destroys it when done sending it.  
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] timeout Number of msecs to wait for successfull send. Zero (0) means wait indefinitely.
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.  
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
int
    bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, uint32_t timeout, ymsg_t **send_p);

/*!
 \brief Receive message from malamute.

 Takes ownership of the message received.
 \param[in] self    Bios agent
 \note Caller is responsible to destroy the received message when done
 \return Received ROZP message on success, NULL on failure
*/
ymsg_t *
    bios_agent_recv (bios_agent_t *self);

// TODO: doxygen
int
    ymsg_get_version (ymsg_t *self);
int
    ymsg_get_status (ymsg_t *self, bool *status);
int
    ymsg_set_status (ymsg_t *self, bool status);
int
    ymsg_get_repeat (ymsg_t *self, bool *repeat);
int
    ymsg_set_repeat (ymsg_t *self, bool repeat);    
int
    ymsg_get_content_type (ymsg_t *self, char *content_type);
int
    ymsg_set_content_type (ymsg_t *self, const char *content_type);

// Later we can go for further functionality....
// zlist_t *
//     ymsg_get_x_headers (ymsg_t *self);
//     ymsg_x_headers (ymsg_t *self);

////////////////////////////////////////////////////////////////////////////////
//   Getters

/*
 \brief Return last received command.
 \param[in] self bios agent
 \return NULL on failure, one of the following values on success:
        "STREAM DELIVER"
        "MAILBOX DELIVER"
        "SERVICE DELIVER"
*/
const char *
    bios_agent_command (bios_agent_t *self);

/*
 \brief Return last received status
 \param[in] self bios agent
 \return -1 on failure, last received status on success
*/
int
    bios_agent_status (bios_agent_t *self);

/*
 \brief Return last received reason
 \param[in] self bios agent
 \return last received reason on success, NULL on failure
*/
const char *
    bios_agent_reason (bios_agent_t *self);

/*
 \brief Return last received address
 \param[in] self bios agent
 \return last received address on success, NULL on failure
*/
const char *
    bios_agent_address (bios_agent_t *self);

/*
 \brief Return last received sender
 \param[in] self bios agent
 \return last received sender on success, NULL on failure
*/
const char *
    bios_agent_sender (bios_agent_t *self);

/*
 \brief Return last received subject
 \param[in] self bios agent
 \return last received sender on success, NULL on failure
*/
const char *
    bios_agent_subject (bios_agent_t *self);

/*
 \brief Return last received content
 \param[in] self bios agent
 \return last received content on success, NULL on failure
*/
ymsg_t *
    bios_agent_content (bios_agent_t *self);

/*
 \brief Return last received tracker
 \param[in] self bios agent
 \return last received tracker on success, NULL on failure
*/
const char *
    bios_agent_tracker (bios_agent_t *self);

#ifdef __cplusplus
}
#endif

#endif // SRC_INCLUDE_BIOS_AGENT__

