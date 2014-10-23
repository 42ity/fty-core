/*  =========================================================================
    asset_msg - assets management protocol

    Codec class for asset_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: asset_msg.xml, or
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
    asset_msg - assets management protocol
@discuss
@end
*/

#include "./asset_msg.h"

//  Structure of our class

struct _asset_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  asset_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *name;                         //  Name of the element
    uint32_t location;                  //  ID of the parent element
    byte location_type;                 //  Type of the parent element, defined in asset_type
    byte type;                          //  Type of the element, defined in asset_type
    zhash_t *ext;                       //  Hash map of extended attributes
    size_t ext_bytes;                   //  Size of dictionary content
    char *device_type;                  //  Type of the device, freeform string not the thing from asset_type
    zlist_t *groups;                    //  List of IDs of groups device belongs to
    zlist_t *powers;                    //  List of encoded link messages
    char *ip;                           //  IP of the device
    char *hostname;                     //  Hostname
    char *fqdn;                         //  Fully qualified domain name
    char *mac;                          //  MAC address of the device
    zmsg_t *msg;                        //  Element that we are extending to the device
    uint16_t src_socket;                //  Source socket
    uint16_t dst_socket;                //  Destination socket
    uint32_t src_location;              //  ID of the src device
    uint32_t dst_location;              //  ID of the parent element
    uint32_t element_id;                //  Unique ID of the asset element
    byte error_id;                      //  Type of the error, enum defined in persistence header file
    zhash_t *element_ids;               //  Unique IDs of the asset element (as a key) mapped to the elements name (as a value)
    size_t element_ids_bytes;           //  Size of dictionary content
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
//  Create a new asset_msg

asset_msg_t *
asset_msg_new (int id)
{
    asset_msg_t *self = (asset_msg_t *) zmalloc (sizeof (asset_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the asset_msg

void
asset_msg_destroy (asset_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        asset_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->name);
        zhash_destroy (&self->ext);
        free (self->device_type);
        if (self->groups)
            zlist_destroy (&self->groups);
        if (self->powers)
            zlist_destroy (&self->powers);
        free (self->ip);
        free (self->hostname);
        free (self->fqdn);
        free (self->mac);
        zmsg_destroy (&self->msg);
        zhash_destroy (&self->element_ids);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a asset_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

asset_msg_t *
asset_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    asset_msg_t *self = asset_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 5))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            GET_STRING (self->name);
            GET_NUMBER4 (self->location);
            GET_NUMBER1 (self->location_type);
            GET_NUMBER1 (self->type);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->ext = zhash_new ();
                zhash_autofree (self->ext);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->ext, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        case ASSET_MSG_DEVICE:
            GET_STRING (self->device_type);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->groups = zlist_new ();
                zlist_autofree (self->groups);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->groups, string);
                    free (string);
                }
            }
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->powers = zlist_new ();
                zlist_autofree (self->powers);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->powers, string);
                    free (string);
                }
            }
            GET_STRING (self->ip);
            GET_STRING (self->hostname);
            GET_STRING (self->fqdn);
            GET_STRING (self->mac);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case ASSET_MSG_LINK:
            GET_NUMBER2 (self->src_socket);
            GET_NUMBER2 (self->dst_socket);
            GET_NUMBER4 (self->src_location);
            GET_NUMBER4 (self->dst_location);
            break;

        case ASSET_MSG_GET_ELEMENT:
            GET_NUMBER4 (self->element_id);
            GET_NUMBER1 (self->type);
            break;

        case ASSET_MSG_RETURN_ELEMENT:
            GET_NUMBER4 (self->element_id);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case ASSET_MSG_UPDATE_ELEMENT:
            GET_NUMBER4 (self->element_id);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case ASSET_MSG_INSERT_ELEMENT:
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case ASSET_MSG_DELETE_ELEMENT:
            GET_NUMBER4 (self->element_id);
            GET_NUMBER1 (self->type);
            break;

        case ASSET_MSG_OK:
            GET_NUMBER4 (self->element_id);
            break;

        case ASSET_MSG_FAIL:
            GET_NUMBER1 (self->error_id);
            break;

        case ASSET_MSG_GET_ELEMENTS:
            GET_NUMBER1 (self->type);
            break;

        case ASSET_MSG_RETURN_ELEMENTS:
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->element_ids = zhash_new ();
                zhash_autofree (self->element_ids);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->element_ids, key, value);
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
        asset_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode asset_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
