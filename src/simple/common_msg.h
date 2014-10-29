/*  =========================================================================
    common_msg - common messages
    
    Codec header for common_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: common_msg.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
                                                                        
    Copyright (C) 2014 Eaton                                            
                                                                        
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

#ifndef __COMMON_MSG_H_INCLUDED__
#define __COMMON_MSG_H_INCLUDED__

/*  These are the common_msg messages:

    FAIL - A failed message indicates that some error had occured
        errtype             number 1     An error type, defined in enum somewhere
        errorno             number 4     An error id
        errmsg              string       A user visible error string
        erraux              dictionary   An optional additional information about occured error

    DB_OK - An ok message indicates that during the work with db no error had occured
        rowid               number 4     Id of the row processed

    CLIENT - Structure describing client
        name                string       Name of the client

    INSERT_CLIENT - Insert a client
        msg                 msg          Client to be inserted

    UPDATE_CLIENT - Update a client
        client_id           number 4     Unique ID of the client to be updated
        msg                 msg          Client to be updated

    DELETE_CLIENT - Delete a client
        client_id           number 4     Unique ID of the client to be deleted

    RETURN_CLIENT - Return a client we were asked for
        client_id           number 4     Unique ID of the client
        msg                 msg          Client

    CLIENT_INFO - Structure describing client info
        client_id           number 4     A client id
        device_id           number 4     A device id
        info                string       Information about device gathered by client
        date                number 4     Date when this information was gathered

    INSERT_CINFO - Insert a client info
        msg                 msg          Client info to be inserted

    DELETE_CINFO - Delete a client info
        cinfo_id            number 4     Unique ID of the client info to be deleted

    RETURN_CINFO - Return a client info we were asked for
        cinfo_id            number 4     Unique ID of the client info
        msg                 msg          Client info
*/

#define COMMON_MSG_VERSION                  1.0

#define COMMON_MSG_FAIL                     201
#define COMMON_MSG_DB_OK                    202
#define COMMON_MSG_CLIENT                   203
#define COMMON_MSG_INSERT_CLIENT            204
#define COMMON_MSG_UPDATE_CLIENT            205
#define COMMON_MSG_DELETE_CLIENT            206
#define COMMON_MSG_RETURN_CLIENT            207
#define COMMON_MSG_CLIENT_INFO              208
#define COMMON_MSG_INSERT_CINFO             209
#define COMMON_MSG_DELETE_CINFO             210
#define COMMON_MSG_RETURN_CINFO             211

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _common_msg_t common_msg_t;

//  @interface
//  Create a new common_msg
common_msg_t *
    common_msg_new (int id);

//  Destroy the common_msg
void
    common_msg_destroy (common_msg_t **self_p);

//  Parse a common_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
common_msg_t *
    common_msg_decode (zmsg_t **msg_p);

//  Encode common_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    common_msg_encode (common_msg_t **self_p);

//  Receive and parse a common_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
common_msg_t *
    common_msg_recv (void *input);

//  Receive and parse a common_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
common_msg_t *
    common_msg_recv_nowait (void *input);

//  Send the common_msg to the output, and destroy it
int
    common_msg_send (common_msg_t **self_p, void *output);

//  Send the common_msg to the output, and do not destroy it
int
    common_msg_send_again (common_msg_t *self, void *output);

//  Encode the FAIL 
zmsg_t *
    common_msg_encode_fail (
        byte errtype,
        uint32_t errorno,
        const char *errmsg,
        zhash_t *erraux);

//  Encode the DB_OK 
zmsg_t *
    common_msg_encode_db_ok (
        uint32_t rowid);

//  Encode the CLIENT 
zmsg_t *
    common_msg_encode_client (
        const char *name);

//  Encode the INSERT_CLIENT 
zmsg_t *
    common_msg_encode_insert_client (
        zmsg_t *msg);

//  Encode the UPDATE_CLIENT 
zmsg_t *
    common_msg_encode_update_client (
        uint32_t client_id,
        zmsg_t *msg);

//  Encode the DELETE_CLIENT 
zmsg_t *
    common_msg_encode_delete_client (
        uint32_t client_id);

//  Encode the RETURN_CLIENT 
zmsg_t *
    common_msg_encode_return_client (
        uint32_t client_id,
        zmsg_t *msg);

//  Encode the CLIENT_INFO 
zmsg_t *
    common_msg_encode_client_info (
        uint32_t client_id,
        uint32_t device_id,
        const char *info,
        uint32_t date);

//  Encode the INSERT_CINFO 
zmsg_t *
    common_msg_encode_insert_cinfo (
        zmsg_t *msg);

//  Encode the DELETE_CINFO 
zmsg_t *
    common_msg_encode_delete_cinfo (
        uint32_t cinfo_id);

//  Encode the RETURN_CINFO 
zmsg_t *
    common_msg_encode_return_cinfo (
        uint32_t cinfo_id,
        zmsg_t *msg);


