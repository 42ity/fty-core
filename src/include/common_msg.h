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
        rowid               number 4     Unique ID of the client
        msg                 msg          Client

    KEY - Structure describing keys
        keyname             string       Name of the client
        scale               number 4     Scale TODO

    INSERT_KEY - Insert a key
        msg                 msg          Key to be inserted

    DELETE_KEY - Delete a key
        rowid               number 4     Unique ID of the key to be deleted

    RETURN_KEY - Return a key we were asked for
        keytagid            number 4     Unique ID of the key
        msg                 msg          Key 

    GET_KEY - Ask for a key
        rowid               number 4     Unique ID of the key

    NEW_MEASUREMENT - New measurment
        client_name         string       Name of the client
        device_name         string       device name 
        device_type         string       device type 
        keytagname          string       key 
        subkeytag           number 4     subkey 
        value               number 8     measurement value 

    GET_LAST_MEASUREMENTS - Request for the last measurements about the device with device_id
        device_id           number 4     An asset device id

    RETURN_LAST_MEASUREMENTS - The last measurements about the device with device_id
        device_id           number 4     An asset device id
        measurements        dictionary   A map of keytags on values

    CLIENT_INFO - Structure describing client info
        client_id           number 4     A client id
        device_id           number 4     A device id
        info                chunk        Information about device gathered by client (data+ its size)
        date                number 4     Date when this information was gathered

    INSERT_CINFO - Insert a client info
        msg                 msg          Client info to be inserted

    DELETE_CINFO - Delete a client info
        cinfo_id            number 4     Unique ID of the client info to be deleted

    RETURN_CINFO - Return a client info we were asked for
        rowid               number 4     Unique ID of the client info
        msg                 msg          Client info

    DEVICE - Structure describing device
        devicetype_id       number 4     A devicetype id
        name                string       Device name

    INSERT_DEVICE - Insert a
        msg                 msg          Device info to be inserted

    DELETE_DEVICE - Delete a device
        device_id           number 4     Unique ID of the device to be deleted

    RETURN_DEVICE - Return a device we were asked for
        rowid               number 4     Unique ID of the device
        msg                 msg          Device

    DEVICE_TYPE - Structure describing device_type
        name                string       Device type name

    INSERT_DEVTYPE - Insert a device type
        msg                 msg          Device type to be inserted

    DELETE_DEVTYPE - Delete a device type
        devicetype_id       number 4     Unique ID of the device type to be deleted

    RETURN_DEVTYPE - Return a device type we were asked for
        rowid               number 4     Unique ID of the device type
        msg                 msg          Device type

    GET_CLIENT - Ask for a client
        client_id           number 4     Unique ID of the client

    GET_CINFO - Ask for a client info
        cinfo_id            number 4     Unique ID of the client info

    GET_DEVICE - Ask for  a device
        device_id           number 4     Unique ID of the device

    GET_DEVTYPE - Ask for a device type
        devicetype_id       number 4     Unique ID of the device type
*/

#define COMMON_MSG_VERSION                  1.0

#define COMMON_MSG_FAIL                     201
#define COMMON_MSG_DB_OK                    202
#define COMMON_MSG_CLIENT                   203
#define COMMON_MSG_INSERT_CLIENT            204
#define COMMON_MSG_UPDATE_CLIENT            205
#define COMMON_MSG_DELETE_CLIENT            206
#define COMMON_MSG_RETURN_CLIENT            207
#define COMMON_MSG_KEY                      227
#define COMMON_MSG_INSERT_KEY               228
#define COMMON_MSG_DELETE_KEY               230
#define COMMON_MSG_RETURN_KEY               231
#define COMMON_MSG_GET_KEY                  229
#define COMMON_MSG_NEW_MEASUREMENT          239
#define COMMON_MSG_GET_LAST_MEASUREMENTS    238
#define COMMON_MSG_RETURN_LAST_MEASUREMENTS  240
#define COMMON_MSG_CLIENT_INFO              208
#define COMMON_MSG_INSERT_CINFO             209
#define COMMON_MSG_DELETE_CINFO             210
#define COMMON_MSG_RETURN_CINFO             211
#define COMMON_MSG_DEVICE                   212
#define COMMON_MSG_INSERT_DEVICE            213
#define COMMON_MSG_DELETE_DEVICE            215
#define COMMON_MSG_RETURN_DEVICE            216
#define COMMON_MSG_DEVICE_TYPE              217
#define COMMON_MSG_INSERT_DEVTYPE           218
#define COMMON_MSG_DELETE_DEVTYPE           220
#define COMMON_MSG_RETURN_DEVTYPE           221
#define COMMON_MSG_GET_CLIENT               223
#define COMMON_MSG_GET_CINFO                224
#define COMMON_MSG_GET_DEVICE               225
#define COMMON_MSG_GET_DEVTYPE              226

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
        uint32_t rowid,
        zmsg_t *msg);