asset_msg_encode (asset_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    asset_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  location is a 4-byte integer
            frame_size += 4;
            //  location_type is a 1-byte integer
            frame_size += 1;
            //  type is a 1-byte integer
            frame_size += 1;
            //  ext is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->ext) {
                self->ext_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    self->ext_bytes += 1 + strlen ((const char *) zhash_cursor (self->ext));
                    self->ext_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            frame_size += self->ext_bytes;
            break;
            
        case ASSET_MSG_DEVICE:
            //  device_type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->device_type)
                frame_size += strlen (self->device_type);
            //  groups is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->groups) {
                //  Add up size of list contents
                char *groups = (char *) zlist_first (self->groups);
                while (groups) {
                    frame_size += 4 + strlen (groups);
                    groups = (char *) zlist_next (self->groups);
                }
            }
            //  powers is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->powers) {
                //  Add up size of list contents
                char *powers = (char *) zlist_first (self->powers);
                while (powers) {
                    frame_size += 4 + strlen (powers);
                    powers = (char *) zlist_next (self->powers);
                }
            }
            //  ip is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ip)
                frame_size += strlen (self->ip);
            //  hostname is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->hostname)
                frame_size += strlen (self->hostname);
            //  fqdn is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->fqdn)
                frame_size += strlen (self->fqdn);
            //  mac is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mac)
                frame_size += strlen (self->mac);
            break;
            
        case ASSET_MSG_LINK:
            //  src_socket is a 2-byte integer
            frame_size += 2;
            //  dst_socket is a 2-byte integer
            frame_size += 2;
            //  src_location is a 4-byte integer
            frame_size += 4;
            //  dst_location is a 4-byte integer
            frame_size += 4;
            break;
            
        case ASSET_MSG_GET_ELEMENT:
            //  element_id is a 4-byte integer
            frame_size += 4;
            //  type is a 1-byte integer
            frame_size += 1;
            break;
            
        case ASSET_MSG_RETURN_ELEMENT:
            //  element_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case ASSET_MSG_UPDATE_ELEMENT:
            //  element_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case ASSET_MSG_INSERT_ELEMENT:
            break;
            
        case ASSET_MSG_DELETE_ELEMENT:
            //  element_id is a 4-byte integer
            frame_size += 4;
            //  type is a 1-byte integer
            frame_size += 1;
            break;
            
        case ASSET_MSG_OK:
            //  element_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case ASSET_MSG_FAIL:
            //  error_id is a 1-byte integer
            frame_size += 1;
            break;
            
        case ASSET_MSG_GET_ELEMENTS:
            //  type is a 1-byte integer
            frame_size += 1;
            break;
            
        case ASSET_MSG_RETURN_ELEMENTS:
            //  element_ids is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->element_ids) {
                self->element_ids_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->element_ids);
                while (item) {
                    self->element_ids_bytes += 1 + strlen ((const char *) zhash_cursor (self->element_ids));
                    self->element_ids_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->element_ids);
                }
            }
            frame_size += self->element_ids_bytes;
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 5);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER4 (self->location);
            PUT_NUMBER1 (self->location_type);
            PUT_NUMBER1 (self->type);
            if (self->ext) {
                PUT_NUMBER4 (zhash_size (self->ext));
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->ext));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case ASSET_MSG_DEVICE:
            if (self->device_type) {
                PUT_STRING (self->device_type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->groups) {
                PUT_NUMBER4 (zlist_size (self->groups));
                char *groups = (char *) zlist_first (self->groups);
                while (groups) {
                    PUT_LONGSTR (groups);
                    groups = (char *) zlist_next (self->groups);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            if (self->powers) {
                PUT_NUMBER4 (zlist_size (self->powers));
                char *powers = (char *) zlist_first (self->powers);
                while (powers) {
                    PUT_LONGSTR (powers);
                    powers = (char *) zlist_next (self->powers);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            if (self->ip) {
                PUT_STRING (self->ip);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->hostname) {
                PUT_STRING (self->hostname);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->fqdn) {
                PUT_STRING (self->fqdn);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->mac) {
                PUT_STRING (self->mac);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case ASSET_MSG_LINK:
            PUT_NUMBER2 (self->src_socket);
            PUT_NUMBER2 (self->dst_socket);
            PUT_NUMBER4 (self->src_location);
            PUT_NUMBER4 (self->dst_location);
            break;

        case ASSET_MSG_GET_ELEMENT:
            PUT_NUMBER4 (self->element_id);
            PUT_NUMBER1 (self->type);
            break;

        case ASSET_MSG_RETURN_ELEMENT:
            PUT_NUMBER4 (self->element_id);
            break;

        case ASSET_MSG_UPDATE_ELEMENT:
            PUT_NUMBER4 (self->element_id);
            break;

        case ASSET_MSG_INSERT_ELEMENT:
            break;

        case ASSET_MSG_DELETE_ELEMENT:
            PUT_NUMBER4 (self->element_id);
            PUT_NUMBER1 (self->type);
            break;

        case ASSET_MSG_OK:
            PUT_NUMBER4 (self->element_id);
            break;

        case ASSET_MSG_FAIL:
            PUT_NUMBER1 (self->error_id);
            break;

        case ASSET_MSG_GET_ELEMENTS:
            PUT_NUMBER1 (self->type);
            break;

        case ASSET_MSG_RETURN_ELEMENTS:
            if (self->element_ids) {
                PUT_NUMBER4 (zhash_size (self->element_ids));
                char *item = (char *) zhash_first (self->element_ids);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->element_ids));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->element_ids);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        asset_msg_destroy (self_p);
        return NULL;
    }
    //  Now send the message field if there is any
    if (self->id == ASSET_MSG_DEVICE) {
        if (self->msg) {
            zframe_t *frame = zmsg_pop (self->msg);
            while (frame) {
                zmsg_append (msg, &frame);
                frame = zmsg_pop (self->msg);
            }
        }
        else {
            zframe_t *frame = zframe_new (NULL, 0);
            zmsg_append (msg, &frame);
        }
    }
    //  Now send the message field if there is any
    if (self->id == ASSET_MSG_RETURN_ELEMENT) {
        if (self->msg) {
            zframe_t *frame = zmsg_pop (self->msg);
            while (frame) {
                zmsg_append (msg, &frame);
                frame = zmsg_pop (self->msg);
            }
        }
        else {
            zframe_t *frame = zframe_new (NULL, 0);
            zmsg_append (msg, &frame);
        }
    }
    //  Now send the message field if there is any
    if (self->id == ASSET_MSG_UPDATE_ELEMENT) {
        if (self->msg) {
            zframe_t *frame = zmsg_pop (self->msg);
            while (frame) {
                zmsg_append (msg, &frame);
                frame = zmsg_pop (self->msg);
            }
        }
        else {
            zframe_t *frame = zframe_new (NULL, 0);
            zmsg_append (msg, &frame);
        }
    }
    //  Now send the message field if there is any
    if (self->id == ASSET_MSG_INSERT_ELEMENT) {
        if (self->msg) {
            zframe_t *frame = zmsg_pop (self->msg);
            while (frame) {
                zmsg_append (msg, &frame);
                frame = zmsg_pop (self->msg);
            }
        }
        else {
            zframe_t *frame = zframe_new (NULL, 0);
            zmsg_append (msg, &frame);
        }
    }
    //  Destroy asset_msg object
    asset_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a asset_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

asset_msg_t *
asset_msg_recv (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv (input);
    if (!msg)
        return NULL;            //  Interrupted
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    asset_msg_t *asset_msg = asset_msg_decode (&msg);
    if (asset_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        asset_msg->routing_id = routing_id;

    return asset_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a asset_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

asset_msg_t *
asset_msg_recv_nowait (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv_nowait (input);
    if (!msg)
        return NULL;            //  Interrupted
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    asset_msg_t *asset_msg = asset_msg_decode (&msg);
    if (asset_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        asset_msg->routing_id = routing_id;

    return asset_msg;
}


//  --------------------------------------------------------------------------
//  Send the asset_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
asset_msg_send (asset_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    asset_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode asset_msg message to a single zmsg
    zmsg_t *msg = asset_msg_encode (self_p);
    
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
//  Send the asset_msg to the output, and do not destroy it

int
asset_msg_send_again (asset_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = asset_msg_dup (self);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode ELEMENT message

zmsg_t * 
asset_msg_encode_element (
    const char *name,
    uint32_t location,
    byte location_type,
    byte type,
    zhash_t *ext)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_ELEMENT);
    asset_msg_set_name (self, name);
    asset_msg_set_location (self, location);
    asset_msg_set_location_type (self, location_type);
    asset_msg_set_type (self, type);
    zhash_t *ext_copy = zhash_dup (ext);
    asset_msg_set_ext (self, &ext_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DEVICE message

zmsg_t * 
asset_msg_encode_device (
    const char *device_type,
    zlist_t *groups,
    zlist_t *powers,
    const char *ip,
    const char *hostname,
    const char *fqdn,
    const char *mac,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_DEVICE);
    asset_msg_set_device_type (self, device_type);
    zlist_t *groups_copy = zlist_dup (groups);
    asset_msg_set_groups (self, &groups_copy);
    zlist_t *powers_copy = zlist_dup (powers);
    asset_msg_set_powers (self, &powers_copy);
    asset_msg_set_ip (self, ip);
    asset_msg_set_hostname (self, hostname);
    asset_msg_set_fqdn (self, fqdn);
    asset_msg_set_mac (self, mac);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode LINK message

zmsg_t * 
asset_msg_encode_link (
    uint16_t src_socket,
    uint16_t dst_socket,
    uint32_t src_location,
    uint32_t dst_location)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_LINK);
    asset_msg_set_src_socket (self, src_socket);
    asset_msg_set_dst_socket (self, dst_socket);
    asset_msg_set_src_location (self, src_location);
    asset_msg_set_dst_location (self, dst_location);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_ELEMENT message

zmsg_t * 
asset_msg_encode_get_element (
    uint32_t element_id,
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_GET_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    asset_msg_set_type (self, type);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_ELEMENT message

zmsg_t * 
asset_msg_encode_return_element (
    uint32_t element_id,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode UPDATE_ELEMENT message

zmsg_t * 
asset_msg_encode_update_element (
    uint32_t element_id,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_UPDATE_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INSERT_ELEMENT message

zmsg_t * 
asset_msg_encode_insert_element (
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_INSERT_ELEMENT);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DELETE_ELEMENT message

zmsg_t * 
asset_msg_encode_delete_element (
    uint32_t element_id,
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_DELETE_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    asset_msg_set_type (self, type);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode OK message

zmsg_t * 
asset_msg_encode_ok (
    uint32_t element_id)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_OK);
    asset_msg_set_element_id (self, element_id);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode FAIL message

zmsg_t * 
asset_msg_encode_fail (
    byte error_id)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_FAIL);
    asset_msg_set_error_id (self, error_id);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_ELEMENTS message

zmsg_t * 
asset_msg_encode_get_elements (
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_GET_ELEMENTS);
    asset_msg_set_type (self, type);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_ELEMENTS message

zmsg_t * 
asset_msg_encode_return_elements (
    zhash_t *element_ids)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
    zhash_t *element_ids_copy = zhash_dup (element_ids);
    asset_msg_set_element_ids (self, &element_ids_copy);
    return asset_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the ELEMENT to the socket in one step

int
asset_msg_send_element (
    void *output,
    const char *name,
    uint32_t location,
    byte location_type,
    byte type,
    zhash_t *ext)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_ELEMENT);
    asset_msg_set_name (self, name);
    asset_msg_set_location (self, location);
    asset_msg_set_location_type (self, location_type);
    asset_msg_set_type (self, type);
    zhash_t *ext_copy = zhash_dup (ext);
    asset_msg_set_ext (self, &ext_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DEVICE to the socket in one step

int
asset_msg_send_device (
    void *output,
    const char *device_type,
    zlist_t *groups,
    zlist_t *powers,
    const char *ip,
    const char *hostname,
    const char *fqdn,
    const char *mac,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_DEVICE);
    asset_msg_set_device_type (self, device_type);
    zlist_t *groups_copy = zlist_dup (groups);
    asset_msg_set_groups (self, &groups_copy);
    zlist_t *powers_copy = zlist_dup (powers);
    asset_msg_set_powers (self, &powers_copy);
    asset_msg_set_ip (self, ip);
    asset_msg_set_hostname (self, hostname);
    asset_msg_set_fqdn (self, fqdn);
    asset_msg_set_mac (self, mac);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the LINK to the socket in one step

int
asset_msg_send_link (
    void *output,
    uint16_t src_socket,
    uint16_t dst_socket,
    uint32_t src_location,
    uint32_t dst_location)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_LINK);
    asset_msg_set_src_socket (self, src_socket);
    asset_msg_set_dst_socket (self, dst_socket);
    asset_msg_set_src_location (self, src_location);
    asset_msg_set_dst_location (self, dst_location);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_ELEMENT to the socket in one step

int
asset_msg_send_get_element (
    void *output,
    uint32_t element_id,
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_GET_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    asset_msg_set_type (self, type);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_ELEMENT to the socket in one step

int
asset_msg_send_return_element (
    void *output,
    uint32_t element_id,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the UPDATE_ELEMENT to the socket in one step

int
asset_msg_send_update_element (
    void *output,
    uint32_t element_id,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_UPDATE_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INSERT_ELEMENT to the socket in one step

int
asset_msg_send_insert_element (
    void *output,
    zmsg_t *msg)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_INSERT_ELEMENT);
    zmsg_t *msg_copy = zmsg_dup (msg);
    asset_msg_set_msg (self, &msg_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELETE_ELEMENT to the socket in one step

int
asset_msg_send_delete_element (
    void *output,
    uint32_t element_id,
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_DELETE_ELEMENT);
    asset_msg_set_element_id (self, element_id);
    asset_msg_set_type (self, type);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the OK to the socket in one step

int
asset_msg_send_ok (
    void *output,
    uint32_t element_id)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_OK);
    asset_msg_set_element_id (self, element_id);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the FAIL to the socket in one step

int
asset_msg_send_fail (
    void *output,
    byte error_id)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_FAIL);
    asset_msg_set_error_id (self, error_id);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_ELEMENTS to the socket in one step

int
asset_msg_send_get_elements (
    void *output,
    byte type)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_GET_ELEMENTS);
    asset_msg_set_type (self, type);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_ELEMENTS to the socket in one step

int
asset_msg_send_return_elements (
    void *output,
    zhash_t *element_ids)
{
    asset_msg_t *self = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
    zhash_t *element_ids_copy = zhash_dup (element_ids);
    asset_msg_set_element_ids (self, &element_ids_copy);
    return asset_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the asset_msg message

asset_msg_t *
asset_msg_dup (asset_msg_t *self)
{
    if (!self)
        return NULL;
        
    asset_msg_t *copy = asset_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            copy->name = self->name? strdup (self->name): NULL;
            copy->location = self->location;
            copy->location_type = self->location_type;
            copy->type = self->type;
            copy->ext = self->ext? zhash_dup (self->ext): NULL;
            break;

        case ASSET_MSG_DEVICE:
            copy->device_type = self->device_type? strdup (self->device_type): NULL;
            copy->groups = self->groups? zlist_dup (self->groups): NULL;
            copy->powers = self->powers? zlist_dup (self->powers): NULL;
            copy->ip = self->ip? strdup (self->ip): NULL;
            copy->hostname = self->hostname? strdup (self->hostname): NULL;
            copy->fqdn = self->fqdn? strdup (self->fqdn): NULL;
            copy->mac = self->mac? strdup (self->mac): NULL;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case ASSET_MSG_LINK:
            copy->src_socket = self->src_socket;
            copy->dst_socket = self->dst_socket;
            copy->src_location = self->src_location;
            copy->dst_location = self->dst_location;
            break;

        case ASSET_MSG_GET_ELEMENT:
            copy->element_id = self->element_id;
            copy->type = self->type;
            break;

        case ASSET_MSG_RETURN_ELEMENT:
            copy->element_id = self->element_id;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case ASSET_MSG_UPDATE_ELEMENT:
            copy->element_id = self->element_id;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case ASSET_MSG_INSERT_ELEMENT:
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case ASSET_MSG_DELETE_ELEMENT:
            copy->element_id = self->element_id;
            copy->type = self->type;
            break;

        case ASSET_MSG_OK:
            copy->element_id = self->element_id;
            break;

        case ASSET_MSG_FAIL:
            copy->error_id = self->error_id;
            break;

        case ASSET_MSG_GET_ELEMENTS:
            copy->type = self->type;
            break;

        case ASSET_MSG_RETURN_ELEMENTS:
            copy->element_ids = self->element_ids? zhash_dup (self->element_ids): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
asset_msg_print (asset_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            zsys_debug ("ASSET_MSG_ELEMENT:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    location=%ld", (long) self->location);
            zsys_debug ("    location_type=%ld", (long) self->location_type);
            zsys_debug ("    type=%ld", (long) self->type);
            zsys_debug ("    ext=");
            if (self->ext) {
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->ext), item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
        case ASSET_MSG_DEVICE:
            zsys_debug ("ASSET_MSG_DEVICE:");
            if (self->device_type)
                zsys_debug ("    device_type='%s'", self->device_type);
            else
                zsys_debug ("    device_type=");
            zsys_debug ("    groups=");
            if (self->groups) {
                char *groups = (char *) zlist_first (self->groups);
                while (groups) {
                    zsys_debug ("        '%s'", groups);
                    groups = (char *) zlist_next (self->groups);
                }
            }
            zsys_debug ("    powers=");
            if (self->powers) {
                char *powers = (char *) zlist_first (self->powers);
                while (powers) {
                    zsys_debug ("        '%s'", powers);
                    powers = (char *) zlist_next (self->powers);
                }
            }
            if (self->ip)
                zsys_debug ("    ip='%s'", self->ip);
            else
                zsys_debug ("    ip=");
            if (self->hostname)
                zsys_debug ("    hostname='%s'", self->hostname);
            else
                zsys_debug ("    hostname=");
            if (self->fqdn)
                zsys_debug ("    fqdn='%s'", self->fqdn);
            else
                zsys_debug ("    fqdn=");
            if (self->mac)
                zsys_debug ("    mac='%s'", self->mac);
            else
                zsys_debug ("    mac=");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case ASSET_MSG_LINK:
            zsys_debug ("ASSET_MSG_LINK:");
            zsys_debug ("    src_socket=%ld", (long) self->src_socket);
            zsys_debug ("    dst_socket=%ld", (long) self->dst_socket);
            zsys_debug ("    src_location=%ld", (long) self->src_location);
            zsys_debug ("    dst_location=%ld", (long) self->dst_location);
            break;
            
        case ASSET_MSG_GET_ELEMENT:
            zsys_debug ("ASSET_MSG_GET_ELEMENT:");
            zsys_debug ("    element_id=%ld", (long) self->element_id);
            zsys_debug ("    type=%ld", (long) self->type);
            break;
            
        case ASSET_MSG_RETURN_ELEMENT:
            zsys_debug ("ASSET_MSG_RETURN_ELEMENT:");
            zsys_debug ("    element_id=%ld", (long) self->element_id);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case ASSET_MSG_UPDATE_ELEMENT:
            zsys_debug ("ASSET_MSG_UPDATE_ELEMENT:");
            zsys_debug ("    element_id=%ld", (long) self->element_id);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case ASSET_MSG_INSERT_ELEMENT:
            zsys_debug ("ASSET_MSG_INSERT_ELEMENT:");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case ASSET_MSG_DELETE_ELEMENT:
            zsys_debug ("ASSET_MSG_DELETE_ELEMENT:");
            zsys_debug ("    element_id=%ld", (long) self->element_id);
            zsys_debug ("    type=%ld", (long) self->type);
            break;
            
        case ASSET_MSG_OK:
            zsys_debug ("ASSET_MSG_OK:");
            zsys_debug ("    element_id=%ld", (long) self->element_id);
            break;
            
        case ASSET_MSG_FAIL:
            zsys_debug ("ASSET_MSG_FAIL:");
            zsys_debug ("    error_id=%ld", (long) self->error_id);
            break;
            
        case ASSET_MSG_GET_ELEMENTS:
            zsys_debug ("ASSET_MSG_GET_ELEMENTS:");
            zsys_debug ("    type=%ld", (long) self->type);
            break;
            
        case ASSET_MSG_RETURN_ELEMENTS:
            zsys_debug ("ASSET_MSG_RETURN_ELEMENTS:");
            zsys_debug ("    element_ids=");
            if (self->element_ids) {
                char *item = (char *) zhash_first (self->element_ids);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->element_ids), item);
                    item = (char *) zhash_next (self->element_ids);
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
asset_msg_routing_id (asset_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
asset_msg_set_routing_id (asset_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the asset_msg id

int
asset_msg_id (asset_msg_t *self)
{
    assert (self);
    return self->id;
}

void
asset_msg_set_id (asset_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
asset_msg_command (asset_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case ASSET_MSG_ELEMENT:
            return ("ELEMENT");
            break;
        case ASSET_MSG_DEVICE:
            return ("DEVICE");
            break;
        case ASSET_MSG_LINK:
            return ("LINK");
            break;
        case ASSET_MSG_GET_ELEMENT:
            return ("GET_ELEMENT");
            break;
        case ASSET_MSG_RETURN_ELEMENT:
            return ("RETURN_ELEMENT");
            break;
        case ASSET_MSG_UPDATE_ELEMENT:
            return ("UPDATE_ELEMENT");
            break;
        case ASSET_MSG_INSERT_ELEMENT:
            return ("INSERT_ELEMENT");
            break;
        case ASSET_MSG_DELETE_ELEMENT:
            return ("DELETE_ELEMENT");
            break;
        case ASSET_MSG_OK:
            return ("OK");
            break;
        case ASSET_MSG_FAIL:
            return ("FAIL");
            break;
        case ASSET_MSG_GET_ELEMENTS:
            return ("GET_ELEMENTS");
            break;
        case ASSET_MSG_RETURN_ELEMENTS:
            return ("RETURN_ELEMENTS");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the name field

const char *
asset_msg_name (asset_msg_t *self)
{
    assert (self);
    return self->name;
}

void
asset_msg_set_name (asset_msg_t *self, const char *format, ...)
{
    //  Format name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->name);
    self->name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the location field

uint32_t
asset_msg_location (asset_msg_t *self)
{
    assert (self);
    return self->location;
}

void
asset_msg_set_location (asset_msg_t *self, uint32_t location)
{
    assert (self);
    self->location = location;
}


//  --------------------------------------------------------------------------
//  Get/set the location_type field

byte
asset_msg_location_type (asset_msg_t *self)
{
    assert (self);
    return self->location_type;
}

void
asset_msg_set_location_type (asset_msg_t *self, byte location_type)
{
    assert (self);
    self->location_type = location_type;
}


//  --------------------------------------------------------------------------
//  Get/set the type field

byte
asset_msg_type (asset_msg_t *self)
{
    assert (self);
    return self->type;
}

void
asset_msg_set_type (asset_msg_t *self, byte type)
{
    assert (self);
    self->type = type;
}


//  --------------------------------------------------------------------------
//  Get the ext field without transferring ownership

zhash_t *
asset_msg_ext (asset_msg_t *self)
{
    assert (self);
    return self->ext;
}

//  Get the ext field and transfer ownership to caller

zhash_t *
asset_msg_get_ext (asset_msg_t *self)
{
    zhash_t *ext = self->ext;
    self->ext = NULL;
    return ext;
}

//  Set the ext field, transferring ownership from caller

void
asset_msg_set_ext (asset_msg_t *self, zhash_t **ext_p)
{
    assert (self);
    assert (ext_p);
    zhash_destroy (&self->ext);
    self->ext = *ext_p;
    *ext_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the ext dictionary

const char *
asset_msg_ext_string (asset_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->ext)
        value = (const char *) (zhash_lookup (self->ext, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
asset_msg_ext_number (asset_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->ext)
        string = (char *) (zhash_lookup (self->ext, key));
    if (string)
        value = atol (string);

    return value;
}

void
asset_msg_ext_insert (asset_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->ext) {
        self->ext = zhash_new ();
        zhash_autofree (self->ext);
    }
    zhash_update (self->ext, key, string);
    free (string);
}

size_t
asset_msg_ext_size (asset_msg_t *self)
{
    return zhash_size (self->ext);
}


//  --------------------------------------------------------------------------
//  Get/set the device_type field

const char *
asset_msg_device_type (asset_msg_t *self)
{
    assert (self);
    return self->device_type;
}

void
asset_msg_set_device_type (asset_msg_t *self, const char *format, ...)
{
    //  Format device_type from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->device_type);
    self->device_type = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the groups field, without transferring ownership

zlist_t *
asset_msg_groups (asset_msg_t *self)
{
    assert (self);
    return self->groups;
}

//  Get the groups field and transfer ownership to caller

zlist_t *
asset_msg_get_groups (asset_msg_t *self)
{
    assert (self);
    zlist_t *groups = self->groups;
    self->groups = NULL;
    return groups;
}

//  Set the groups field, transferring ownership from caller

void
asset_msg_set_groups (asset_msg_t *self, zlist_t **groups_p)
{
    assert (self);
    assert (groups_p);
    zlist_destroy (&self->groups);
    self->groups = *groups_p;
    *groups_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the groups field, and append a groups value

const char *
asset_msg_groups_first (asset_msg_t *self)
{
    assert (self);
    if (self->groups)
        return (char *) (zlist_first (self->groups));
    else
        return NULL;
}

const char *
asset_msg_groups_next (asset_msg_t *self)
{
    assert (self);
    if (self->groups)
        return (char *) (zlist_next (self->groups));
    else
        return NULL;
}

void
asset_msg_groups_append (asset_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->groups) {
        self->groups = zlist_new ();
        zlist_autofree (self->groups);
    }
    zlist_append (self->groups, string);
    free (string);
}

size_t
asset_msg_groups_size (asset_msg_t *self)
{
    return zlist_size (self->groups);
}


//  --------------------------------------------------------------------------
//  Get the powers field, without transferring ownership

zlist_t *
asset_msg_powers (asset_msg_t *self)
{
    assert (self);
    return self->powers;
}

//  Get the powers field and transfer ownership to caller

zlist_t *
asset_msg_get_powers (asset_msg_t *self)
{
    assert (self);
    zlist_t *powers = self->powers;
    self->powers = NULL;
    return powers;
}

//  Set the powers field, transferring ownership from caller

void
asset_msg_set_powers (asset_msg_t *self, zlist_t **powers_p)
{
    assert (self);
    assert (powers_p);
    zlist_destroy (&self->powers);
    self->powers = *powers_p;
    *powers_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the powers field, and append a powers value

const char *
asset_msg_powers_first (asset_msg_t *self)
{
    assert (self);
    if (self->powers)
        return (char *) (zlist_first (self->powers));
    else
        return NULL;
}

const char *
asset_msg_powers_next (asset_msg_t *self)
{
    assert (self);
    if (self->powers)
        return (char *) (zlist_next (self->powers));
    else
        return NULL;
}

void
asset_msg_powers_append (asset_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->powers) {
        self->powers = zlist_new ();
        zlist_autofree (self->powers);
    }
    zlist_append (self->powers, string);
    free (string);
}

size_t
asset_msg_powers_size (asset_msg_t *self)
{
    return zlist_size (self->powers);
}


//  --------------------------------------------------------------------------
//  Get/set the ip field

const char *
asset_msg_ip (asset_msg_t *self)
{
    assert (self);
    return self->ip;
}

void
asset_msg_set_ip (asset_msg_t *self, const char *format, ...)
{
    //  Format ip from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->ip);
    self->ip = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the hostname field

const char *
asset_msg_hostname (asset_msg_t *self)
{
    assert (self);
    return self->hostname;
}

void
asset_msg_set_hostname (asset_msg_t *self, const char *format, ...)
{
    //  Format hostname from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->hostname);
    self->hostname = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the fqdn field

const char *
asset_msg_fqdn (asset_msg_t *self)
{
    assert (self);
    return self->fqdn;
}

void
asset_msg_set_fqdn (asset_msg_t *self, const char *format, ...)
{
    //  Format fqdn from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->fqdn);
    self->fqdn = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the mac field

const char *
asset_msg_mac (asset_msg_t *self)
{
    assert (self);
    return self->mac;
}

void
asset_msg_set_mac (asset_msg_t *self, const char *format, ...)
{
    //  Format mac from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->mac);
    self->mac = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the msg field without transferring ownership

zmsg_t *
asset_msg_msg (asset_msg_t *self)
{
    assert (self);
    return self->msg;
}

//  Get the msg field and transfer ownership to caller

zmsg_t *
asset_msg_get_msg (asset_msg_t *self)
{
    zmsg_t *msg = self->msg;
    self->msg = NULL;
    return msg;
}

//  Set the msg field, transferring ownership from caller

void
asset_msg_set_msg (asset_msg_t *self, zmsg_t **msg_p)
{
    assert (self);
    assert (msg_p);
    zmsg_destroy (&self->msg);
    self->msg = *msg_p;
    *msg_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the src_socket field

uint16_t
asset_msg_src_socket (asset_msg_t *self)
{
    assert (self);
    return self->src_socket;
}

void
asset_msg_set_src_socket (asset_msg_t *self, uint16_t src_socket)
{
    assert (self);
    self->src_socket = src_socket;
}


//  --------------------------------------------------------------------------
//  Get/set the dst_socket field

uint16_t
asset_msg_dst_socket (asset_msg_t *self)
{
    assert (self);
    return self->dst_socket;
}

void
asset_msg_set_dst_socket (asset_msg_t *self, uint16_t dst_socket)
{
    assert (self);
    self->dst_socket = dst_socket;
}


//  --------------------------------------------------------------------------
//  Get/set the src_location field

uint32_t
asset_msg_src_location (asset_msg_t *self)
{
    assert (self);
    return self->src_location;
}

void
asset_msg_set_src_location (asset_msg_t *self, uint32_t src_location)
{
    assert (self);
    self->src_location = src_location;
}


//  --------------------------------------------------------------------------
//  Get/set the dst_location field

uint32_t
asset_msg_dst_location (asset_msg_t *self)
{
    assert (self);
    return self->dst_location;
}

void
asset_msg_set_dst_location (asset_msg_t *self, uint32_t dst_location)
{
    assert (self);
    self->dst_location = dst_location;
}


//  --------------------------------------------------------------------------
//  Get/set the element_id field

uint32_t
asset_msg_element_id (asset_msg_t *self)
{
    assert (self);
    return self->element_id;
}

void
asset_msg_set_element_id (asset_msg_t *self, uint32_t element_id)
{
    assert (self);
    self->element_id = element_id;
}


//  --------------------------------------------------------------------------
//  Get/set the error_id field

byte
asset_msg_error_id (asset_msg_t *self)
{
    assert (self);
    return self->error_id;
}

void
asset_msg_set_error_id (asset_msg_t *self, byte error_id)
{
    assert (self);
    self->error_id = error_id;
}


//  --------------------------------------------------------------------------
//  Get the element_ids field without transferring ownership

zhash_t *
asset_msg_element_ids (asset_msg_t *self)
{
    assert (self);
    return self->element_ids;
}

//  Get the element_ids field and transfer ownership to caller

zhash_t *
asset_msg_get_element_ids (asset_msg_t *self)
{
    zhash_t *element_ids = self->element_ids;
    self->element_ids = NULL;
    return element_ids;
}

//  Set the element_ids field, transferring ownership from caller

void
asset_msg_set_element_ids (asset_msg_t *self, zhash_t **element_ids_p)
{
    assert (self);
    assert (element_ids_p);
    zhash_destroy (&self->element_ids);
    self->element_ids = *element_ids_p;
    *element_ids_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the element_ids dictionary

const char *
asset_msg_element_ids_string (asset_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->element_ids)
        value = (const char *) (zhash_lookup (self->element_ids, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
asset_msg_element_ids_number (asset_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->element_ids)
        string = (char *) (zhash_lookup (self->element_ids, key));
    if (string)
        value = atol (string);

    return value;
}

void
asset_msg_element_ids_insert (asset_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->element_ids) {
        self->element_ids = zhash_new ();
        zhash_autofree (self->element_ids);
    }
    zhash_update (self->element_ids, key, string);
    free (string);
}

size_t
asset_msg_element_ids_size (asset_msg_t *self)
{
    return zhash_size (self->element_ids);
}



//  --------------------------------------------------------------------------
//  Selftest

int
asset_msg_test (bool verbose)
{
    printf (" * asset_msg: ");

    //  @selftest
    //  Simple create/destroy test
    asset_msg_t *self = asset_msg_new (0);
    assert (self);
    asset_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-asset_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-asset_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    asset_msg_t *copy;
    self = asset_msg_new (ASSET_MSG_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_name (self, "Life is short but Now lasts for ever");
    asset_msg_set_location (self, 123);
    asset_msg_set_location_type (self, 123);
    asset_msg_set_type (self, 123);
    asset_msg_ext_insert (self, "Name", "Brutus");
    asset_msg_ext_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (streq (asset_msg_name (self), "Life is short but Now lasts for ever"));
        assert (asset_msg_location (self) == 123);
        assert (asset_msg_location_type (self) == 123);
        assert (asset_msg_type (self) == 123);
        assert (asset_msg_ext_size (self) == 2);
        assert (streq (asset_msg_ext_string (self, "Name", "?"), "Brutus"));
        assert (asset_msg_ext_number (self, "Age", 0) == 43);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_DEVICE);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_device_type (self, "Life is short but Now lasts for ever");
    asset_msg_groups_append (self, "Name: %s", "Brutus");
    asset_msg_groups_append (self, "Age: %d", 43);
    asset_msg_powers_append (self, "Name: %s", "Brutus");
    asset_msg_powers_append (self, "Age: %d", 43);
    asset_msg_set_ip (self, "Life is short but Now lasts for ever");
    asset_msg_set_hostname (self, "Life is short but Now lasts for ever");
    asset_msg_set_fqdn (self, "Life is short but Now lasts for ever");
    asset_msg_set_mac (self, "Life is short but Now lasts for ever");
    zmsg_t *device_msg = zmsg_new ();
    asset_msg_set_msg (self, &device_msg);
    zmsg_addstr (asset_msg_msg (self), "Hello, World");
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (streq (asset_msg_device_type (self), "Life is short but Now lasts for ever"));
        assert (asset_msg_groups_size (self) == 2);
        assert (streq (asset_msg_groups_first (self), "Name: Brutus"));
        assert (streq (asset_msg_groups_next (self), "Age: 43"));
        assert (asset_msg_powers_size (self) == 2);
        assert (streq (asset_msg_powers_first (self), "Name: Brutus"));
        assert (streq (asset_msg_powers_next (self), "Age: 43"));
        assert (streq (asset_msg_ip (self), "Life is short but Now lasts for ever"));
        assert (streq (asset_msg_hostname (self), "Life is short but Now lasts for ever"));
        assert (streq (asset_msg_fqdn (self), "Life is short but Now lasts for ever"));
        assert (streq (asset_msg_mac (self), "Life is short but Now lasts for ever"));
        assert (zmsg_size (asset_msg_msg (self)) == 1);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_LINK);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_src_socket (self, 123);
    asset_msg_set_dst_socket (self, 123);
    asset_msg_set_src_location (self, 123);
    asset_msg_set_dst_location (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_src_socket (self) == 123);
        assert (asset_msg_dst_socket (self) == 123);
        assert (asset_msg_src_location (self) == 123);
        assert (asset_msg_dst_location (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_GET_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_element_id (self, 123);
    asset_msg_set_type (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_id (self) == 123);
        assert (asset_msg_type (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_element_id (self, 123);
    zmsg_t *return_element_msg = zmsg_new ();
    asset_msg_set_msg (self, &return_element_msg);
    zmsg_addstr (asset_msg_msg (self), "Hello, World");
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_id (self) == 123);
        assert (zmsg_size (asset_msg_msg (self)) == 1);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_UPDATE_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_element_id (self, 123);
    zmsg_t *update_element_msg = zmsg_new ();
    asset_msg_set_msg (self, &update_element_msg);
    zmsg_addstr (asset_msg_msg (self), "Hello, World");
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_id (self) == 123);
        assert (zmsg_size (asset_msg_msg (self)) == 1);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_INSERT_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    zmsg_t *insert_element_msg = zmsg_new ();
    asset_msg_set_msg (self, &insert_element_msg);
    zmsg_addstr (asset_msg_msg (self), "Hello, World");
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (zmsg_size (asset_msg_msg (self)) == 1);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_DELETE_ELEMENT);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_element_id (self, 123);
    asset_msg_set_type (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_id (self) == 123);
        assert (asset_msg_type (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_OK);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_element_id (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_id (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_FAIL);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_error_id (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_error_id (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_GET_ELEMENTS);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_set_type (self, 123);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_type (self) == 123);
        asset_msg_destroy (&self);
    }
    self = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
    
    //  Check that _dup works on empty message
    copy = asset_msg_dup (self);
    assert (copy);
    asset_msg_destroy (&copy);

    asset_msg_element_ids_insert (self, "Name", "Brutus");
    asset_msg_element_ids_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    asset_msg_send_again (self, output);
    asset_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = asset_msg_recv (input);
        assert (self);
        assert (asset_msg_routing_id (self));
        
        assert (asset_msg_element_ids_size (self) == 2);
        assert (streq (asset_msg_element_ids_string (self, "Name", "?"), "Brutus"));
        assert (asset_msg_element_ids_number (self, "Age", 0) == 43);
        asset_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
