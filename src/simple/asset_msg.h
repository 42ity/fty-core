/*  =========================================================================
    asset_msg - assets management protocol
    
    Codec header for asset_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: asset_msg.xml
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

#ifndef __ASSET_MSG_H_INCLUDED__
#define __ASSET_MSG_H_INCLUDED__

/*  These are the asset_msg messages:

    ELEMENT - Structure describing asset element
        name                string      Name of the element
        location            number 4    ID of the parent element
        type                number 1    Type of the device, defined in enum somewhere
        ext                 dictionary  Hash map of extended attributes

    GET_ELEMENT - Ask for specific element
        element_id          number 4    Unique ID of the asset element
        type                number 1    Type of the device, defined in enum somewhere

    RETURN_ELEMENT - Returns element we were asked for
        element_id          number 4    Unique ID of the asset element
        msg                 msg         Element to be delivered, NULL if not found

    UPDATE_ELEMENT - Returns element we were asked for
        element_id          number 4    Unique ID of the asset element
        msg                 msg         Element to be updated

    INSERT_ELEMENT - Returns element we were asked for
        msg                 msg         Element to be inserted

    DELETE_ELEMENT - Returns element we were asked for
        element_id          number 4    Unique ID of the element to be deleted
        type                number 1    Type of the device, defined in enum somewhere

    OK - Message from database that everything was processed successfully.
        element_id          number 4    Unique ID of the element that was proccessed

    FAIL - Message from database that something went wrong.
        element_id          number 4    Unique ID of the element that was being proccessed
        error_id            number 1    Type of the error, enum defined somewhere else

    GET_ELEMENTS - Ask for all elements of specific type
        type                number 1    Type of the device, defined in enum somewhere

    RETURN_ELEMENTS - Returns elements we were asked for
        elemenet_ids        dictionary  Unique IDs of the asset element mapped to the elements name
*/

#define ASSET_MSG_VERSION                   1.0

#define ASSET_MSG_ELEMENT                   1
#define ASSET_MSG_GET_ELEMENT               2
#define ASSET_MSG_RETURN_ELEMENT            3
#define ASSET_MSG_UPDATE_ELEMENT            4
#define ASSET_MSG_INSERT_ELEMENT            5
#define ASSET_MSG_DELETE_ELEMENT            6
#define ASSET_MSG_OK                        7
#define ASSET_MSG_FAIL                      8
#define ASSET_MSG_GET_ELEMENTS              9
#define ASSET_MSG_RETURN_ELEMENTS           10

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _asset_msg_t asset_msg_t;

//  @interface
//  Create a new asset_msg
asset_msg_t *
    asset_msg_new (int id);

//  Destroy the asset_msg
void
    asset_msg_destroy (asset_msg_t **self_p);

//  Parse a asset_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
asset_msg_t *
    asset_msg_decode (zmsg_t **msg_p);

//  Encode asset_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    asset_msg_encode (asset_msg_t **self_p);

//  Receive and parse a asset_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
asset_msg_t *
    asset_msg_recv (void *input);

//  Receive and parse a asset_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
asset_msg_t *
    asset_msg_recv_nowait (void *input);

//  Send the asset_msg to the output, and destroy it
int
    asset_msg_send (asset_msg_t **self_p, void *output);

//  Send the asset_msg to the output, and do not destroy it
int
    asset_msg_send_again (asset_msg_t *self, void *output);

//  Encode the ELEMENT 
zmsg_t *
    asset_msg_encode_element (
        const char *name,
        uint32_t location,
        byte type,
        zhash_t *ext);

//  Encode the GET_ELEMENT 
zmsg_t *
    asset_msg_encode_get_element (
        uint32_t element_id,
        byte type);

//  Encode the RETURN_ELEMENT 
zmsg_t *
    asset_msg_encode_return_element (
        uint32_t element_id,
        zmsg_t *msg);

//  Encode the UPDATE_ELEMENT 
zmsg_t *
    asset_msg_encode_update_element (
        uint32_t element_id,
        zmsg_t *msg);

//  Encode the INSERT_ELEMENT 
zmsg_t *
    asset_msg_encode_insert_element (
        zmsg_t *msg);

//  Encode the DELETE_ELEMENT 
zmsg_t *
    asset_msg_encode_delete_element (
        uint32_t element_id,
        byte type);

//  Encode the OK 
zmsg_t *
    asset_msg_encode_ok (
        uint32_t element_id);

//  Encode the FAIL 
zmsg_t *
    asset_msg_encode_fail (
        uint32_t element_id,
        byte error_id);

//  Encode the GET_ELEMENTS 
zmsg_t *
    asset_msg_encode_get_elements (
        byte type);