//  Encode the KEY 
zmsg_t *
    common_msg_encode_key (
        const char *keyname,
        uint32_t scale);

//  Encode the INSERT_KEY 
zmsg_t *
    common_msg_encode_insert_key (
        zmsg_t *msg);

//  Encode the DELETE_KEY 
zmsg_t *
    common_msg_encode_delete_key (
        uint32_t rowid);

//  Encode the RETURN_KEY 
zmsg_t *
    common_msg_encode_return_key (
        uint32_t keytagid,
        zmsg_t *msg);

//  Encode the GET_KEY 
zmsg_t *
    common_msg_encode_get_key (
        uint32_t rowid);

//  Encode the NEW_MEASUREMENT 
zmsg_t *
    common_msg_encode_new_measurement (
        const char *client_name,
        const char *device_name,
        const char *device_type,
        const char *keytagname,
        uint32_t subkeytag,
        uint64_t value);

//  Encode the GET_LAST_MEASUREMENTS 
zmsg_t *
    common_msg_encode_get_last_measurements (
        uint32_t device_id);

//  Encode the RETURN_LAST_MEASUREMENTS 
zmsg_t *
    common_msg_encode_return_last_measurements (
        uint32_t device_id,
        zhash_t *measurements);

//  Encode the CLIENT_INFO 
zmsg_t *
    common_msg_encode_client_info (
        uint32_t client_id,
        uint32_t device_id,
        zchunk_t *info,
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
        uint32_t rowid,
        zmsg_t *msg);

//  Encode the DEVICE 
zmsg_t *
    common_msg_encode_device (
        uint32_t devicetype_id,
        const char *name);

//  Encode the INSERT_DEVICE 
zmsg_t *
    common_msg_encode_insert_device (
        zmsg_t *msg);

//  Encode the DELETE_DEVICE 
zmsg_t *
    common_msg_encode_delete_device (
        uint32_t device_id);

//  Encode the RETURN_DEVICE 
zmsg_t *
    common_msg_encode_return_device (
        uint32_t rowid,
        zmsg_t *msg);

//  Encode the DEVICE_TYPE 
zmsg_t *
    common_msg_encode_device_type (
        const char *name);

//  Encode the INSERT_DEVTYPE 
zmsg_t *
    common_msg_encode_insert_devtype (
        zmsg_t *msg);

//  Encode the DELETE_DEVTYPE 
zmsg_t *
    common_msg_encode_delete_devtype (
        uint32_t devicetype_id);

//  Encode the RETURN_DEVTYPE 
zmsg_t *
    common_msg_encode_return_devtype (
        uint32_t rowid,
        zmsg_t *msg);

//  Encode the GET_CLIENT 
zmsg_t *
    common_msg_encode_get_client (
        uint32_t client_id);

//  Encode the GET_CINFO 
zmsg_t *
    common_msg_encode_get_cinfo (
        uint32_t cinfo_id);

//  Encode the GET_DEVICE 
zmsg_t *
    common_msg_encode_get_device (
        uint32_t device_id);

//  Encode the GET_DEVTYPE 
zmsg_t *
    common_msg_encode_get_devtype (
        uint32_t devicetype_id);


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
        uint32_t rowid,
        zmsg_t *msg);
    
//  Send the KEY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_key (void *output,
        const char *keyname,
        uint32_t scale);
    
//  Send the INSERT_KEY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_insert_key (void *output,
        zmsg_t *msg);
    
//  Send the DELETE_KEY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_delete_key (void *output,
        uint32_t rowid);
    
//  Send the RETURN_KEY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_key (void *output,
        uint32_t keytagid,
        zmsg_t *msg);
    
//  Send the GET_KEY to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_key (void *output,
        uint32_t rowid);
    
//  Send the NEW_MEASUREMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_new_measurement (void *output,
        const char *client_name,
        const char *device_name,
        const char *device_type,
        const char *keytagname,
        uint32_t subkeytag,
        uint64_t value);
    
//  Send the GET_LAST_MEASUREMENTS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_last_measurements (void *output,
        uint32_t device_id);
    
//  Send the RETURN_LAST_MEASUREMENTS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_last_measurements (void *output,
        uint32_t device_id,
        zhash_t *measurements);
    
