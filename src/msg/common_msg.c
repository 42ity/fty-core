/*  =========================================================================
    common_msg - common messages

    Codec class for common_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: common_msg.xml, or
     * The code generation script that built this file: zproto_codec_c_v1
    ************************************************************************
                                                                        
    Copyright (C) 2014 Eaton                                            
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/*
@header
    common_msg - common messages
@discuss
@end
*/

#include "./common_msg.h"

//  Structure of our class

struct _common_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  common_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    uint16_t mt_id;                     //  Measurement type id
    char *mt_name;                      //  Measurement type name
    char *mt_unit;                      //  Prefered units
    uint16_t mts_id;                    //  Measurement subtype id
    char *mts_name;                     //  Measurement subtype name
    byte mts_scale;                     //  Preffered scale
    byte errtype;                       //   An error type, defined in enum somewhere
    uint32_t errorno;                   //   An error id
    char *errmsg;                       //   A user visible error string
    zhash_t *aux;                       //   An optional additional information about occured error
    size_t aux_bytes;                   //  Size of dictionary content
    uint32_t rowid;                     //   Id of the row processed
    char *name;                         //   Name of the client
    zmsg_t *msg;                        //   Client to be inserted
    uint32_t client_id;                 //   Unique ID of the client to be updated
    char *client_name;                  //   Name of the client
    char *device_name;                  //   device name 
    char *device_type;                  //   device type 
    uint64_t value;                     //   measurement value 
    uint32_t device_id;                 //   A device id
    zchunk_t *info;                     //   Information about device gathered by client (data+ its size)
    uint32_t date;                      //   Date when this information was gathered
    uint32_t cinfo_id;                  //   Unique ID of the client info to be deleted
    uint32_t devicetype_id;             //   A devicetype id
    zlist_t *measurements;              //   A list of string values "keytagid:subkeytagid:value:scale"
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
//  Create a new common_msg

common_msg_t *
common_msg_new (int id)
{
    common_msg_t *self = (common_msg_t *) zmalloc (sizeof (common_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the common_msg

void
common_msg_destroy (common_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        common_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->mt_name);
        free (self->mt_unit);
        free (self->mts_name);
        free (self->errmsg);
        zhash_destroy (&self->aux);
        free (self->name);
        zmsg_destroy (&self->msg);
        free (self->client_name);
        free (self->device_name);
        free (self->device_type);
        zchunk_destroy (&self->info);
        if (self->measurements)
            zlist_destroy (&self->measurements);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  Parse a zmsg_t and decides whether it is common_msg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
is_common_msg (zmsg_t *msg)
{
    if (msg == NULL)
        return false;

    zframe_t *frame = zmsg_first (msg);

    //  Get and check protocol signature
    common_msg_t *self = common_msg_new (0);
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 9))
        goto fail;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
        case COMMON_MSG_GET_MEASURE_TYPE_S:
        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
        case COMMON_MSG_RETURN_MEASURE_TYPE:
        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
        case COMMON_MSG_FAIL:
        case COMMON_MSG_DB_OK:
        case COMMON_MSG_CLIENT:
        case COMMON_MSG_INSERT_CLIENT:
        case COMMON_MSG_UPDATE_CLIENT:
        case COMMON_MSG_DELETE_CLIENT:
        case COMMON_MSG_RETURN_CLIENT:
        case COMMON_MSG_NEW_MEASUREMENT:
        case COMMON_MSG_CLIENT_INFO:
        case COMMON_MSG_INSERT_CINFO:
        case COMMON_MSG_DELETE_CINFO:
        case COMMON_MSG_RETURN_CINFO:
        case COMMON_MSG_DEVICE:
        case COMMON_MSG_INSERT_DEVICE:
        case COMMON_MSG_DELETE_DEVICE:
        case COMMON_MSG_RETURN_DEVICE:
        case COMMON_MSG_DEVICE_TYPE:
        case COMMON_MSG_INSERT_DEVTYPE:
        case COMMON_MSG_DELETE_DEVTYPE:
        case COMMON_MSG_RETURN_DEVTYPE:
        case COMMON_MSG_GET_CLIENT:
        case COMMON_MSG_GET_CINFO:
        case COMMON_MSG_GET_DEVICE:
        case COMMON_MSG_GET_DEVTYPE:
        case COMMON_MSG_GET_LAST_MEASUREMENTS:
        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            common_msg_destroy (&self);
            return true;
        default:
            goto fail;
    }
    fail:
    malformed:
        common_msg_destroy (&self);
        return false;
}

//  --------------------------------------------------------------------------
//  Parse a common_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

common_msg_t *
common_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    common_msg_t *self = common_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 9))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            GET_NUMBER2 (self->mt_id);
            break;

        case COMMON_MSG_GET_MEASURE_TYPE_S:
            GET_STRING (self->mt_name);
            GET_STRING (self->mt_unit);
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            GET_NUMBER2 (self->mt_id);
            GET_NUMBER2 (self->mts_id);
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            GET_NUMBER2 (self->mt_id);
            GET_STRING (self->mts_name);
            GET_NUMBER1 (self->mts_scale);
            break;

        case COMMON_MSG_RETURN_MEASURE_TYPE:
            GET_NUMBER2 (self->mt_id);
            GET_STRING (self->mt_name);
            GET_STRING (self->mt_unit);
            break;

        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            GET_NUMBER2 (self->mts_id);
            GET_NUMBER2 (self->mt_id);
            GET_NUMBER1 (self->mts_scale);
            GET_STRING (self->mts_name);
            break;

        case COMMON_MSG_FAIL:
            GET_NUMBER1 (self->errtype);
            GET_NUMBER4 (self->errorno);
            GET_LONGSTR (self->errmsg);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->aux = zhash_new ();
                zhash_autofree (self->aux);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->aux, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        case COMMON_MSG_DB_OK:
            GET_NUMBER4 (self->rowid);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->aux = zhash_new ();
                zhash_autofree (self->aux);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->aux, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        case COMMON_MSG_CLIENT:
            GET_STRING (self->name);
            break;

        case COMMON_MSG_INSERT_CLIENT:
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_UPDATE_CLIENT:
            GET_NUMBER4 (self->client_id);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DELETE_CLIENT:
            GET_NUMBER4 (self->client_id);
            break;

        case COMMON_MSG_RETURN_CLIENT:
            GET_NUMBER4 (self->rowid);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_NEW_MEASUREMENT:
            GET_STRING (self->client_name);
            GET_STRING (self->device_name);
            GET_STRING (self->device_type);
            GET_NUMBER2 (self->mt_id);
            GET_NUMBER2 (self->mts_id);
            GET_NUMBER8 (self->value);
            break;

        case COMMON_MSG_CLIENT_INFO:
            GET_NUMBER4 (self->client_id);
            GET_NUMBER4 (self->device_id);
            {
                size_t chunk_size;
                GET_NUMBER4 (chunk_size);
                if (self->needle + chunk_size > (self->ceiling))
                    goto malformed;
                self->info = zchunk_new (self->needle, chunk_size);
                self->needle += chunk_size;
            }
            GET_NUMBER4 (self->date);
            break;

        case COMMON_MSG_INSERT_CINFO:
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DELETE_CINFO:
            GET_NUMBER4 (self->cinfo_id);
            break;

        case COMMON_MSG_RETURN_CINFO:
            GET_NUMBER4 (self->rowid);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DEVICE:
            GET_NUMBER4 (self->devicetype_id);
            GET_STRING (self->name);
            break;

        case COMMON_MSG_INSERT_DEVICE:
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DELETE_DEVICE:
            GET_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_RETURN_DEVICE:
            GET_NUMBER4 (self->rowid);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DEVICE_TYPE:
            GET_STRING (self->name);
            break;

        case COMMON_MSG_INSERT_DEVTYPE:
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_DELETE_DEVTYPE:
            GET_NUMBER4 (self->devicetype_id);
            break;

        case COMMON_MSG_RETURN_DEVTYPE:
            GET_NUMBER4 (self->rowid);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->msg = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->msg, zmsg_pop (msg));
            break;

        case COMMON_MSG_GET_CLIENT:
            GET_NUMBER4 (self->client_id);
            break;

        case COMMON_MSG_GET_CINFO:
            GET_NUMBER4 (self->cinfo_id);
            break;

        case COMMON_MSG_GET_DEVICE:
            GET_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_GET_DEVTYPE:
            GET_NUMBER4 (self->devicetype_id);
            break;

        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            GET_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            GET_NUMBER4 (self->device_id);
            GET_STRING (self->device_name);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->measurements = zlist_new ();
                zlist_autofree (self->measurements);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->measurements, string);
                    free (string);
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
        common_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode common_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
