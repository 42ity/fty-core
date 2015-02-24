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
 \author Karol Hrdina    <KarolHrdina@eaton.com>
*/

#ifndef SRC_INCLUDE_BIOS_AGENT__
#define SRC_INCLUDE_BIOS_AGENT__

#include <czmq.h>
#include <malamute.h>

#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _bios_agent_t {
    mlm_client_t *client;   // isn't this obvious?
    void* seq;              //  
};

typedef struct _bios_agent_t bios_agent_t;

bios_agent_t*
    bios_agent_new (const char* endpoint, uint32_t timeout, const char* address);

void
    bios_agent_destroy (bios_agent_t **self_p);

// Closer look reveals it does not make sense
//bool
//    is_bios_agent (bios_agent_t *bios_agent);

// Following functions do not need to be wrapped: 
//  mlm_client_actor(), mlm_client_msgpipe(), mlm_client_set_producer(), mlm_client_set_consumer(), mlm_client_set_worker()
// because they operate solely on mlm_client_t and that is easily obtainable as a member of bios_agent_t

// 0 - success, -2 bad args, -1 failure
int
    bios_agent_send (bios_agent_t *bios_agent, const char *subject, ymsg_t **ymsg_p);

int
    bios_agent_sendto (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p);

int
    bios_agent_replyto (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send);
// TODO:
// int bios_agent_replyto (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, uint16_t rep, zchunk_t **request);

int
    bios_agent_sendfor (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p);

int
    bios_agent_replyfor (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send);
// TODO:
// int bios_agent_replyfor (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, uint16_t rep, zchunk_t **request);

ymsg_t *
    bios_agent_recv (bios_agent_t *bios_agent);

////////////////////////////////////////////////////////////////////////////////
//   Getters

//  Return last received command. Can be one of these values:
//      "STREAM DELIVER"
//      "MAILBOX DELIVER"
//      "SERVICE DELIVER"
const char *
    bios_agent_command (bios_agent_t *bios_agent);

//  Return last received status
int
    bios_agent_status (bios_agent_t *bios_agent);

//  Return last received reason
const char *
    bios_agent_reason (bios_agent_t *bios_agent);

//  Return last received address
const char *
    bios_agent_address (bios_agent_t *bios_agent);

//  Return last received sender
const char *
    bios_agent_sender (bios_agent_t *bios_agent);

//  Return last received subject
const char *
    bios_agent_subject (bios_agent_t *bios_agent);

//  Return last received content
ymsg_t *
    bios_agent_content (bios_agent_t *bios_agent);

//  Return last received tracker
const char *
    bios_agent_tracker (bios_agent_t *bios_agent);

#ifdef __cplusplus
}
#endif

#endif // SRC_INCLUDE_BIOS_AGENT__

