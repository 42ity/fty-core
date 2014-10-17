/*  =========================================================================
    powerdev_msg - power device message
    
    Codec header for powerdev_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: powerdev_msg.xml, or
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

#ifndef __POWERDEV_MSG_H_INCLUDED__
#define __POWERDEV_MSG_H_INCLUDED__

#include <czmq.h>

/*  These are the powerdev_msg messages:

    POWERDEV_STATUS - Actual device status. Values, that allows UPS identification, are specified individually, rest is a dictionary
        deviceid            string      Device ID - nut name
        model               string      Device model
        manufacturer        string      Device manufacturer
        serial              string      Serial number
        type                string      Device type (UPS/ePDU/...)
        status              string      UPS status
        otherproperties     dictionary  Other device properties
*/

#define POWERDEV_MSG_VERSION                1.0

#define POWERDEV_MSG_POWERDEV_STATUS        201

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _powerdev_msg_t powerdev_msg_t;

//  @interface
//  Create a new powerdev_msg
powerdev_msg_t *
    powerdev_msg_new (int id);

//  Destroy the powerdev_msg
void
    powerdev_msg_destroy (powerdev_msg_t **self_p);

//  Parse a powerdev_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
powerdev_msg_t *
    powerdev_msg_decode (zmsg_t **msg_p);

//  Encode powerdev_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    powerdev_msg_encode (powerdev_msg_t **self_p);

//  Receive and parse a powerdev_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
powerdev_msg_t *
    powerdev_msg_recv (void *input);

//  Receive and parse a powerdev_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
powerdev_msg_t *
    powerdev_msg_recv_nowait (void *input);

//  Send the powerdev_msg to the output, and destroy it
int
    powerdev_msg_send (powerdev_msg_t **self_p, void *output);

//  Send the powerdev_msg to the output, and do not destroy it
int
    powerdev_msg_send_again (powerdev_msg_t *self, void *output);

//  Encode the POWERDEV_STATUS 
zmsg_t *
    powerdev_msg_encode_powerdev_status (
        const char *deviceid,
        const char *model,
        const char *manufacturer,
        const char *serial,
        const char *type,
        const char *status,
        zhash_t *otherproperties);


//  Send the POWERDEV_STATUS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    powerdev_msg_send_powerdev_status (void *output,
        const char *deviceid,
        const char *model,
        const char *manufacturer,
        const char *serial,
        const char *type,
        const char *status,
        zhash_t *otherproperties);
    
//  Duplicate the powerdev_msg message
powerdev_msg_t *
    powerdev_msg_dup (powerdev_msg_t *self);

//  Print contents of message to stdout
void
    powerdev_msg_print (powerdev_msg_t *self);

//  Get/set the message routing id
zframe_t *
    powerdev_msg_routing_id (powerdev_msg_t *self);
void
    powerdev_msg_set_routing_id (powerdev_msg_t *self, zframe_t *routing_id);

//  Get the powerdev_msg id and printable command
int
    powerdev_msg_id (powerdev_msg_t *self);
void
    powerdev_msg_set_id (powerdev_msg_t *self, int id);
const char *
    powerdev_msg_command (powerdev_msg_t *self);

//  Get/set the deviceid field
const char *
    powerdev_msg_deviceid (powerdev_msg_t *self);
void
    powerdev_msg_set_deviceid (powerdev_msg_t *self, const char *format, ...);

//  Get/set the model field
const char *
    powerdev_msg_model (powerdev_msg_t *self);
void
    powerdev_msg_set_model (powerdev_msg_t *self, const char *format, ...);

//  Get/set the manufacturer field
const char *
    powerdev_msg_manufacturer (powerdev_msg_t *self);
void
    powerdev_msg_set_manufacturer (powerdev_msg_t *self, const char *format, ...);

//  Get/set the serial field
const char *
    powerdev_msg_serial (powerdev_msg_t *self);
void
    powerdev_msg_set_serial (powerdev_msg_t *self, const char *format, ...);

//  Get/set the type field
const char *
    powerdev_msg_type (powerdev_msg_t *self);
void
    powerdev_msg_set_type (powerdev_msg_t *self, const char *format, ...);

//  Get/set the status field
const char *
    powerdev_msg_status (powerdev_msg_t *self);
void
    powerdev_msg_set_status (powerdev_msg_t *self, const char *format, ...);

//  Get/set the otherproperties field
zhash_t *
    powerdev_msg_otherproperties (powerdev_msg_t *self);
//  Get the otherproperties field and transfer ownership to caller
zhash_t *
    powerdev_msg_get_otherproperties (powerdev_msg_t *self);
//  Set the otherproperties field, transferring ownership from caller
void
    powerdev_msg_set_otherproperties (powerdev_msg_t *self, zhash_t **otherproperties_p);
    
//  Get/set a value in the otherproperties dictionary
const char *
    powerdev_msg_otherproperties_string (powerdev_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    powerdev_msg_otherproperties_number (powerdev_msg_t *self,
        const char *key, uint64_t default_value);
void
    powerdev_msg_otherproperties_insert (powerdev_msg_t *self,
        const char *key, const char *format, ...);
size_t
    powerdev_msg_otherproperties_size (powerdev_msg_t *self);

//  Self test of this class
int
    powerdev_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define powerdev_msg_dump   powerdev_msg_print

#ifdef __cplusplus
}
#endif

#endif