common_msg_encode (common_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    common_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            //  mt_id is a 2-byte integer
            frame_size += 2;
            break;
            
        case COMMON_MSG_GET_MEASURE_TYPE_S:
            //  mt_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mt_name)
                frame_size += strlen (self->mt_name);
            //  mt_unit is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mt_unit)
                frame_size += strlen (self->mt_unit);
            break;
            
        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            //  mt_id is a 2-byte integer
            frame_size += 2;
            //  mts_id is a 2-byte integer
            frame_size += 2;
            break;
            
        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            //  mt_id is a 2-byte integer
            frame_size += 2;
            //  mts_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mts_name)
                frame_size += strlen (self->mts_name);
            //  mts_scale is a 1-byte integer
            frame_size += 1;
            break;
            
        case COMMON_MSG_RETURN_MEASURE_TYPE:
            //  mt_id is a 2-byte integer
            frame_size += 2;
            //  mt_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mt_name)
                frame_size += strlen (self->mt_name);
            //  mt_unit is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mt_unit)
                frame_size += strlen (self->mt_unit);
            break;
            
        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            //  mts_id is a 2-byte integer
            frame_size += 2;
            //  mt_id is a 2-byte integer
            frame_size += 2;
            //  mts_scale is a 1-byte integer
            frame_size += 1;
            //  mts_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mts_name)
                frame_size += strlen (self->mts_name);
            break;
            
        case COMMON_MSG_FAIL:
            //  errtype is a 1-byte integer
            frame_size += 1;
            //  errorno is a 4-byte integer
            frame_size += 4;
            //  errmsg is a string with 4-byte length
            frame_size += 4;
            if (self->errmsg)
                frame_size += strlen (self->errmsg);
            //  aux is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->aux) {
                self->aux_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    self->aux_bytes += 1 + strlen ((const char *) zhash_cursor (self->aux));
                    self->aux_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            frame_size += self->aux_bytes;
            break;
            
        case COMMON_MSG_DB_OK:
            //  rowid is a 4-byte integer
            frame_size += 4;
            //  aux is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->aux) {
                self->aux_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    self->aux_bytes += 1 + strlen ((const char *) zhash_cursor (self->aux));
                    self->aux_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            frame_size += self->aux_bytes;
            break;
            
        case COMMON_MSG_CLIENT:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            break;
            
        case COMMON_MSG_INSERT_CLIENT:
            break;
            
        case COMMON_MSG_UPDATE_CLIENT:
            //  client_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_DELETE_CLIENT:
            //  client_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_RETURN_CLIENT:
            //  rowid is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_NEW_MEASUREMENT:
            //  client_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->client_name)
                frame_size += strlen (self->client_name);
            //  device_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->device_name)
                frame_size += strlen (self->device_name);
            //  device_type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->device_type)
                frame_size += strlen (self->device_type);
            //  mt_id is a 2-byte integer
            frame_size += 2;
            //  mts_id is a 2-byte integer
            frame_size += 2;
            //  value is a 8-byte integer
            frame_size += 8;
            break;
            
        case COMMON_MSG_CLIENT_INFO:
            //  client_id is a 4-byte integer
            frame_size += 4;
            //  device_id is a 4-byte integer
            frame_size += 4;
            //  info is a chunk with 4-byte length
            frame_size += 4;
            if (self->info)
                frame_size += zchunk_size (self->info);
            //  date is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_INSERT_CINFO:
            break;
            
        case COMMON_MSG_DELETE_CINFO:
            //  cinfo_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_RETURN_CINFO:
            //  rowid is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_DEVICE:
            //  devicetype_id is a 4-byte integer
            frame_size += 4;
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            break;
            
        case COMMON_MSG_INSERT_DEVICE:
            break;
            
        case COMMON_MSG_DELETE_DEVICE:
            //  device_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_RETURN_DEVICE:
            //  rowid is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_DEVICE_TYPE:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            break;
            
        case COMMON_MSG_INSERT_DEVTYPE:
            break;
            
        case COMMON_MSG_DELETE_DEVTYPE:
            //  devicetype_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_RETURN_DEVTYPE:
            //  rowid is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_GET_CLIENT:
            //  client_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_GET_CINFO:
            //  cinfo_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_GET_DEVICE:
            //  device_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_GET_DEVTYPE:
            //  devicetype_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            //  device_id is a 4-byte integer
            frame_size += 4;
            break;
            
        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            //  device_id is a 4-byte integer
            frame_size += 4;
            //  device_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->device_name)
                frame_size += strlen (self->device_name);
            //  measurements is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->measurements) {
                //  Add up size of list contents
                char *measurements = (char *) zlist_first (self->measurements);
                while (measurements) {
                    frame_size += 4 + strlen (measurements);
                    measurements = (char *) zlist_next (self->measurements);
                }
            }
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 9);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            PUT_NUMBER2 (self->mt_id);
            break;

        case COMMON_MSG_GET_MEASURE_TYPE_S:
            if (self->mt_name) {
                PUT_STRING (self->mt_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->mt_unit) {
                PUT_STRING (self->mt_unit);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            PUT_NUMBER2 (self->mt_id);
            PUT_NUMBER2 (self->mts_id);
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            PUT_NUMBER2 (self->mt_id);
            if (self->mts_name) {
                PUT_STRING (self->mts_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->mts_scale);
            break;

        case COMMON_MSG_RETURN_MEASURE_TYPE:
            PUT_NUMBER2 (self->mt_id);
            if (self->mt_name) {
                PUT_STRING (self->mt_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->mt_unit) {
                PUT_STRING (self->mt_unit);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            PUT_NUMBER2 (self->mts_id);
            PUT_NUMBER2 (self->mt_id);
            PUT_NUMBER1 (self->mts_scale);
            if (self->mts_name) {
                PUT_STRING (self->mts_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_FAIL:
            PUT_NUMBER1 (self->errtype);
            PUT_NUMBER4 (self->errorno);
            if (self->errmsg) {
                PUT_LONGSTR (self->errmsg);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            if (self->aux) {
                PUT_NUMBER4 (zhash_size (self->aux));
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->aux));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case COMMON_MSG_DB_OK:
            PUT_NUMBER4 (self->rowid);
            if (self->aux) {
                PUT_NUMBER4 (zhash_size (self->aux));
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->aux));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case COMMON_MSG_CLIENT:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_INSERT_CLIENT:
            break;

        case COMMON_MSG_UPDATE_CLIENT:
            PUT_NUMBER4 (self->client_id);
            break;

        case COMMON_MSG_DELETE_CLIENT:
            PUT_NUMBER4 (self->client_id);
            break;

        case COMMON_MSG_RETURN_CLIENT:
            PUT_NUMBER4 (self->rowid);
            break;

        case COMMON_MSG_NEW_MEASUREMENT:
            if (self->client_name) {
                PUT_STRING (self->client_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->device_name) {
                PUT_STRING (self->device_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->device_type) {
                PUT_STRING (self->device_type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER2 (self->mt_id);
            PUT_NUMBER2 (self->mts_id);
            PUT_NUMBER8 (self->value);
            break;

        case COMMON_MSG_CLIENT_INFO:
            PUT_NUMBER4 (self->client_id);
            PUT_NUMBER4 (self->device_id);
            if (self->info) {
                PUT_NUMBER4 (zchunk_size (self->info));
                memcpy (self->needle,
                        zchunk_data (self->info),
                        zchunk_size (self->info));
                self->needle += zchunk_size (self->info);
            }
            else
                PUT_NUMBER4 (0);    //  Empty chunk
            PUT_NUMBER4 (self->date);
            break;

        case COMMON_MSG_INSERT_CINFO:
            break;

        case COMMON_MSG_DELETE_CINFO:
            PUT_NUMBER4 (self->cinfo_id);
            break;

        case COMMON_MSG_RETURN_CINFO:
            PUT_NUMBER4 (self->rowid);
            break;

        case COMMON_MSG_DEVICE:
            PUT_NUMBER4 (self->devicetype_id);
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_INSERT_DEVICE:
            break;

        case COMMON_MSG_DELETE_DEVICE:
            PUT_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_RETURN_DEVICE:
            PUT_NUMBER4 (self->rowid);
            break;

        case COMMON_MSG_DEVICE_TYPE:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case COMMON_MSG_INSERT_DEVTYPE:
            break;

        case COMMON_MSG_DELETE_DEVTYPE:
            PUT_NUMBER4 (self->devicetype_id);
            break;

        case COMMON_MSG_RETURN_DEVTYPE:
            PUT_NUMBER4 (self->rowid);
            break;

        case COMMON_MSG_GET_CLIENT:
            PUT_NUMBER4 (self->client_id);
            break;

        case COMMON_MSG_GET_CINFO:
            PUT_NUMBER4 (self->cinfo_id);
            break;

        case COMMON_MSG_GET_DEVICE:
            PUT_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_GET_DEVTYPE:
            PUT_NUMBER4 (self->devicetype_id);
            break;

        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            PUT_NUMBER4 (self->device_id);
            break;

        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            PUT_NUMBER4 (self->device_id);
            if (self->device_name) {
                PUT_STRING (self->device_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->measurements) {
                PUT_NUMBER4 (zlist_size (self->measurements));
                char *measurements = (char *) zlist_first (self->measurements);
                while (measurements) {
                    PUT_LONGSTR (measurements);
                    measurements = (char *) zlist_next (self->measurements);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        common_msg_destroy (self_p);
        return NULL;
    }
    //  Now send the message field if there is any
    if (self->id == COMMON_MSG_INSERT_CLIENT) {
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
    if (self->id == COMMON_MSG_UPDATE_CLIENT) {
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
    if (self->id == COMMON_MSG_RETURN_CLIENT) {
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
    if (self->id == COMMON_MSG_INSERT_CINFO) {
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
    if (self->id == COMMON_MSG_RETURN_CINFO) {
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
    if (self->id == COMMON_MSG_INSERT_DEVICE) {
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
    if (self->id == COMMON_MSG_RETURN_DEVICE) {
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
    if (self->id == COMMON_MSG_INSERT_DEVTYPE) {
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
    if (self->id == COMMON_MSG_RETURN_DEVTYPE) {
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
    //  Destroy common_msg object
    common_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a common_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

common_msg_t *
common_msg_recv (void *input)
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
    common_msg_t *common_msg = common_msg_decode (&msg);
    if (common_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        common_msg->routing_id = routing_id;

    return common_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a common_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

common_msg_t *
common_msg_recv_nowait (void *input)
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
    common_msg_t *common_msg = common_msg_decode (&msg);
    if (common_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        common_msg->routing_id = routing_id;

    return common_msg;
}


//  --------------------------------------------------------------------------
//  Send the common_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
common_msg_send (common_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    common_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode common_msg message to a single zmsg
    zmsg_t *msg = common_msg_encode (self_p);
    
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
//  Send the common_msg to the output, and do not destroy it

int
common_msg_send_again (common_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = common_msg_dup (self);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode GET_MEASURE_TYPE_I message

zmsg_t * 
common_msg_encode_get_measure_type_i (
    uint16_t mt_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_I);
    common_msg_set_mt_id (self, mt_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_MEASURE_TYPE_S message

zmsg_t * 
common_msg_encode_get_measure_type_s (
    const char *mt_name,
    const char *mt_unit)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_S);
    common_msg_set_mt_name (self, "%s", mt_name);
    common_msg_set_mt_unit (self, "%s", mt_unit);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_MEASURE_SUBTYPE_I message

zmsg_t * 
common_msg_encode_get_measure_subtype_i (
    uint16_t mt_id,
    uint16_t mts_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_I);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_id (self, mts_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_MEASURE_SUBTYPE_S message

zmsg_t * 
common_msg_encode_get_measure_subtype_s (
    uint16_t mt_id,
    const char *mts_name,
    byte mts_scale)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_S);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_name (self, "%s", mts_name);
    common_msg_set_mts_scale (self, mts_scale);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_MEASURE_TYPE message

zmsg_t * 
common_msg_encode_return_measure_type (
    uint16_t mt_id,
    const char *mt_name,
    const char *mt_unit)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_MEASURE_TYPE);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mt_name (self, "%s", mt_name);
    common_msg_set_mt_unit (self, "%s", mt_unit);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_MEASURE_SUBTYPE message

zmsg_t * 
common_msg_encode_return_measure_subtype (
    uint16_t mts_id,
    uint16_t mt_id,
    byte mts_scale,
    const char *mts_name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_MEASURE_SUBTYPE);
    common_msg_set_mts_id (self, mts_id);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_scale (self, mts_scale);
    common_msg_set_mts_name (self, "%s", mts_name);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode FAIL message

zmsg_t * 
common_msg_encode_fail (
    byte errtype,
    uint32_t errorno,
    const char *errmsg,
    zhash_t *aux)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_FAIL);
    common_msg_set_errtype (self, errtype);
    common_msg_set_errorno (self, errorno);
    common_msg_set_errmsg (self, "%s", errmsg);
    zhash_t *aux_copy = zhash_dup (aux);
    common_msg_set_aux (self, &aux_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DB_OK message

zmsg_t * 
common_msg_encode_db_ok (
    uint32_t rowid,
    zhash_t *aux)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DB_OK);
    common_msg_set_rowid (self, rowid);
    zhash_t *aux_copy = zhash_dup (aux);
    common_msg_set_aux (self, &aux_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode CLIENT message

zmsg_t * 
common_msg_encode_client (
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_CLIENT);
    common_msg_set_name (self, "%s", name);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INSERT_CLIENT message

zmsg_t * 
common_msg_encode_insert_client (
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_CLIENT);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode UPDATE_CLIENT message

zmsg_t * 
common_msg_encode_update_client (
    uint32_t client_id,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_UPDATE_CLIENT);
    common_msg_set_client_id (self, client_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DELETE_CLIENT message

zmsg_t * 
common_msg_encode_delete_client (
    uint32_t client_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_CLIENT);
    common_msg_set_client_id (self, client_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_CLIENT message

zmsg_t * 
common_msg_encode_return_client (
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_CLIENT);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode NEW_MEASUREMENT message

zmsg_t * 
common_msg_encode_new_measurement (
    const char *client_name,
    const char *device_name,
    const char *device_type,
    uint16_t mt_id,
    uint16_t mts_id,
    uint64_t value)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_NEW_MEASUREMENT);
    common_msg_set_client_name (self, "%s", client_name);
    common_msg_set_device_name (self, "%s", device_name);
    common_msg_set_device_type (self, "%s", device_type);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_id (self, mts_id);
    common_msg_set_value (self, value);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode CLIENT_INFO message

zmsg_t * 
common_msg_encode_client_info (
    uint32_t client_id,
    uint32_t device_id,
    zchunk_t *info,
    uint32_t date)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_CLIENT_INFO);
    common_msg_set_client_id (self, client_id);
    common_msg_set_device_id (self, device_id);
    zchunk_t *info_copy = zchunk_dup (info);
    common_msg_set_info (self, &info_copy);
    common_msg_set_date (self, date);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INSERT_CINFO message

zmsg_t * 
common_msg_encode_insert_cinfo (
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_CINFO);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DELETE_CINFO message

zmsg_t * 
common_msg_encode_delete_cinfo (
    uint32_t cinfo_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_CINFO);
    common_msg_set_cinfo_id (self, cinfo_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_CINFO message

zmsg_t * 
common_msg_encode_return_cinfo (
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_CINFO);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DEVICE message

zmsg_t * 
common_msg_encode_device (
    uint32_t devicetype_id,
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DEVICE);
    common_msg_set_devicetype_id (self, devicetype_id);
    common_msg_set_name (self, "%s", name);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INSERT_DEVICE message

zmsg_t * 
common_msg_encode_insert_device (
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_DEVICE);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DELETE_DEVICE message

zmsg_t * 
common_msg_encode_delete_device (
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_DEVICE);
    common_msg_set_device_id (self, device_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_DEVICE message

zmsg_t * 
common_msg_encode_return_device (
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_DEVICE);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DEVICE_TYPE message

zmsg_t * 
common_msg_encode_device_type (
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    common_msg_set_name (self, "%s", name);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INSERT_DEVTYPE message

zmsg_t * 
common_msg_encode_insert_devtype (
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_DEVTYPE);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DELETE_DEVTYPE message

zmsg_t * 
common_msg_encode_delete_devtype (
    uint32_t devicetype_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_DEVTYPE);
    common_msg_set_devicetype_id (self, devicetype_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_DEVTYPE message

zmsg_t * 
common_msg_encode_return_devtype (
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_CLIENT message

zmsg_t * 
common_msg_encode_get_client (
    uint32_t client_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_CLIENT);
    common_msg_set_client_id (self, client_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_CINFO message

zmsg_t * 
common_msg_encode_get_cinfo (
    uint32_t cinfo_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_CINFO);
    common_msg_set_cinfo_id (self, cinfo_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_DEVICE message

zmsg_t * 
common_msg_encode_get_device (
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_DEVICE);
    common_msg_set_device_id (self, device_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_DEVTYPE message

zmsg_t * 
common_msg_encode_get_devtype (
    uint32_t devicetype_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_DEVTYPE);
    common_msg_set_devicetype_id (self, devicetype_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_LAST_MEASUREMENTS message

zmsg_t * 
common_msg_encode_get_last_measurements (
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_LAST_MEASUREMENTS);
    common_msg_set_device_id (self, device_id);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_LAST_MEASUREMENTS message

zmsg_t * 
common_msg_encode_return_last_measurements (
    uint32_t device_id,
    const char *device_name,
    zlist_t *measurements)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_LAST_MEASUREMENTS);
    common_msg_set_device_id (self, device_id);
    common_msg_set_device_name (self, "%s", device_name);
    zlist_t *measurements_copy = zlist_dup (measurements);
    common_msg_set_measurements (self, &measurements_copy);
    return common_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the GET_MEASURE_TYPE_I to the socket in one step

int
common_msg_send_get_measure_type_i (
    void *output,
    uint16_t mt_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_I);
    common_msg_set_mt_id (self, mt_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_MEASURE_TYPE_S to the socket in one step

int
common_msg_send_get_measure_type_s (
    void *output,
    const char *mt_name,
    const char *mt_unit)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_S);
    common_msg_set_mt_name (self, mt_name);
    common_msg_set_mt_unit (self, mt_unit);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_MEASURE_SUBTYPE_I to the socket in one step

int
common_msg_send_get_measure_subtype_i (
    void *output,
    uint16_t mt_id,
    uint16_t mts_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_I);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_id (self, mts_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_MEASURE_SUBTYPE_S to the socket in one step

int
common_msg_send_get_measure_subtype_s (
    void *output,
    uint16_t mt_id,
    const char *mts_name,
    byte mts_scale)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_S);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_name (self, mts_name);
    common_msg_set_mts_scale (self, mts_scale);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_MEASURE_TYPE to the socket in one step

int
common_msg_send_return_measure_type (
    void *output,
    uint16_t mt_id,
    const char *mt_name,
    const char *mt_unit)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_MEASURE_TYPE);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mt_name (self, mt_name);
    common_msg_set_mt_unit (self, mt_unit);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_MEASURE_SUBTYPE to the socket in one step

int
common_msg_send_return_measure_subtype (
    void *output,
    uint16_t mts_id,
    uint16_t mt_id,
    byte mts_scale,
    const char *mts_name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_MEASURE_SUBTYPE);
    common_msg_set_mts_id (self, mts_id);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_scale (self, mts_scale);
    common_msg_set_mts_name (self, mts_name);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the FAIL to the socket in one step

int
common_msg_send_fail (
    void *output,
    byte errtype,
    uint32_t errorno,
    const char *errmsg,
    zhash_t *aux)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_FAIL);
    common_msg_set_errtype (self, errtype);
    common_msg_set_errorno (self, errorno);
    common_msg_set_errmsg (self, errmsg);
    zhash_t *aux_copy = zhash_dup (aux);
    common_msg_set_aux (self, &aux_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DB_OK to the socket in one step

int
common_msg_send_db_ok (
    void *output,
    uint32_t rowid,
    zhash_t *aux)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DB_OK);
    common_msg_set_rowid (self, rowid);
    zhash_t *aux_copy = zhash_dup (aux);
    common_msg_set_aux (self, &aux_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CLIENT to the socket in one step

int
common_msg_send_client (
    void *output,
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_CLIENT);
    common_msg_set_name (self, name);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INSERT_CLIENT to the socket in one step

int
common_msg_send_insert_client (
    void *output,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_CLIENT);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the UPDATE_CLIENT to the socket in one step

int
common_msg_send_update_client (
    void *output,
    uint32_t client_id,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_UPDATE_CLIENT);
    common_msg_set_client_id (self, client_id);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELETE_CLIENT to the socket in one step

int
common_msg_send_delete_client (
    void *output,
    uint32_t client_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_CLIENT);
    common_msg_set_client_id (self, client_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_CLIENT to the socket in one step

int
common_msg_send_return_client (
    void *output,
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_CLIENT);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the NEW_MEASUREMENT to the socket in one step

int
common_msg_send_new_measurement (
    void *output,
    const char *client_name,
    const char *device_name,
    const char *device_type,
    uint16_t mt_id,
    uint16_t mts_id,
    uint64_t value)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_NEW_MEASUREMENT);
    common_msg_set_client_name (self, client_name);
    common_msg_set_device_name (self, device_name);
    common_msg_set_device_type (self, device_type);
    common_msg_set_mt_id (self, mt_id);
    common_msg_set_mts_id (self, mts_id);
    common_msg_set_value (self, value);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the CLIENT_INFO to the socket in one step

int
common_msg_send_client_info (
    void *output,
    uint32_t client_id,
    uint32_t device_id,
    zchunk_t *info,
    uint32_t date)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_CLIENT_INFO);
    common_msg_set_client_id (self, client_id);
    common_msg_set_device_id (self, device_id);
    zchunk_t *info_copy = zchunk_dup (info);
    common_msg_set_info (self, &info_copy);
    common_msg_set_date (self, date);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INSERT_CINFO to the socket in one step

int
common_msg_send_insert_cinfo (
    void *output,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_CINFO);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELETE_CINFO to the socket in one step

int
common_msg_send_delete_cinfo (
    void *output,
    uint32_t cinfo_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_CINFO);
    common_msg_set_cinfo_id (self, cinfo_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_CINFO to the socket in one step

int
common_msg_send_return_cinfo (
    void *output,
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_CINFO);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DEVICE to the socket in one step

int
common_msg_send_device (
    void *output,
    uint32_t devicetype_id,
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DEVICE);
    common_msg_set_devicetype_id (self, devicetype_id);
    common_msg_set_name (self, name);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INSERT_DEVICE to the socket in one step

int
common_msg_send_insert_device (
    void *output,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_DEVICE);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELETE_DEVICE to the socket in one step

int
common_msg_send_delete_device (
    void *output,
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_DEVICE);
    common_msg_set_device_id (self, device_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_DEVICE to the socket in one step

int
common_msg_send_return_device (
    void *output,
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_DEVICE);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DEVICE_TYPE to the socket in one step

int
common_msg_send_device_type (
    void *output,
    const char *name)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    common_msg_set_name (self, name);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INSERT_DEVTYPE to the socket in one step

int
common_msg_send_insert_devtype (
    void *output,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_INSERT_DEVTYPE);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DELETE_DEVTYPE to the socket in one step

int
common_msg_send_delete_devtype (
    void *output,
    uint32_t devicetype_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_DELETE_DEVTYPE);
    common_msg_set_devicetype_id (self, devicetype_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_DEVTYPE to the socket in one step

int
common_msg_send_return_devtype (
    void *output,
    uint32_t rowid,
    zmsg_t *msg)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    common_msg_set_rowid (self, rowid);
    zmsg_t *msg_copy = zmsg_dup (msg);
    common_msg_set_msg (self, &msg_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_CLIENT to the socket in one step

int
common_msg_send_get_client (
    void *output,
    uint32_t client_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_CLIENT);
    common_msg_set_client_id (self, client_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_CINFO to the socket in one step

int
common_msg_send_get_cinfo (
    void *output,
    uint32_t cinfo_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_CINFO);
    common_msg_set_cinfo_id (self, cinfo_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_DEVICE to the socket in one step

int
common_msg_send_get_device (
    void *output,
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_DEVICE);
    common_msg_set_device_id (self, device_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_DEVTYPE to the socket in one step

int
common_msg_send_get_devtype (
    void *output,
    uint32_t devicetype_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_DEVTYPE);
    common_msg_set_devicetype_id (self, devicetype_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_LAST_MEASUREMENTS to the socket in one step

int
common_msg_send_get_last_measurements (
    void *output,
    uint32_t device_id)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_GET_LAST_MEASUREMENTS);
    common_msg_set_device_id (self, device_id);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_LAST_MEASUREMENTS to the socket in one step

int
common_msg_send_return_last_measurements (
    void *output,
    uint32_t device_id,
    const char *device_name,
    zlist_t *measurements)
{
    common_msg_t *self = common_msg_new (COMMON_MSG_RETURN_LAST_MEASUREMENTS);
    common_msg_set_device_id (self, device_id);
    common_msg_set_device_name (self, device_name);
    zlist_t *measurements_copy = zlist_dup (measurements);
    common_msg_set_measurements (self, &measurements_copy);
    return common_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the common_msg message

common_msg_t *
common_msg_dup (common_msg_t *self)
{
    if (!self)
        return NULL;
        
    common_msg_t *copy = common_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            copy->mt_id = self->mt_id;
            break;

        case COMMON_MSG_GET_MEASURE_TYPE_S:
            copy->mt_name = self->mt_name? strdup (self->mt_name): NULL;
            copy->mt_unit = self->mt_unit? strdup (self->mt_unit): NULL;
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            copy->mt_id = self->mt_id;
            copy->mts_id = self->mts_id;
            break;

        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            copy->mt_id = self->mt_id;
            copy->mts_name = self->mts_name? strdup (self->mts_name): NULL;
            copy->mts_scale = self->mts_scale;
            break;

        case COMMON_MSG_RETURN_MEASURE_TYPE:
            copy->mt_id = self->mt_id;
            copy->mt_name = self->mt_name? strdup (self->mt_name): NULL;
            copy->mt_unit = self->mt_unit? strdup (self->mt_unit): NULL;
            break;

        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            copy->mts_id = self->mts_id;
            copy->mt_id = self->mt_id;
            copy->mts_scale = self->mts_scale;
            copy->mts_name = self->mts_name? strdup (self->mts_name): NULL;
            break;

        case COMMON_MSG_FAIL:
            copy->errtype = self->errtype;
            copy->errorno = self->errorno;
            copy->errmsg = self->errmsg? strdup (self->errmsg): NULL;
            copy->aux = self->aux? zhash_dup (self->aux): NULL;
            break;

        case COMMON_MSG_DB_OK:
            copy->rowid = self->rowid;
            copy->aux = self->aux? zhash_dup (self->aux): NULL;
            break;

        case COMMON_MSG_CLIENT:
            copy->name = self->name? strdup (self->name): NULL;
            break;

        case COMMON_MSG_INSERT_CLIENT:
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_UPDATE_CLIENT:
            copy->client_id = self->client_id;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DELETE_CLIENT:
            copy->client_id = self->client_id;
            break;

        case COMMON_MSG_RETURN_CLIENT:
            copy->rowid = self->rowid;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_NEW_MEASUREMENT:
            copy->client_name = self->client_name? strdup (self->client_name): NULL;
            copy->device_name = self->device_name? strdup (self->device_name): NULL;
            copy->device_type = self->device_type? strdup (self->device_type): NULL;
            copy->mt_id = self->mt_id;
            copy->mts_id = self->mts_id;
            copy->value = self->value;
            break;

        case COMMON_MSG_CLIENT_INFO:
            copy->client_id = self->client_id;
            copy->device_id = self->device_id;
            copy->info = self->info? zchunk_dup (self->info): NULL;
            copy->date = self->date;
            break;

        case COMMON_MSG_INSERT_CINFO:
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DELETE_CINFO:
            copy->cinfo_id = self->cinfo_id;
            break;

        case COMMON_MSG_RETURN_CINFO:
            copy->rowid = self->rowid;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DEVICE:
            copy->devicetype_id = self->devicetype_id;
            copy->name = self->name? strdup (self->name): NULL;
            break;

        case COMMON_MSG_INSERT_DEVICE:
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DELETE_DEVICE:
            copy->device_id = self->device_id;
            break;

        case COMMON_MSG_RETURN_DEVICE:
            copy->rowid = self->rowid;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DEVICE_TYPE:
            copy->name = self->name? strdup (self->name): NULL;
            break;

        case COMMON_MSG_INSERT_DEVTYPE:
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_DELETE_DEVTYPE:
            copy->devicetype_id = self->devicetype_id;
            break;

        case COMMON_MSG_RETURN_DEVTYPE:
            copy->rowid = self->rowid;
            copy->msg = self->msg? zmsg_dup (self->msg): NULL;
            break;

        case COMMON_MSG_GET_CLIENT:
            copy->client_id = self->client_id;
            break;

        case COMMON_MSG_GET_CINFO:
            copy->cinfo_id = self->cinfo_id;
            break;

        case COMMON_MSG_GET_DEVICE:
            copy->device_id = self->device_id;
            break;

        case COMMON_MSG_GET_DEVTYPE:
            copy->devicetype_id = self->devicetype_id;
            break;

        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            copy->device_id = self->device_id;
            break;

        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            copy->device_id = self->device_id;
            copy->device_name = self->device_name? strdup (self->device_name): NULL;
            copy->measurements = self->measurements? zlist_dup (self->measurements): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
common_msg_print (common_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            zsys_debug ("COMMON_MSG_GET_MEASURE_TYPE_I:");
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            break;
            
        case COMMON_MSG_GET_MEASURE_TYPE_S:
            zsys_debug ("COMMON_MSG_GET_MEASURE_TYPE_S:");
            if (self->mt_name)
                zsys_debug ("    mt_name='%s'", self->mt_name);
            else
                zsys_debug ("    mt_name=");
            if (self->mt_unit)
                zsys_debug ("    mt_unit='%s'", self->mt_unit);
            else
                zsys_debug ("    mt_unit=");
            break;
            
        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            zsys_debug ("COMMON_MSG_GET_MEASURE_SUBTYPE_I:");
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            zsys_debug ("    mts_id=%ld", (long) self->mts_id);
            break;
            
        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            zsys_debug ("COMMON_MSG_GET_MEASURE_SUBTYPE_S:");
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            if (self->mts_name)
                zsys_debug ("    mts_name='%s'", self->mts_name);
            else
                zsys_debug ("    mts_name=");
            zsys_debug ("    mts_scale=%ld", (long) self->mts_scale);
            break;
            
        case COMMON_MSG_RETURN_MEASURE_TYPE:
            zsys_debug ("COMMON_MSG_RETURN_MEASURE_TYPE:");
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            if (self->mt_name)
                zsys_debug ("    mt_name='%s'", self->mt_name);
            else
                zsys_debug ("    mt_name=");
            if (self->mt_unit)
                zsys_debug ("    mt_unit='%s'", self->mt_unit);
            else
                zsys_debug ("    mt_unit=");
            break;
            
        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            zsys_debug ("COMMON_MSG_RETURN_MEASURE_SUBTYPE:");
            zsys_debug ("    mts_id=%ld", (long) self->mts_id);
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            zsys_debug ("    mts_scale=%ld", (long) self->mts_scale);
            if (self->mts_name)
                zsys_debug ("    mts_name='%s'", self->mts_name);
            else
                zsys_debug ("    mts_name=");
            break;
            
        case COMMON_MSG_FAIL:
            zsys_debug ("COMMON_MSG_FAIL:");
            zsys_debug ("    errtype=%ld", (long) self->errtype);
            zsys_debug ("    errorno=%ld", (long) self->errorno);
            if (self->errmsg)
                zsys_debug ("    errmsg='%s'", self->errmsg);
            else
                zsys_debug ("    errmsg=");
            zsys_debug ("    aux=");
            if (self->aux) {
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->aux), item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DB_OK:
            zsys_debug ("COMMON_MSG_DB_OK:");
            zsys_debug ("    rowid=%ld", (long) self->rowid);
            zsys_debug ("    aux=");
            if (self->aux) {
                char *item = (char *) zhash_first (self->aux);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->aux), item);
                    item = (char *) zhash_next (self->aux);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_CLIENT:
            zsys_debug ("COMMON_MSG_CLIENT:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            break;
            
        case COMMON_MSG_INSERT_CLIENT:
            zsys_debug ("COMMON_MSG_INSERT_CLIENT:");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_UPDATE_CLIENT:
            zsys_debug ("COMMON_MSG_UPDATE_CLIENT:");
            zsys_debug ("    client_id=%ld", (long) self->client_id);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DELETE_CLIENT:
            zsys_debug ("COMMON_MSG_DELETE_CLIENT:");
            zsys_debug ("    client_id=%ld", (long) self->client_id);
            break;
            
        case COMMON_MSG_RETURN_CLIENT:
            zsys_debug ("COMMON_MSG_RETURN_CLIENT:");
            zsys_debug ("    rowid=%ld", (long) self->rowid);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_NEW_MEASUREMENT:
            zsys_debug ("COMMON_MSG_NEW_MEASUREMENT:");
            if (self->client_name)
                zsys_debug ("    client_name='%s'", self->client_name);
            else
                zsys_debug ("    client_name=");
            if (self->device_name)
                zsys_debug ("    device_name='%s'", self->device_name);
            else
                zsys_debug ("    device_name=");
            if (self->device_type)
                zsys_debug ("    device_type='%s'", self->device_type);
            else
                zsys_debug ("    device_type=");
            zsys_debug ("    mt_id=%ld", (long) self->mt_id);
            zsys_debug ("    mts_id=%ld", (long) self->mts_id);
            zsys_debug ("    value=%ld", (long) self->value);
            break;
            
        case COMMON_MSG_CLIENT_INFO:
            zsys_debug ("COMMON_MSG_CLIENT_INFO:");
            zsys_debug ("    client_id=%ld", (long) self->client_id);
            zsys_debug ("    device_id=%ld", (long) self->device_id);
            zsys_debug ("    info=[ ... ]");
            zsys_debug ("    date=%ld", (long) self->date);
            break;
            
        case COMMON_MSG_INSERT_CINFO:
            zsys_debug ("COMMON_MSG_INSERT_CINFO:");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DELETE_CINFO:
            zsys_debug ("COMMON_MSG_DELETE_CINFO:");
            zsys_debug ("    cinfo_id=%ld", (long) self->cinfo_id);
            break;
            
        case COMMON_MSG_RETURN_CINFO:
            zsys_debug ("COMMON_MSG_RETURN_CINFO:");
            zsys_debug ("    rowid=%ld", (long) self->rowid);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DEVICE:
            zsys_debug ("COMMON_MSG_DEVICE:");
            zsys_debug ("    devicetype_id=%ld", (long) self->devicetype_id);
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            break;
            
        case COMMON_MSG_INSERT_DEVICE:
            zsys_debug ("COMMON_MSG_INSERT_DEVICE:");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DELETE_DEVICE:
            zsys_debug ("COMMON_MSG_DELETE_DEVICE:");
            zsys_debug ("    device_id=%ld", (long) self->device_id);
            break;
            
        case COMMON_MSG_RETURN_DEVICE:
            zsys_debug ("COMMON_MSG_RETURN_DEVICE:");
            zsys_debug ("    rowid=%ld", (long) self->rowid);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DEVICE_TYPE:
            zsys_debug ("COMMON_MSG_DEVICE_TYPE:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            break;
            
        case COMMON_MSG_INSERT_DEVTYPE:
            zsys_debug ("COMMON_MSG_INSERT_DEVTYPE:");
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_DELETE_DEVTYPE:
            zsys_debug ("COMMON_MSG_DELETE_DEVTYPE:");
            zsys_debug ("    devicetype_id=%ld", (long) self->devicetype_id);
            break;
            
        case COMMON_MSG_RETURN_DEVTYPE:
            zsys_debug ("COMMON_MSG_RETURN_DEVTYPE:");
            zsys_debug ("    rowid=%ld", (long) self->rowid);
            zsys_debug ("    msg=");
            if (self->msg)
                zmsg_print (self->msg);
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMMON_MSG_GET_CLIENT:
            zsys_debug ("COMMON_MSG_GET_CLIENT:");
            zsys_debug ("    client_id=%ld", (long) self->client_id);
            break;
            
        case COMMON_MSG_GET_CINFO:
            zsys_debug ("COMMON_MSG_GET_CINFO:");
            zsys_debug ("    cinfo_id=%ld", (long) self->cinfo_id);
            break;
            
        case COMMON_MSG_GET_DEVICE:
            zsys_debug ("COMMON_MSG_GET_DEVICE:");
            zsys_debug ("    device_id=%ld", (long) self->device_id);
            break;
            
        case COMMON_MSG_GET_DEVTYPE:
            zsys_debug ("COMMON_MSG_GET_DEVTYPE:");
            zsys_debug ("    devicetype_id=%ld", (long) self->devicetype_id);
            break;
            
        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            zsys_debug ("COMMON_MSG_GET_LAST_MEASUREMENTS:");
            zsys_debug ("    device_id=%ld", (long) self->device_id);
            break;
            
        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            zsys_debug ("COMMON_MSG_RETURN_LAST_MEASUREMENTS:");
            zsys_debug ("    device_id=%ld", (long) self->device_id);
            if (self->device_name)
                zsys_debug ("    device_name='%s'", self->device_name);
            else
                zsys_debug ("    device_name=");
            zsys_debug ("    measurements=");
            if (self->measurements) {
                char *measurements = (char *) zlist_first (self->measurements);
                while (measurements) {
                    zsys_debug ("        '%s'", measurements);
                    measurements = (char *) zlist_next (self->measurements);
                }
            }
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
common_msg_routing_id (common_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
common_msg_set_routing_id (common_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the common_msg id

int
common_msg_id (common_msg_t *self)
{
    assert (self);
    return self->id;
}

void
common_msg_set_id (common_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
common_msg_command (common_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case COMMON_MSG_GET_MEASURE_TYPE_I:
            return ("GET_MEASURE_TYPE_I");
            break;
        case COMMON_MSG_GET_MEASURE_TYPE_S:
            return ("GET_MEASURE_TYPE_S");
            break;
        case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
            return ("GET_MEASURE_SUBTYPE_I");
            break;
        case COMMON_MSG_GET_MEASURE_SUBTYPE_S:
            return ("GET_MEASURE_SUBTYPE_S");
            break;
        case COMMON_MSG_RETURN_MEASURE_TYPE:
            return ("RETURN_MEASURE_TYPE");
            break;
        case COMMON_MSG_RETURN_MEASURE_SUBTYPE:
            return ("RETURN_MEASURE_SUBTYPE");
            break;
        case COMMON_MSG_FAIL:
            return ("FAIL");
            break;
        case COMMON_MSG_DB_OK:
            return ("DB_OK");
            break;
        case COMMON_MSG_CLIENT:
            return ("CLIENT");
            break;
        case COMMON_MSG_INSERT_CLIENT:
            return ("INSERT_CLIENT");
            break;
        case COMMON_MSG_UPDATE_CLIENT:
            return ("UPDATE_CLIENT");
            break;
        case COMMON_MSG_DELETE_CLIENT:
            return ("DELETE_CLIENT");
            break;
        case COMMON_MSG_RETURN_CLIENT:
            return ("RETURN_CLIENT");
            break;
        case COMMON_MSG_NEW_MEASUREMENT:
            return ("NEW_MEASUREMENT");
            break;
        case COMMON_MSG_CLIENT_INFO:
            return ("CLIENT_INFO");
            break;
        case COMMON_MSG_INSERT_CINFO:
            return ("INSERT_CINFO");
            break;
        case COMMON_MSG_DELETE_CINFO:
            return ("DELETE_CINFO");
            break;
        case COMMON_MSG_RETURN_CINFO:
            return ("RETURN_CINFO");
            break;
        case COMMON_MSG_DEVICE:
            return ("DEVICE");
            break;
        case COMMON_MSG_INSERT_DEVICE:
            return ("INSERT_DEVICE");
            break;
        case COMMON_MSG_DELETE_DEVICE:
            return ("DELETE_DEVICE");
            break;
        case COMMON_MSG_RETURN_DEVICE:
            return ("RETURN_DEVICE");
            break;
        case COMMON_MSG_DEVICE_TYPE:
            return ("DEVICE_TYPE");
            break;
        case COMMON_MSG_INSERT_DEVTYPE:
            return ("INSERT_DEVTYPE");
            break;
        case COMMON_MSG_DELETE_DEVTYPE:
            return ("DELETE_DEVTYPE");
            break;
        case COMMON_MSG_RETURN_DEVTYPE:
            return ("RETURN_DEVTYPE");
            break;
        case COMMON_MSG_GET_CLIENT:
            return ("GET_CLIENT");
            break;
        case COMMON_MSG_GET_CINFO:
            return ("GET_CINFO");
            break;
        case COMMON_MSG_GET_DEVICE:
            return ("GET_DEVICE");
            break;
        case COMMON_MSG_GET_DEVTYPE:
            return ("GET_DEVTYPE");
            break;
        case COMMON_MSG_GET_LAST_MEASUREMENTS:
            return ("GET_LAST_MEASUREMENTS");
            break;
        case COMMON_MSG_RETURN_LAST_MEASUREMENTS:
            return ("RETURN_LAST_MEASUREMENTS");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the mt_id field

uint16_t
common_msg_mt_id (common_msg_t *self)
{
    assert (self);
    return self->mt_id;
}

void
common_msg_set_mt_id (common_msg_t *self, uint16_t mt_id)
{
    assert (self);
    self->mt_id = mt_id;
}


//  --------------------------------------------------------------------------
//  Get/set the mt_name field

const char *
common_msg_mt_name (common_msg_t *self)
{
    assert (self);
    return self->mt_name;
}

void
common_msg_set_mt_name (common_msg_t *self, const char *format, ...)
{
    //  Format mt_name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->mt_name);
    self->mt_name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the mt_unit field

const char *
common_msg_mt_unit (common_msg_t *self)
{
    assert (self);
    return self->mt_unit;
}

void
common_msg_set_mt_unit (common_msg_t *self, const char *format, ...)
{
    //  Format mt_unit from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->mt_unit);
    self->mt_unit = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the mts_id field

uint16_t
common_msg_mts_id (common_msg_t *self)
{
    assert (self);
    return self->mts_id;
}

void
common_msg_set_mts_id (common_msg_t *self, uint16_t mts_id)
{
    assert (self);
    self->mts_id = mts_id;
}


//  --------------------------------------------------------------------------
//  Get/set the mts_name field

const char *
common_msg_mts_name (common_msg_t *self)
{
    assert (self);
    return self->mts_name;
}

void
common_msg_set_mts_name (common_msg_t *self, const char *format, ...)
{
    //  Format mts_name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->mts_name);
    self->mts_name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the mts_scale field

byte
common_msg_mts_scale (common_msg_t *self)
{
    assert (self);
    return self->mts_scale;
}

void
common_msg_set_mts_scale (common_msg_t *self, byte mts_scale)
{
    assert (self);
    self->mts_scale = mts_scale;
}


//  --------------------------------------------------------------------------
//  Get/set the errtype field

byte
common_msg_errtype (common_msg_t *self)
{
    assert (self);
    return self->errtype;
}

void
common_msg_set_errtype (common_msg_t *self, byte errtype)
{
    assert (self);
    self->errtype = errtype;
}


//  --------------------------------------------------------------------------
//  Get/set the errorno field

uint32_t
common_msg_errorno (common_msg_t *self)
{
    assert (self);
    return self->errorno;
}

void
common_msg_set_errorno (common_msg_t *self, uint32_t errorno)
{
    assert (self);
    self->errorno = errorno;
}


//  --------------------------------------------------------------------------
//  Get/set the errmsg field

const char *
common_msg_errmsg (common_msg_t *self)
{
    assert (self);
    return self->errmsg;
}

void
common_msg_set_errmsg (common_msg_t *self, const char *format, ...)
{
    //  Format errmsg from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->errmsg);
    self->errmsg = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the aux field without transferring ownership

zhash_t *
common_msg_aux (common_msg_t *self)
{
    assert (self);
    return self->aux;
}

//  Get the aux field and transfer ownership to caller

zhash_t *
common_msg_get_aux (common_msg_t *self)
{
    zhash_t *aux = self->aux;
    self->aux = NULL;
    return aux;
}

//  Set the aux field, transferring ownership from caller

void
common_msg_set_aux (common_msg_t *self, zhash_t **aux_p)
{
    assert (self);
    assert (aux_p);
    zhash_destroy (&self->aux);
    self->aux = *aux_p;
    *aux_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the aux dictionary

const char *
common_msg_aux_string (common_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->aux)
        value = (const char *) (zhash_lookup (self->aux, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
common_msg_aux_number (common_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->aux)
        string = (char *) (zhash_lookup (self->aux, key));
    if (string)
        value = atol (string);

    return value;
}

void
common_msg_aux_insert (common_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->aux) {
        self->aux = zhash_new ();
        zhash_autofree (self->aux);
    }
    zhash_update (self->aux, key, string);
    free (string);
}

size_t
common_msg_aux_size (common_msg_t *self)
{
    return zhash_size (self->aux);
}


//  --------------------------------------------------------------------------
//  Get/set the rowid field

uint32_t
common_msg_rowid (common_msg_t *self)
{
    assert (self);
    return self->rowid;
}

void
common_msg_set_rowid (common_msg_t *self, uint32_t rowid)
{
    assert (self);
    self->rowid = rowid;
}


//  --------------------------------------------------------------------------
//  Get/set the name field

const char *
common_msg_name (common_msg_t *self)
{
    assert (self);
    return self->name;
}

void
common_msg_set_name (common_msg_t *self, const char *format, ...)
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
//  Get the msg field without transferring ownership

zmsg_t *
common_msg_msg (common_msg_t *self)
{
    assert (self);
    return self->msg;
}

//  Get the msg field and transfer ownership to caller

zmsg_t *
common_msg_get_msg (common_msg_t *self)
{
    zmsg_t *msg = self->msg;
    self->msg = NULL;
    return msg;
}

//  Set the msg field, transferring ownership from caller

void
common_msg_set_msg (common_msg_t *self, zmsg_t **msg_p)
{
    assert (self);
    assert (msg_p);
    zmsg_destroy (&self->msg);
    self->msg = *msg_p;
    *msg_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the client_id field

uint32_t
common_msg_client_id (common_msg_t *self)
{
    assert (self);
    return self->client_id;
}

void
common_msg_set_client_id (common_msg_t *self, uint32_t client_id)
{
    assert (self);
    self->client_id = client_id;
}


//  --------------------------------------------------------------------------
//  Get/set the client_name field

const char *
common_msg_client_name (common_msg_t *self)
{
    assert (self);
    return self->client_name;
}

void
common_msg_set_client_name (common_msg_t *self, const char *format, ...)
{
    //  Format client_name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->client_name);
    self->client_name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the device_name field

const char *
common_msg_device_name (common_msg_t *self)
{
    assert (self);
    return self->device_name;
}

void
common_msg_set_device_name (common_msg_t *self, const char *format, ...)
{
    //  Format device_name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->device_name);
    self->device_name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the device_type field

const char *
common_msg_device_type (common_msg_t *self)
{
    assert (self);
    return self->device_type;
}

void
common_msg_set_device_type (common_msg_t *self, const char *format, ...)
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
//  Get/set the value field

uint64_t
common_msg_value (common_msg_t *self)
{
    assert (self);
    return self->value;
}

void
common_msg_set_value (common_msg_t *self, uint64_t value)
{
    assert (self);
    self->value = value;
}


//  --------------------------------------------------------------------------
//  Get/set the device_id field

uint32_t
common_msg_device_id (common_msg_t *self)
{
    assert (self);
    return self->device_id;
}

void
common_msg_set_device_id (common_msg_t *self, uint32_t device_id)
{
    assert (self);
    self->device_id = device_id;
}


//  --------------------------------------------------------------------------
//  Get the info field without transferring ownership

zchunk_t *
common_msg_info (common_msg_t *self)
{
    assert (self);
    return self->info;
}

//  Get the info field and transfer ownership to caller

zchunk_t *
common_msg_get_info (common_msg_t *self)
{
    zchunk_t *info = self->info;
    self->info = NULL;
    return info;
}

//  Set the info field, transferring ownership from caller

void
common_msg_set_info (common_msg_t *self, zchunk_t **chunk_p)
{
    assert (self);
    assert (chunk_p);
    zchunk_destroy (&self->info);
    self->info = *chunk_p;
    *chunk_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the date field

uint32_t
common_msg_date (common_msg_t *self)
{
    assert (self);
    return self->date;
}

void
common_msg_set_date (common_msg_t *self, uint32_t date)
{
    assert (self);
    self->date = date;
}


//  --------------------------------------------------------------------------
//  Get/set the cinfo_id field

uint32_t
common_msg_cinfo_id (common_msg_t *self)
{
    assert (self);
    return self->cinfo_id;
}

void
common_msg_set_cinfo_id (common_msg_t *self, uint32_t cinfo_id)
{
    assert (self);
    self->cinfo_id = cinfo_id;
}


//  --------------------------------------------------------------------------
//  Get/set the devicetype_id field

uint32_t
common_msg_devicetype_id (common_msg_t *self)
{
    assert (self);
    return self->devicetype_id;
}

void
common_msg_set_devicetype_id (common_msg_t *self, uint32_t devicetype_id)
{
    assert (self);
    self->devicetype_id = devicetype_id;
}


//  --------------------------------------------------------------------------
//  Get the measurements field, without transferring ownership

zlist_t *
common_msg_measurements (common_msg_t *self)
{
    assert (self);
    return self->measurements;
}

//  Get the measurements field and transfer ownership to caller

zlist_t *
common_msg_get_measurements (common_msg_t *self)
{
    assert (self);
    zlist_t *measurements = self->measurements;
    self->measurements = NULL;
    return measurements;
}

//  Set the measurements field, transferring ownership from caller

void
common_msg_set_measurements (common_msg_t *self, zlist_t **measurements_p)
{
    assert (self);
    assert (measurements_p);
    zlist_destroy (&self->measurements);
    self->measurements = *measurements_p;
    *measurements_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the measurements field, and append a measurements value

const char *
common_msg_measurements_first (common_msg_t *self)
{
    assert (self);
    if (self->measurements)
        return (char *) (zlist_first (self->measurements));
    else
        return NULL;
}

const char *
common_msg_measurements_next (common_msg_t *self)
{
    assert (self);
    if (self->measurements)
        return (char *) (zlist_next (self->measurements));
    else
        return NULL;
}

void
common_msg_measurements_append (common_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->measurements) {
        self->measurements = zlist_new ();
        zlist_autofree (self->measurements);
    }
    zlist_append (self->measurements, string);
    free (string);
}

size_t
common_msg_measurements_size (common_msg_t *self)
{
    return zlist_size (self->measurements);
}



//  --------------------------------------------------------------------------
//  Selftest

int
common_msg_test (bool verbose)
{
    printf (" * common_msg: ");

    //  Silence an "unused" warning by "using" the verbose variable
    if (verbose) {;}

    //  @selftest
    //  Simple create/destroy test
    common_msg_t *self = common_msg_new (0);
    assert (self);
    common_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-common_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-common_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    common_msg_t *copy;
    self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_I);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mt_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_mt_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_MEASURE_TYPE_S);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mt_name (self, "Life is short but Now lasts for ever");
    common_msg_set_mt_unit (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (streq (common_msg_mt_name (self), "Life is short but Now lasts for ever"));
        assert (streq (common_msg_mt_unit (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_I);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mt_id (self, 123);
    common_msg_set_mts_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_mt_id (self) == 123);
        assert (common_msg_mts_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_MEASURE_SUBTYPE_S);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mt_id (self, 123);
    common_msg_set_mts_name (self, "Life is short but Now lasts for ever");
    common_msg_set_mts_scale (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_mt_id (self) == 123);
        assert (streq (common_msg_mts_name (self), "Life is short but Now lasts for ever"));
        assert (common_msg_mts_scale (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_MEASURE_TYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mt_id (self, 123);
    common_msg_set_mt_name (self, "Life is short but Now lasts for ever");
    common_msg_set_mt_unit (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_mt_id (self) == 123);
        assert (streq (common_msg_mt_name (self), "Life is short but Now lasts for ever"));
        assert (streq (common_msg_mt_unit (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_MEASURE_SUBTYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_mts_id (self, 123);
    common_msg_set_mt_id (self, 123);
    common_msg_set_mts_scale (self, 123);
    common_msg_set_mts_name (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_mts_id (self) == 123);
        assert (common_msg_mt_id (self) == 123);
        assert (common_msg_mts_scale (self) == 123);
        assert (streq (common_msg_mts_name (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_FAIL);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_errtype (self, 123);
    common_msg_set_errorno (self, 123);
    common_msg_set_errmsg (self, "Life is short but Now lasts for ever");
    common_msg_aux_insert (self, "Name", "Brutus");
    common_msg_aux_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_errtype (self) == 123);
        assert (common_msg_errorno (self) == 123);
        assert (streq (common_msg_errmsg (self), "Life is short but Now lasts for ever"));
        assert (common_msg_aux_size (self) == 2);
        assert (streq (common_msg_aux_string (self, "Name", "?"), "Brutus"));
        assert (common_msg_aux_number (self, "Age", 0) == 43);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DB_OK);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_rowid (self, 123);
    common_msg_aux_insert (self, "Name", "Brutus");
    common_msg_aux_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_rowid (self) == 123);
        assert (common_msg_aux_size (self) == 2);
        assert (streq (common_msg_aux_string (self, "Name", "?"), "Brutus"));
        assert (common_msg_aux_number (self, "Age", 0) == 43);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_name (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (streq (common_msg_name (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_INSERT_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    zmsg_t *insert_client_msg = zmsg_new ();
    common_msg_set_msg (self, &insert_client_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_UPDATE_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_client_id (self, 123);
    zmsg_t *update_client_msg = zmsg_new ();
    common_msg_set_msg (self, &update_client_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_client_id (self) == 123);
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DELETE_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_client_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_client_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_rowid (self, 123);
    zmsg_t *return_client_msg = zmsg_new ();
    common_msg_set_msg (self, &return_client_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_rowid (self) == 123);
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_NEW_MEASUREMENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_client_name (self, "Life is short but Now lasts for ever");
    common_msg_set_device_name (self, "Life is short but Now lasts for ever");
    common_msg_set_device_type (self, "Life is short but Now lasts for ever");
    common_msg_set_mt_id (self, 123);
    common_msg_set_mts_id (self, 123);
    common_msg_set_value (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (streq (common_msg_client_name (self), "Life is short but Now lasts for ever"));
        assert (streq (common_msg_device_name (self), "Life is short but Now lasts for ever"));
        assert (streq (common_msg_device_type (self), "Life is short but Now lasts for ever"));
        assert (common_msg_mt_id (self) == 123);
        assert (common_msg_mts_id (self) == 123);
        assert (common_msg_value (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_CLIENT_INFO);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_client_id (self, 123);
    common_msg_set_device_id (self, 123);
    zchunk_t *client_info_info = zchunk_new ("Captcha Diem", 12);
    common_msg_set_info (self, &client_info_info);
    common_msg_set_date (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_client_id (self) == 123);
        assert (common_msg_device_id (self) == 123);
        assert (memcmp (zchunk_data (common_msg_info (self)), "Captcha Diem", 12) == 0);
        assert (common_msg_date (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_INSERT_CINFO);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    zmsg_t *insert_cinfo_msg = zmsg_new ();
    common_msg_set_msg (self, &insert_cinfo_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DELETE_CINFO);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_cinfo_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_cinfo_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_CINFO);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_rowid (self, 123);
    zmsg_t *return_cinfo_msg = zmsg_new ();
    common_msg_set_msg (self, &return_cinfo_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_rowid (self) == 123);
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DEVICE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_devicetype_id (self, 123);
    common_msg_set_name (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_devicetype_id (self) == 123);
        assert (streq (common_msg_name (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_INSERT_DEVICE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    zmsg_t *insert_device_msg = zmsg_new ();
    common_msg_set_msg (self, &insert_device_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DELETE_DEVICE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_device_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_device_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_DEVICE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_rowid (self, 123);
    zmsg_t *return_device_msg = zmsg_new ();
    common_msg_set_msg (self, &return_device_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_rowid (self) == 123);
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_name (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (streq (common_msg_name (self), "Life is short but Now lasts for ever"));
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_INSERT_DEVTYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    zmsg_t *insert_devtype_msg = zmsg_new ();
    common_msg_set_msg (self, &insert_devtype_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_DELETE_DEVTYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_devicetype_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_devicetype_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_rowid (self, 123);
    zmsg_t *return_devtype_msg = zmsg_new ();
    common_msg_set_msg (self, &return_devtype_msg);
    zmsg_addstr (common_msg_msg (self), "Hello, World");
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_rowid (self) == 123);
        assert (zmsg_size (common_msg_msg (self)) == 1);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_CLIENT);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_client_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_client_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_CINFO);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_cinfo_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_cinfo_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_DEVICE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_device_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_device_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_DEVTYPE);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_devicetype_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_devicetype_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_GET_LAST_MEASUREMENTS);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_device_id (self, 123);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_device_id (self) == 123);
        common_msg_destroy (&self);
    }
    self = common_msg_new (COMMON_MSG_RETURN_LAST_MEASUREMENTS);
    
    //  Check that _dup works on empty message
    copy = common_msg_dup (self);
    assert (copy);
    common_msg_destroy (&copy);

    common_msg_set_device_id (self, 123);
    common_msg_set_device_name (self, "Life is short but Now lasts for ever");
    common_msg_measurements_append (self, "Name: %s", "Brutus");
    common_msg_measurements_append (self, "Age: %d", 43);
    //  Send twice from same object
    common_msg_send_again (self, output);
    common_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = common_msg_recv (input);
        assert (self);
        assert (common_msg_routing_id (self));
        
        assert (common_msg_device_id (self) == 123);
        assert (streq (common_msg_device_name (self), "Life is short but Now lasts for ever"));
        assert (common_msg_measurements_size (self) == 2);
        assert (streq (common_msg_measurements_first (self), "Name: Brutus"));
        assert (streq (common_msg_measurements_next (self), "Age: 43"));
        common_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