//  Send the FAIL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_fail (void *output,
        byte errtype,
        uint32_t errorno,
        const char *errmsg,
        zhash_t *erraux);
    
//  Send the DB_OK to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_db_ok (void *output,
        uint32_t rowid);
    
//  Send the CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_client (void *output,
        const char *name);
    
//  Send the INSERT_CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_insert_client (void *output,
        zmsg_t *msg);
    
//  Send the UPDATE_CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_update_client (void *output,
        uint32_t client_id,
        zmsg_t *msg);
    
//  Send the DELETE_CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_delete_client (void *output,
        uint32_t client_id);
    
//  Send the RETURN_CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_client (void *output,
        uint32_t client_id,
        zmsg_t *msg);
    
//  Send the CLIENT_INFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_client_info (void *output,
        uint32_t client_id,
        uint32_t device_id,
        const char *info,
        uint32_t date);
    
//  Send the INSERT_CINFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_insert_cinfo (void *output,
        zmsg_t *msg);
    
//  Send the DELETE_CINFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_delete_cinfo (void *output,
        uint32_t cinfo_id);
    
//  Send the RETURN_CINFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_cinfo (void *output,
        uint32_t cinfo_id,
        zmsg_t *msg);
    
//  Duplicate the common_msg message
common_msg_t *
    common_msg_dup (common_msg_t *self);

//  Print contents of message to stdout
void
    common_msg_print (common_msg_t *self);

//  Get/set the message routing id
zframe_t *
    common_msg_routing_id (common_msg_t *self);
void
    common_msg_set_routing_id (common_msg_t *self, zframe_t *routing_id);

//  Get the common_msg id and printable command
int
    common_msg_id (common_msg_t *self);
void
    common_msg_set_id (common_msg_t *self, int id);
const char *
    common_msg_command (common_msg_t *self);

//  Get/set the errtype field
byte
    common_msg_errtype (common_msg_t *self);
void
    common_msg_set_errtype (common_msg_t *self, byte errtype);

//  Get/set the errorno field
uint32_t
    common_msg_errorno (common_msg_t *self);
void
    common_msg_set_errorno (common_msg_t *self, uint32_t errorno);

//  Get/set the errmsg field
const char *
    common_msg_errmsg (common_msg_t *self);
void
    common_msg_set_errmsg (common_msg_t *self, const char *format, ...);

//  Get/set the erraux field
zhash_t *
    common_msg_erraux (common_msg_t *self);
//  Get the erraux field and transfer ownership to caller
zhash_t *
    common_msg_get_erraux (common_msg_t *self);
//  Set the erraux field, transferring ownership from caller
void
    common_msg_set_erraux (common_msg_t *self, zhash_t **erraux_p);
    
//  Get/set a value in the erraux dictionary
const char *
    common_msg_erraux_string (common_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    common_msg_erraux_number (common_msg_t *self,
        const char *key, uint64_t default_value);
void
    common_msg_erraux_insert (common_msg_t *self,
        const char *key, const char *format, ...);
size_t
    common_msg_erraux_size (common_msg_t *self);

//  Get/set the rowid field
uint32_t
    common_msg_rowid (common_msg_t *self);
void
    common_msg_set_rowid (common_msg_t *self, uint32_t rowid);

//  Get/set the name field
const char *
    common_msg_name (common_msg_t *self);
void
    common_msg_set_name (common_msg_t *self, const char *format, ...);

//  Get a copy of the msg field
zmsg_t *
    common_msg_msg (common_msg_t *self);
//  Get the msg field and transfer ownership to caller
zmsg_t *
    common_msg_get_msg (common_msg_t *self);
//  Set the msg field, transferring ownership from caller
void
    common_msg_set_msg (common_msg_t *self, zmsg_t **msg_p);

//  Get/set the client_id field
uint32_t
    common_msg_client_id (common_msg_t *self);
void
    common_msg_set_client_id (common_msg_t *self, uint32_t client_id);

//  Get/set the device_id field
uint32_t
    common_msg_device_id (common_msg_t *self);
void
    common_msg_set_device_id (common_msg_t *self, uint32_t device_id);

//  Get/set the info field
const char *
    common_msg_info (common_msg_t *self);
void
    common_msg_set_info (common_msg_t *self, const char *format, ...);

//  Get/set the date field
uint32_t
    common_msg_date (common_msg_t *self);
void
    common_msg_set_date (common_msg_t *self, uint32_t date);

//  Get/set the cinfo_id field
uint32_t
    common_msg_cinfo_id (common_msg_t *self);
void
    common_msg_set_cinfo_id (common_msg_t *self, uint32_t cinfo_id);

//  Self test of this class
int
    common_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define common_msg_dump     common_msg_print

#ifdef __cplusplus
}
#endif

#endif