//  Send the CLIENT_INFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_client_info (void *output,
        uint32_t client_id,
        uint32_t device_id,
        zchunk_t *info,
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
        uint32_t rowid,
        zmsg_t *msg);
    
//  Send the DEVICE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_device (void *output,
        uint32_t devicetype_id,
        const char *name);
    
//  Send the INSERT_DEVICE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_insert_device (void *output,
        zmsg_t *msg);
    
//  Send the DELETE_DEVICE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_delete_device (void *output,
        uint32_t device_id);
    
//  Send the RETURN_DEVICE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_device (void *output,
        uint32_t rowid,
        zmsg_t *msg);
    
//  Send the DEVICE_TYPE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_device_type (void *output,
        const char *name);
    
//  Send the INSERT_DEVTYPE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_insert_devtype (void *output,
        zmsg_t *msg);
    
//  Send the DELETE_DEVTYPE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_delete_devtype (void *output,
        uint32_t devicetype_id);
    
//  Send the RETURN_DEVTYPE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_return_devtype (void *output,
        uint32_t rowid,
        zmsg_t *msg);
    
//  Send the GET_CLIENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_client (void *output,
        uint32_t client_id);
    
//  Send the GET_CINFO to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_cinfo (void *output,
        uint32_t cinfo_id);
    
//  Send the GET_DEVICE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_device (void *output,
        uint32_t device_id);
    
//  Send the GET_DEVTYPE to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    common_msg_send_get_devtype (void *output,
        uint32_t devicetype_id);
    
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

//  Get/set the keyname field
const char *
    common_msg_keyname (common_msg_t *self);
void
    common_msg_set_keyname (common_msg_t *self, const char *format, ...);

//  Get/set the scale field
uint32_t
    common_msg_scale (common_msg_t *self);
void
    common_msg_set_scale (common_msg_t *self, uint32_t scale);

//  Get/set the keytagid field
uint32_t
    common_msg_keytagid (common_msg_t *self);
void
    common_msg_set_keytagid (common_msg_t *self, uint32_t keytagid);

//  Get/set the client_name field
const char *
    common_msg_client_name (common_msg_t *self);
void
    common_msg_set_client_name (common_msg_t *self, const char *format, ...);

//  Get/set the device_name field
const char *
    common_msg_device_name (common_msg_t *self);
void
    common_msg_set_device_name (common_msg_t *self, const char *format, ...);

//  Get/set the device_type field
const char *
    common_msg_device_type (common_msg_t *self);
void
    common_msg_set_device_type (common_msg_t *self, const char *format, ...);

//  Get/set the keytagname field
const char *
    common_msg_keytagname (common_msg_t *self);
void
    common_msg_set_keytagname (common_msg_t *self, const char *format, ...);

//  Get/set the subkeytag field
uint32_t
    common_msg_subkeytag (common_msg_t *self);
void
    common_msg_set_subkeytag (common_msg_t *self, uint32_t subkeytag);

//  Get/set the value field
uint64_t
    common_msg_value (common_msg_t *self);
void
    common_msg_set_value (common_msg_t *self, uint64_t value);

//  Get/set the device_id field
uint32_t
    common_msg_device_id (common_msg_t *self);
void
    common_msg_set_device_id (common_msg_t *self, uint32_t device_id);

//  Get/set the measurements field
zhash_t *
    common_msg_measurements (common_msg_t *self);
//  Get the measurements field and transfer ownership to caller
zhash_t *
    common_msg_get_measurements (common_msg_t *self);
//  Set the measurements field, transferring ownership from caller
void
    common_msg_set_measurements (common_msg_t *self, zhash_t **measurements_p);
    
//  Get/set a value in the measurements dictionary
const char *
    common_msg_measurements_string (common_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    common_msg_measurements_number (common_msg_t *self,
        const char *key, uint64_t default_value);
void
    common_msg_measurements_insert (common_msg_t *self,
        const char *key, const char *format, ...);
size_t
    common_msg_measurements_size (common_msg_t *self);

//  Get a copy of the info field
zchunk_t *
    common_msg_info (common_msg_t *self);
//  Get the info field and transfer ownership to caller
zchunk_t *
    common_msg_get_info (common_msg_t *self);
//  Set the info field, transferring ownership from caller
void
    common_msg_set_info (common_msg_t *self, zchunk_t **chunk_p);

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

//  Get/set the devicetype_id field
uint32_t
    common_msg_devicetype_id (common_msg_t *self);
void
    common_msg_set_devicetype_id (common_msg_t *self, uint32_t devicetype_id);

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