//  Encode the RETURN_ELEMENTS 
zmsg_t *
    asset_msg_encode_return_elements (
        zhash_t *elemenet_ids);


//  Send the ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_element (void *output,
        const char *name,
        uint32_t location,
        byte type,
        zhash_t *ext);
    
//  Send the GET_ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_get_element (void *output,
        uint32_t element_id,
        byte type);
    
//  Send the RETURN_ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_return_element (void *output,
        uint32_t element_id,
        zmsg_t *msg);
    
//  Send the UPDATE_ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_update_element (void *output,
        uint32_t element_id,
        zmsg_t *msg);
    
//  Send the INSERT_ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_insert_element (void *output,
        zmsg_t *msg);
    
//  Send the DELETE_ELEMENT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_delete_element (void *output,
        uint32_t element_id,
        byte type);
    
//  Send the OK to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_ok (void *output,
        uint32_t element_id);
    
//  Send the FAIL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_fail (void *output,
        uint32_t element_id,
        byte error_id);
    
//  Send the GET_ELEMENTS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_get_elements (void *output,
        byte type);
    
//  Send the RETURN_ELEMENTS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    asset_msg_send_return_elements (void *output,
        zhash_t *elemenet_ids);
    
//  Duplicate the asset_msg message
asset_msg_t *
    asset_msg_dup (asset_msg_t *self);

//  Print contents of message to stdout
void
    asset_msg_print (asset_msg_t *self);

//  Get/set the message routing id
zframe_t *
    asset_msg_routing_id (asset_msg_t *self);
void
    asset_msg_set_routing_id (asset_msg_t *self, zframe_t *routing_id);

//  Get the asset_msg id and printable command
int
    asset_msg_id (asset_msg_t *self);
void
    asset_msg_set_id (asset_msg_t *self, int id);
const char *
    asset_msg_command (asset_msg_t *self);

//  Get/set the name field
const char *
    asset_msg_name (asset_msg_t *self);
void
    asset_msg_set_name (asset_msg_t *self, const char *format, ...);

//  Get/set the location field
uint32_t
    asset_msg_location (asset_msg_t *self);
void
    asset_msg_set_location (asset_msg_t *self, uint32_t location);

//  Get/set the type field
byte
    asset_msg_type (asset_msg_t *self);
void
    asset_msg_set_type (asset_msg_t *self, byte type);

//  Get/set the ext field
zhash_t *
    asset_msg_ext (asset_msg_t *self);
//  Get the ext field and transfer ownership to caller
zhash_t *
    asset_msg_get_ext (asset_msg_t *self);
//  Set the ext field, transferring ownership from caller
void
    asset_msg_set_ext (asset_msg_t *self, zhash_t **ext_p);
    
//  Get/set a value in the ext dictionary
const char *
    asset_msg_ext_string (asset_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    asset_msg_ext_number (asset_msg_t *self,
        const char *key, uint64_t default_value);
void
    asset_msg_ext_insert (asset_msg_t *self,
        const char *key, const char *format, ...);
size_t
    asset_msg_ext_size (asset_msg_t *self);

//  Get/set the element_id field
uint32_t
    asset_msg_element_id (asset_msg_t *self);
void
    asset_msg_set_element_id (asset_msg_t *self, uint32_t element_id);

//  Get a copy of the msg field
zmsg_t *
    asset_msg_msg (asset_msg_t *self);
//  Get the msg field and transfer ownership to caller
zmsg_t *
    asset_msg_get_msg (asset_msg_t *self);
//  Set the msg field, transferring ownership from caller
void
    asset_msg_set_msg (asset_msg_t *self, zmsg_t **msg_p);

//  Get/set the error_id field
byte
    asset_msg_error_id (asset_msg_t *self);
void
    asset_msg_set_error_id (asset_msg_t *self, byte error_id);

//  Get/set the elemenet_ids field
zhash_t *
    asset_msg_elemenet_ids (asset_msg_t *self);
//  Get the elemenet_ids field and transfer ownership to caller
zhash_t *
    asset_msg_get_elemenet_ids (asset_msg_t *self);
//  Set the elemenet_ids field, transferring ownership from caller
void
    asset_msg_set_elemenet_ids (asset_msg_t *self, zhash_t **elemenet_ids_p);
    
//  Get/set a value in the elemenet_ids dictionary
const char *
    asset_msg_elemenet_ids_string (asset_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    asset_msg_elemenet_ids_number (asset_msg_t *self,
        const char *key, uint64_t default_value);
void
    asset_msg_elemenet_ids_insert (asset_msg_t *self,
        const char *key, const char *format, ...);
size_t
    asset_msg_elemenet_ids_size (asset_msg_t *self);

//  Self test of this class
int
    asset_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define asset_msg_dump      asset_msg_print

#ifdef __cplusplus
}
#endif

#endif
