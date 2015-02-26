/*  =========================================================================
    ymsg - draft
    
    Codec header for ymsg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: rozp.xml, or
     * The code generation script that built this file: zproto_codec_c_v1
    ************************************************************************
                                                                        
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
    along with this program.  If not, see http://www.gnu.org/licenses.  
    =========================================================================
*/

#ifndef __YMSG_H_INCLUDED__
#define __YMSG_H_INCLUDED__

/*  These are the ymsg messages:

    SEND - Transport layer
        version             number 1    Protocol version
        seq                 number 2    Agent specific, starting value unspecified. Each message sent increments this number by one. Reply message must send this number back encoded in field 'rep'.
        aux                 hash        Extra (auxiliary) headers. Keys must contain only the following characters 'a-zA-Z_-' and values can be any sequence without '\0' (NULL) character. Users can pass non-standard user-defined headers, but they must be prefixed with 'X-'. This field can be used to carry simple key-value app data as well.
        request             chunk       Application specific payload. Not mandatory.

    REPLY - Transport layer reply

Fields that are common with message 'send' are described there.
        version             number 1    
        seq                 number 2    
        rep                 number 2    Value must be identical to field 'seq' of message 'send' to which this reply message is being created.
        aux                 hash        
        response            chunk       
        request             chunk       Application specific payload of message 'send' may be included (repeated) in reply.
*/

#define YMSG_VERSION                        1

#define YMSG_SEND                           1
#define YMSG_REPLY                          2

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef YMSG_T_DEFINED
typedef struct _ymsg_t ymsg_t;
#define YMSG_T_DEFINED
#endif

//  @interface
//  Create a new ymsg
ymsg_t *
    ymsg_new (int id);

//  Destroy the ymsg
void
    ymsg_destroy (ymsg_t **self_p);

//  Parse a zmsg_t and decides whether it is ymsg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
    is_ymsg (zmsg_t *msg_p);

//  Parse a ymsg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
ymsg_t *
    ymsg_decode (zmsg_t **msg_p);

//  Encode ymsg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    ymsg_encode (ymsg_t **self_p);

//  Receive and parse a ymsg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
ymsg_t *
    ymsg_recv (void *input);

//  Receive and parse a ymsg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
ymsg_t *
    ymsg_recv_nowait (void *input);

//  Send the ymsg to the output, and destroy it
int
    ymsg_send (ymsg_t **self_p, void *output);

//  Send the ymsg to the output, and do not destroy it
int
    ymsg_send_again (ymsg_t *self, void *output);

//  Encode the SEND 
zmsg_t *
    ymsg_encode_send (
        byte version,
        uint16_t seq,
        zhash_t *aux,
        zchunk_t *request);

//  Encode the REPLY 
zmsg_t *
    ymsg_encode_reply (
        byte version,
        uint16_t seq,
        uint16_t rep,
        zhash_t *aux,
        zchunk_t *response,
        zchunk_t *request);


//  Send the SEND to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    ymsg_send_send (void *output,
        byte version,
        uint16_t seq,
        zhash_t *aux,
        zchunk_t *request);
    
//  Send the REPLY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    ymsg_send_reply (void *output,
        byte version,
        uint16_t seq,
        uint16_t rep,
        zhash_t *aux,
        zchunk_t *response,
        zchunk_t *request);
    
//  Duplicate the ymsg message
ymsg_t *
    ymsg_dup (ymsg_t *self);

//  Print contents of message to stdout
void
    ymsg_print (ymsg_t *self);

//  Get/set the message routing id
zframe_t *
    ymsg_routing_id (ymsg_t *self);
void
    ymsg_set_routing_id (ymsg_t *self, zframe_t *routing_id);

//  Get the ymsg id and printable command
int
    ymsg_id (ymsg_t *self);
void
    ymsg_set_id (ymsg_t *self, int id);
const char *
    ymsg_command (ymsg_t *self);

//  Get/set the version field
byte
    ymsg_version (ymsg_t *self);
void
    ymsg_set_version (ymsg_t *self, byte version);

//  Get/set the seq field
uint16_t
    ymsg_seq (ymsg_t *self);
void
    ymsg_set_seq (ymsg_t *self, uint16_t seq);

//  Get/set the aux field
zhash_t *
    ymsg_aux (ymsg_t *self);
//  Get the aux field and transfer ownership to caller
zhash_t *
    ymsg_get_aux (ymsg_t *self);
//  Set the aux field, transferring ownership from caller
void
    ymsg_set_aux (ymsg_t *self, zhash_t **aux_p);
    
//  Get/set a value in the aux dictionary
const char *
    ymsg_aux_string (ymsg_t *self,
        const char *key, const char *default_value);
uint64_t
    ymsg_aux_number (ymsg_t *self,
        const char *key, uint64_t default_value);
void
    ymsg_aux_insert (ymsg_t *self,
        const char *key, const char *format, ...);
size_t
    ymsg_aux_size (ymsg_t *self);

//  Get a copy of the request field
zchunk_t *
    ymsg_request (ymsg_t *self);
//  Get the request field and transfer ownership to caller
zchunk_t *
    ymsg_get_request (ymsg_t *self);
//  Set the request field, transferring ownership from caller
void
    ymsg_set_request (ymsg_t *self, zchunk_t **chunk_p);

//  Get/set the rep field
uint16_t
    ymsg_rep (ymsg_t *self);
void
    ymsg_set_rep (ymsg_t *self, uint16_t rep);

//  Get a copy of the response field
zchunk_t *
    ymsg_response (ymsg_t *self);
//  Get the response field and transfer ownership to caller
zchunk_t *
    ymsg_get_response (ymsg_t *self);
//  Set the response field, transferring ownership from caller
void
    ymsg_set_response (ymsg_t *self, zchunk_t **chunk_p);

//  Self test of this class
int
    ymsg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define ymsg_dump           ymsg_print

#ifdef __cplusplus
}
#endif

#endif
