/*  =========================================================================
    powerdev_msg - power device message

    Codec class for powerdev_msg.

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

/*
@header
    powerdev_msg - power device message
@discuss
@end
*/

#include "./powerdev_msg.h"

//  Structure of our class

struct _powerdev_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  powerdev_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *deviceid;                     //  Device ID - nut name
    char *model;                        //  Device model
    char *manufacturer;                 //  Device manufacturer
    char *serial;                       //  Serial number
    char *type;                         //  Device type (UPS/ePDU/...)
    char *status;                       //  UPS status
    zhash_t *otherproperties;           //  Other device properties
    size_t otherproperties_bytes;       //  Size of dictionary content
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) \
        goto malformed; \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) \
        goto malformed; \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) \
        goto malformed; \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) \
        goto malformed; \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) \
        goto malformed; \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new powerdev_msg

powerdev_msg_t *
powerdev_msg_new (int id)
{
    powerdev_msg_t *self = (powerdev_msg_t *) zmalloc (sizeof (powerdev_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the powerdev_msg

void
powerdev_msg_destroy (powerdev_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        powerdev_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->deviceid);
        free (self->model);
        free (self->manufacturer);
        free (self->serial);
        free (self->type);
        free (self->status);
        zhash_destroy (&self->otherproperties);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a powerdev_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

powerdev_msg_t *
powerdev_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    powerdev_msg_t *self = powerdev_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 2))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            GET_STRING (self->deviceid);
            GET_STRING (self->model);
            GET_STRING (self->manufacturer);
            GET_STRING (self->serial);
            GET_STRING (self->type);
            GET_STRING (self->status);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->otherproperties = zhash_new ();
                zhash_autofree (self->otherproperties);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->otherproperties, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        default:
            goto malformed;
    }
    //  Successful return
    zframe_destroy (&frame);
    zmsg_destroy (msg_p);
    return self;

    //  Error returns
    malformed:
        zsys_error ("malformed message '%d'\n", self->id);
    empty:
        zframe_destroy (&frame);
        zmsg_destroy (msg_p);
        powerdev_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode powerdev_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
powerdev_msg_encode (powerdev_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    powerdev_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            //  deviceid is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->deviceid)
                frame_size += strlen (self->deviceid);
            //  model is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->model)
                frame_size += strlen (self->model);
            //  manufacturer is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->manufacturer)
                frame_size += strlen (self->manufacturer);
            //  serial is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->serial)
                frame_size += strlen (self->serial);
            //  type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->type)
                frame_size += strlen (self->type);
            //  status is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->status)
                frame_size += strlen (self->status);
            //  otherproperties is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->otherproperties) {
                self->otherproperties_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->otherproperties);
                while (item) {
                    self->otherproperties_bytes += 1 + strlen ((const char *) zhash_cursor (self->otherproperties));
                    self->otherproperties_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->otherproperties);
                }
            }
            frame_size += self->otherproperties_bytes;
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 2);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            if (self->deviceid) {
                PUT_STRING (self->deviceid);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->model) {
                PUT_STRING (self->model);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->manufacturer) {
                PUT_STRING (self->manufacturer);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->serial) {
                PUT_STRING (self->serial);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->type) {
                PUT_STRING (self->type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->status) {
                PUT_STRING (self->status);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->otherproperties) {
                PUT_NUMBER4 (zhash_size (self->otherproperties));
                char *item = (char *) zhash_first (self->otherproperties);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->otherproperties));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->otherproperties);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        powerdev_msg_destroy (self_p);
        return NULL;
    }
    //  Destroy powerdev_msg object
    powerdev_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a powerdev_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

