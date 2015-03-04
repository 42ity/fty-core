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

// For certain items, contradict the default of GCC "-fvisibility=hidden"
#ifndef BIOS_EXPORT
# if BUILDING_LIBBIOSAPI && HAVE_VISIBILITY
#  define BIOS_EXPORT __attribute__((__visibility__("default")))
# else
#  define BIOS_EXPORT
# endif
#endif

// marker to tell humans and GCC that the unused parameter is there for some
// reason (i.e. API compatibility) and compiler should not warn if not used
#if !defined(UNUSED_PARAM) && defined (__GNUC__)
# define UNUSED_PARAM __attribute__ ((__unused__))
#else
# define UNUSED_PARAM
#endif

typedef struct _bios_agent_t bios_agent_t;

/*!
 \brief Create a new bios_agent.

 Connect to malamute on specified endpoint, with specified timeout in msecs (zero means wait forever). Name of the agent can have form: user/password in which case client logins to the broker via PLAIN authentication.
 \param[in] endpoint    Server endpoint to which to connect
 \param[in] name        Name of the agent. If it is in form user/password, agent connects to the broker via PLAIN authentication.
 \note Please note that you have to call bios_agent_destroy() when you are done working with bios_agent to free up allocated resources.
 \return Newly allocated bios agent on successfull connection, NULL otherwise.
*/
BIOS_EXPORT bios_agent_t*
    bios_agent_new (const char* endpoint, const char* name);

/*!
 \brief Destroy bios_agent, free up resources and nullify the pointer.
 \param[in] self_p  bios agent to be destroyed and nullified
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
 \brief Send ROZP SEND message using pattern MAILBOX SEND to malamute

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p);

/*!
 \brief Send ROZP REPLY message using patter MAILBOX SEND to malamute.

 Message sequence number of supplied 'send' message (which must be of type ROZP SEND) is copied to 'rep' field of 'reply_p' message (which must be of type ROZP REPLY). Message sequence number of 'self' bios agent is assigned to field 'seq' of 'reply_p' message. Depending on presence/value of "repeat" key in field 'aux' of message 'send_p' field 'request' is copied to 'reply_p' message. Takes ownership of message 'reply_p' and destroys it when done sending it.
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] send     ROZP SEND message from
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **reply_p, ymsg_t *send);

/*!
 \brief Send ROZP SEND message using patter SERVICE SEND to malamute

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p);

/*!
 \brief Receive message from malamute.

 Takes ownership of the message received.
 \param[in] self    Bios agent
 \note Caller is responsible to destroy the received message when done
 \return Received ROZP message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_agent_recv (bios_agent_t *self);

/*! 
 \brief Prepare to publish to a specified stream. 
 
 After this, all messages are sent to this stream exclusively.
 
 \param[in] self Bios agent
 \param[in] name of the stream

 \return -2 on bad input, -1 if interrupted, >=0 on success
 */
BIOS_EXPORT int
    bios_agent_set_producer (bios_agent_t *self, const char *stream);

/*!
 \brief Prepare to consume messages from a specified stream with 
 topic that matches the pattern.
 
 The pattern is a regular expression
 using the CZMQ zrex syntax. The most useful elements are: ^ and $ to match the
 start and end, . to match any character, \s and \S to match whitespace and
 non-whitespace, \d and \D to match a digit and non-digit, \a and \A to match
 alphabetic and non-alphabetic, \w and \W to match alphanumeric and
 non-alphanumeric, + for one or more repetitions, * for zero or more repetitions,
 and ( ) to create groups.
 
 \param[in] self     Bios agent
 \param[in] stream   Name of the stream
 \param[in] pattern  regular expression (CZMQ syntax)
 
 \return 0 if subscription was successful, -2 on bad input, -1 on fail for any other reason.
*/
BIOS_EXPORT int
    bios_agent_set_consumer (bios_agent_t *self, const char *stream, const char *pattern);


// TODO ACE: write documentation
BIOS_EXPORT uint64_t
    ymsg_rowid (ymsg_t *self);
BIOS_EXPORT int
    ymsg_set_rowid (ymsg_t *self, uint64_t rowid);
BIOS_EXPORT int
    ymsg_errtype (ymsg_t *self);
BIOS_EXPORT int
    ymsg_set_errtype (ymsg_t *self, int error_type);
BIOS_EXPORT int
    ymsg_errsubtype (ymsg_t *self);
BIOS_EXPORT int
    ymsg_set_errsubtype (ymsg_t *self, int error_subtype);
BIOS_EXPORT const char*
    ymsg_errmsg (ymsg_t *self);
BIOS_EXPORT int
    ymsg_set_errmsg (ymsg_t *self, const char *error_msg);

// without ownership transfer
BIOS_EXPORT zhash_t*
    ymsg_addinfo (ymsg_t *self);   

// transfer ownership
BIOS_EXPORT zhash_t*
    ymsg_get_addinfo (ymsg_t *self);
BIOS_EXPORT int
    ymsg_set_addinfo (ymsg_t *self, zhash_t *addinfo);
BIOS_EXPORT ymsg_t*
    ymsg_generate_ok(uint64_t rowid, zhash_t *addinfo);
BIOS_EXPORT ymsg_t*
    ymsg_generate_fail (int errtype, int errsubtype, const char *errmsg, zhash_t *addinfo);

/*!
 \brief Get number of rows affected of ROZP REPLY message
 \return  number of rows affected. If key is not specified, then -1.
*/
BIOS_EXPORT int
    ymsg_affected_rows (ymsg_t *self);

/*!
 \brief Set number of rows affected in ROZP REPLY message
*/
BIOS_EXPORT int
    ymsg_set_affected_rows (ymsg_t *self, int n);

/*!
 \brief Get status value of ROZP REPLY message
 \return
     -1 on failure (self, aux == NULL, key STATUS missing, message type not REPLY)
    status value otherwise (0 - error, 1 - ok)
*/
BIOS_EXPORT int
    ymsg_status (ymsg_t *self);

/*!
 \brief Set status value of ROZP REPLY message
 \return -1 on failure (self, aux == NULL, message not ROZP REPLY), 0 on success
*/
BIOS_EXPORT int
    ymsg_set_status (ymsg_t *self, bool status);

/*!
 \brief Get repeat value
 \return
    -1 on failure (self, aux == NULL)
    repeat value otherwise (0 - no repeat, 1 - repeat)
*/
BIOS_EXPORT int
    ymsg_repeat (ymsg_t *self);

/*!
 \brief Set repeat value
 \return -1 on failure (self, aux == NULL), 0 on success
*/
BIOS_EXPORT int
    ymsg_set_repeat (ymsg_t *self, bool repeat);

/*!
 \brief Get content type
 /return NULL on failure, content type on success
*/
BIOS_EXPORT const char *
    ymsg_content_type (ymsg_t *self);
/*!
 \brief Set content type
 /return -1 on failure, 0 on success
*/
BIOS_EXPORT int
    ymsg_set_content_type (ymsg_t *self, const char *content_type);

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

const char * ymsg_get_string(ymsg_t* msg, const char *key);
int32_t ymsg_get_int32(ymsg_t* msg, const char *key);
int64_t ymsg_get_int64(ymsg_t* msg, const char *key);
void ymsg_set_string(ymsg_t* msg, const char *key, const char *value);
void ymsg_set_int32(ymsg_t* msg, const char *key, int32_t value);
void ymsg_set_int64(ymsg_t* msg, const char *key, int64_t value);

#ifdef __cplusplus
}
#endif

#endif // SRC_INCLUDE_BIOS_AGENT__