powerdev_msg_t *
powerdev_msg_recv (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv (input);
    if (!msg)
        return NULL;            //  Interrupted
    zmsg_print (msg);
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    powerdev_msg_t *powerdev_msg = powerdev_msg_decode (&msg);
    if (powerdev_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        powerdev_msg->routing_id = routing_id;

    return powerdev_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a powerdev_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

powerdev_msg_t *
powerdev_msg_recv_nowait (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv_nowait (input);
    if (!msg)
        return NULL;            //  Interrupted
    zmsg_print (msg);
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    powerdev_msg_t *powerdev_msg = powerdev_msg_decode (&msg);
    if (powerdev_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        powerdev_msg->routing_id = routing_id;

    return powerdev_msg;
}


//  --------------------------------------------------------------------------
//  Send the powerdev_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
powerdev_msg_send (powerdev_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    powerdev_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode powerdev_msg message to a single zmsg
    zmsg_t *msg = powerdev_msg_encode (self_p);
    
    //  If we're sending to a ROUTER, send the routing_id first
    if (zsocket_type (zsock_resolve (output)) == ZMQ_ROUTER) {
        assert (routing_id);
        zmsg_prepend (msg, &routing_id);
    }
    else
        zframe_destroy (&routing_id);
        
    if (msg && zmsg_send (&msg, output) == 0)
        return 0;
    else
        return -1;              //  Failed to encode, or send
}


//  --------------------------------------------------------------------------
//  Send the powerdev_msg to the output, and do not destroy it

int
powerdev_msg_send_again (powerdev_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = powerdev_msg_dup (self);
    return powerdev_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode POWERDEV_STATUS message

zmsg_t * 
powerdev_msg_encode_powerdev_status (
    const char *deviceid,
    const char *model,
    const char *manufacturer,
    const char *serial,
    const char *type,
    const char *status,
    zhash_t *otherproperties)
{
    powerdev_msg_t *self = powerdev_msg_new (POWERDEV_MSG_POWERDEV_STATUS);
    powerdev_msg_set_deviceid (self, deviceid);
    powerdev_msg_set_model (self, model);
    powerdev_msg_set_manufacturer (self, manufacturer);
    powerdev_msg_set_serial (self, serial);
    powerdev_msg_set_type (self, type);
    powerdev_msg_set_status (self, status);
    zhash_t *otherproperties_copy = zhash_dup (otherproperties);
    powerdev_msg_set_otherproperties (self, &otherproperties_copy);
    return powerdev_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the POWERDEV_STATUS to the socket in one step

int
powerdev_msg_send_powerdev_status (
    void *output,
    const char *deviceid,
    const char *model,
    const char *manufacturer,
    const char *serial,
    const char *type,
    const char *status,
    zhash_t *otherproperties)
{
    powerdev_msg_t *self = powerdev_msg_new (POWERDEV_MSG_POWERDEV_STATUS);
    powerdev_msg_set_deviceid (self, deviceid);
    powerdev_msg_set_model (self, model);
    powerdev_msg_set_manufacturer (self, manufacturer);
    powerdev_msg_set_serial (self, serial);
    powerdev_msg_set_type (self, type);
    powerdev_msg_set_status (self, status);
    zhash_t *otherproperties_copy = zhash_dup (otherproperties);
    powerdev_msg_set_otherproperties (self, &otherproperties_copy);
    return powerdev_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the powerdev_msg message

powerdev_msg_t *
powerdev_msg_dup (powerdev_msg_t *self)
{
    if (!self)
        return NULL;
        
    powerdev_msg_t *copy = powerdev_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            copy->deviceid = self->deviceid? strdup (self->deviceid): NULL;
            copy->model = self->model? strdup (self->model): NULL;
            copy->manufacturer = self->manufacturer? strdup (self->manufacturer): NULL;
            copy->serial = self->serial? strdup (self->serial): NULL;
            copy->type = self->type? strdup (self->type): NULL;
            copy->status = self->status? strdup (self->status): NULL;
            copy->otherproperties = self->otherproperties? zhash_dup (self->otherproperties): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
powerdev_msg_print (powerdev_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            zsys_debug ("POWERDEV_MSG_POWERDEV_STATUS:");
            if (self->deviceid)
                zsys_debug ("    deviceid='%s'", self->deviceid);
            else
                zsys_debug ("    deviceid=");
            if (self->model)
                zsys_debug ("    model='%s'", self->model);
            else
                zsys_debug ("    model=");
            if (self->manufacturer)
                zsys_debug ("    manufacturer='%s'", self->manufacturer);
            else
                zsys_debug ("    manufacturer=");
            if (self->serial)
                zsys_debug ("    serial='%s'", self->serial);
            else
                zsys_debug ("    serial=");
            if (self->type)
                zsys_debug ("    type='%s'", self->type);
            else
                zsys_debug ("    type=");
            if (self->status)
                zsys_debug ("    status='%s'", self->status);
            else
                zsys_debug ("    status=");
            zsys_debug ("    otherproperties=");
            if (self->otherproperties) {
                char *item = (char *) zhash_first (self->otherproperties);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->otherproperties), item);
                    item = (char *) zhash_next (self->otherproperties);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
powerdev_msg_routing_id (powerdev_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
powerdev_msg_set_routing_id (powerdev_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the powerdev_msg id

int
powerdev_msg_id (powerdev_msg_t *self)
{
    assert (self);
    return self->id;
}

void
powerdev_msg_set_id (powerdev_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
powerdev_msg_command (powerdev_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case POWERDEV_MSG_POWERDEV_STATUS:
            return ("POWERDEV_STATUS");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the deviceid field

const char *
powerdev_msg_deviceid (powerdev_msg_t *self)
{
    assert (self);
    return self->deviceid;
}

void
powerdev_msg_set_deviceid (powerdev_msg_t *self, const char *format, ...)
{
    //  Format deviceid from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->deviceid);
    self->deviceid = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the model field

const char *
powerdev_msg_model (powerdev_msg_t *self)
{
    assert (self);
    return self->model;
}

void
powerdev_msg_set_model (powerdev_msg_t *self, const char *format, ...)
{
    //  Format model from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->model);
    self->model = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the manufacturer field

const char *
powerdev_msg_manufacturer (powerdev_msg_t *self)
{
    assert (self);
    return self->manufacturer;
}

void
powerdev_msg_set_manufacturer (powerdev_msg_t *self, const char *format, ...)
{
    //  Format manufacturer from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->manufacturer);
    self->manufacturer = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the serial field

const char *
powerdev_msg_serial (powerdev_msg_t *self)
{
    assert (self);
    return self->serial;
}

void
powerdev_msg_set_serial (powerdev_msg_t *self, const char *format, ...)
{
    //  Format serial from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->serial);
    self->serial = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the type field

const char *
powerdev_msg_type (powerdev_msg_t *self)
{
    assert (self);
    return self->type;
}

void
powerdev_msg_set_type (powerdev_msg_t *self, const char *format, ...)
{
    //  Format type from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->type);
    self->type = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the status field

const char *
powerdev_msg_status (powerdev_msg_t *self)
{
    assert (self);
    return self->status;
}

void
powerdev_msg_set_status (powerdev_msg_t *self, const char *format, ...)
{
    //  Format status from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->status);
    self->status = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the otherproperties field without transferring ownership

zhash_t *
powerdev_msg_otherproperties (powerdev_msg_t *self)
{
    assert (self);
    return self->otherproperties;
}

//  Get the otherproperties field and transfer ownership to caller

zhash_t *
powerdev_msg_get_otherproperties (powerdev_msg_t *self)
{
    zhash_t *otherproperties = self->otherproperties;
    self->otherproperties = NULL;
    return otherproperties;
}

//  Set the otherproperties field, transferring ownership from caller

void
powerdev_msg_set_otherproperties (powerdev_msg_t *self, zhash_t **otherproperties_p)
{
    assert (self);
    assert (otherproperties_p);
    zhash_destroy (&self->otherproperties);
    self->otherproperties = *otherproperties_p;
    *otherproperties_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the otherproperties dictionary

const char *
powerdev_msg_otherproperties_string (powerdev_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->otherproperties)
        value = (const char *) (zhash_lookup (self->otherproperties, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
powerdev_msg_otherproperties_number (powerdev_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->otherproperties)
        string = (char *) (zhash_lookup (self->otherproperties, key));
    if (string)
        value = atol (string);

    return value;
}

void
powerdev_msg_otherproperties_insert (powerdev_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->otherproperties) {
        self->otherproperties = zhash_new ();
        zhash_autofree (self->otherproperties);
    }
    zhash_update (self->otherproperties, key, string);
    free (string);
}

size_t
powerdev_msg_otherproperties_size (powerdev_msg_t *self)
{
    return zhash_size (self->otherproperties);
}



//  --------------------------------------------------------------------------
//  Selftest

int
powerdev_msg_test (bool verbose)
{
    printf (" * powerdev_msg: ");
    if (verbose) {;}	// silence an "unused" warning;
    // TODO: properly fix this in template zproto : zproto_codec_c_v1.gsl
    // so as to not lose the fix upon regeneration of code

    //  @selftest
    //  Simple create/destroy test
    powerdev_msg_t *self = powerdev_msg_new (0);
    assert (self);
    powerdev_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-powerdev_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-powerdev_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    powerdev_msg_t *copy;
    self = powerdev_msg_new (POWERDEV_MSG_POWERDEV_STATUS);
    
    //  Check that _dup works on empty message
    copy = powerdev_msg_dup (self);
    assert (copy);
    powerdev_msg_destroy (&copy);

    powerdev_msg_set_deviceid (self, "Life is short but Now lasts for ever");
    powerdev_msg_set_model (self, "Life is short but Now lasts for ever");
    powerdev_msg_set_manufacturer (self, "Life is short but Now lasts for ever");
    powerdev_msg_set_serial (self, "Life is short but Now lasts for ever");
    powerdev_msg_set_type (self, "Life is short but Now lasts for ever");
    powerdev_msg_set_status (self, "Life is short but Now lasts for ever");
    powerdev_msg_otherproperties_insert (self, "Name", "Brutus");
    powerdev_msg_otherproperties_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    powerdev_msg_send_again (self, output);
    powerdev_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = powerdev_msg_recv (input);
        assert (self);
        assert (powerdev_msg_routing_id (self));
        
        assert (streq (powerdev_msg_deviceid (self), "Life is short but Now lasts for ever"));
        assert (streq (powerdev_msg_model (self), "Life is short but Now lasts for ever"));
        assert (streq (powerdev_msg_manufacturer (self), "Life is short but Now lasts for ever"));
        assert (streq (powerdev_msg_serial (self), "Life is short but Now lasts for ever"));
        assert (streq (powerdev_msg_type (self), "Life is short but Now lasts for ever"));
        assert (streq (powerdev_msg_status (self), "Life is short but Now lasts for ever"));
        assert (powerdev_msg_otherproperties_size (self) == 2);
        assert (streq (powerdev_msg_otherproperties_string (self, "Name", "?"), "Brutus"));
        assert (powerdev_msg_otherproperties_number (self, "Age", 0) == 43);
        powerdev_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
