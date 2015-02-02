/*  =========================================================================
    compute_msg - compute messages

    Codec class for compute_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: compute_msg.xml, or
     * The code generation script that built this file: zproto_codec_c_v1
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
    compute_msg - compute messages
@discuss
@end
*/

#include "./compute_msg.h"

//  Structure of our class

struct _compute_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  compute_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *module_name;                  //  Name of the computational module
    zhash_t *args;                      //  A list of arguments for computation
    size_t args_bytes;                  //  Size of dictionary content
    zhash_t *params;                    //  A list of module parameters
    size_t params_bytes;                //  Size of dictionary content
    zhash_t *results;                   //  List of results according to the list of parameters
    size_t results_bytes;               //  Size of dictionary content
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
//  Create a new compute_msg

compute_msg_t *
compute_msg_new (int id)
{
    compute_msg_t *self = (compute_msg_t *) zmalloc (sizeof (compute_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the compute_msg

void
compute_msg_destroy (compute_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        compute_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->module_name);
        zhash_destroy (&self->args);
        zhash_destroy (&self->params);
        zhash_destroy (&self->results);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  Parse a zmsg_t and decides whether it is compute_msg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
is_compute_msg (zmsg_t *msg)
{
    if (msg == NULL)
        return false;

    zframe_t *frame = zmsg_first (msg);

    //  Get and check protocol signature
    compute_msg_t *self = compute_msg_new (0);
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 10))
        goto fail;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
        case COMPUTE_MSG_RETURN_COMPUTATION:
            compute_msg_destroy (&self);
            return true;
        default:
            goto fail;
    }
    fail:
    malformed:
        compute_msg_destroy (&self);
        return false;
}

//  --------------------------------------------------------------------------
//  Parse a compute_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

compute_msg_t *
compute_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    compute_msg_t *self = compute_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 10))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            GET_STRING (self->module_name);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->args = zhash_new ();
                zhash_autofree (self->args);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->args, key, value);
                    free (key);
                    free (value);
                }
            }
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->params = zhash_new ();
                zhash_autofree (self->params);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->params, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        case COMPUTE_MSG_RETURN_COMPUTATION:
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->results = zhash_new ();
                zhash_autofree (self->results);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->results, key, value);
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
        compute_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode compute_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
compute_msg_encode (compute_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    compute_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            //  module_name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->module_name)
                frame_size += strlen (self->module_name);
            //  args is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->args) {
                self->args_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->args);
                while (item) {
                    self->args_bytes += 1 + strlen ((const char *) zhash_cursor (self->args));
                    self->args_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->args);
                }
            }
            frame_size += self->args_bytes;
            //  params is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->params) {
                self->params_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->params);
                while (item) {
                    self->params_bytes += 1 + strlen ((const char *) zhash_cursor (self->params));
                    self->params_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->params);
                }
            }
            frame_size += self->params_bytes;
            break;
            
        case COMPUTE_MSG_RETURN_COMPUTATION:
            //  results is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->results) {
                self->results_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->results);
                while (item) {
                    self->results_bytes += 1 + strlen ((const char *) zhash_cursor (self->results));
                    self->results_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->results);
                }
            }
            frame_size += self->results_bytes;
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 10);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            if (self->module_name) {
                PUT_STRING (self->module_name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->args) {
                PUT_NUMBER4 (zhash_size (self->args));
                char *item = (char *) zhash_first (self->args);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->args));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->args);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            if (self->params) {
                PUT_NUMBER4 (zhash_size (self->params));
                char *item = (char *) zhash_first (self->params);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->params));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->params);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case COMPUTE_MSG_RETURN_COMPUTATION:
            if (self->results) {
                PUT_NUMBER4 (zhash_size (self->results));
                char *item = (char *) zhash_first (self->results);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->results));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->results);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        compute_msg_destroy (self_p);
        return NULL;
    }
    //  Destroy compute_msg object
    compute_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a compute_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

compute_msg_t *
compute_msg_recv (void *input)
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
    compute_msg_t *compute_msg = compute_msg_decode (&msg);
    if (compute_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        compute_msg->routing_id = routing_id;

    return compute_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a compute_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

compute_msg_t *
compute_msg_recv_nowait (void *input)
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
    compute_msg_t *compute_msg = compute_msg_decode (&msg);
    if (compute_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        compute_msg->routing_id = routing_id;

    return compute_msg;
}


//  --------------------------------------------------------------------------
//  Send the compute_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
compute_msg_send (compute_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    compute_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode compute_msg message to a single zmsg
    zmsg_t *msg = compute_msg_encode (self_p);
    
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
//  Send the compute_msg to the output, and do not destroy it

int
compute_msg_send_again (compute_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = compute_msg_dup (self);
    return compute_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode GET_COMPUTATION message

zmsg_t * 
compute_msg_encode_get_computation (
    const char *module_name,
    zhash_t *args,
    zhash_t *params)
{
    compute_msg_t *self = compute_msg_new (COMPUTE_MSG_GET_COMPUTATION);
    compute_msg_set_module_name (self, module_name);
    zhash_t *args_copy = zhash_dup (args);
    compute_msg_set_args (self, &args_copy);
    zhash_t *params_copy = zhash_dup (params);
    compute_msg_set_params (self, &params_copy);
    return compute_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode RETURN_COMPUTATION message

zmsg_t * 
compute_msg_encode_return_computation (
    zhash_t *results)
{
    compute_msg_t *self = compute_msg_new (COMPUTE_MSG_RETURN_COMPUTATION);
    zhash_t *results_copy = zhash_dup (results);
    compute_msg_set_results (self, &results_copy);
    return compute_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the GET_COMPUTATION to the socket in one step

int
compute_msg_send_get_computation (
    void *output,
    const char *module_name,
    zhash_t *args,
    zhash_t *params)
{
    compute_msg_t *self = compute_msg_new (COMPUTE_MSG_GET_COMPUTATION);
    compute_msg_set_module_name (self, module_name);
    zhash_t *args_copy = zhash_dup (args);
    compute_msg_set_args (self, &args_copy);
    zhash_t *params_copy = zhash_dup (params);
    compute_msg_set_params (self, &params_copy);
    return compute_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the RETURN_COMPUTATION to the socket in one step

int
compute_msg_send_return_computation (
    void *output,
    zhash_t *results)
{
    compute_msg_t *self = compute_msg_new (COMPUTE_MSG_RETURN_COMPUTATION);
    zhash_t *results_copy = zhash_dup (results);
    compute_msg_set_results (self, &results_copy);
    return compute_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the compute_msg message

compute_msg_t *
compute_msg_dup (compute_msg_t *self)
{
    if (!self)
        return NULL;
        
    compute_msg_t *copy = compute_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            copy->module_name = self->module_name? strdup (self->module_name): NULL;
            copy->args = self->args? zhash_dup (self->args): NULL;
            copy->params = self->params? zhash_dup (self->params): NULL;
            break;

        case COMPUTE_MSG_RETURN_COMPUTATION:
            copy->results = self->results? zhash_dup (self->results): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
compute_msg_print (compute_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            zsys_debug ("COMPUTE_MSG_GET_COMPUTATION:");
            if (self->module_name)
                zsys_debug ("    module_name='%s'", self->module_name);
            else
                zsys_debug ("    module_name=");
            zsys_debug ("    args=");
            if (self->args) {
                char *item = (char *) zhash_first (self->args);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->args), item);
                    item = (char *) zhash_next (self->args);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    params=");
            if (self->params) {
                char *item = (char *) zhash_first (self->params);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->params), item);
                    item = (char *) zhash_next (self->params);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
        case COMPUTE_MSG_RETURN_COMPUTATION:
            zsys_debug ("COMPUTE_MSG_RETURN_COMPUTATION:");
            zsys_debug ("    results=");
            if (self->results) {
                char *item = (char *) zhash_first (self->results);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->results), item);
                    item = (char *) zhash_next (self->results);
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
compute_msg_routing_id (compute_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
compute_msg_set_routing_id (compute_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the compute_msg id

int
compute_msg_id (compute_msg_t *self)
{
    assert (self);
    return self->id;
}

void
compute_msg_set_id (compute_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
compute_msg_command (compute_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case COMPUTE_MSG_GET_COMPUTATION:
            return ("GET_COMPUTATION");
            break;
        case COMPUTE_MSG_RETURN_COMPUTATION:
            return ("RETURN_COMPUTATION");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the module_name field

const char *
compute_msg_module_name (compute_msg_t *self)
{
    assert (self);
    return self->module_name;
}

void
compute_msg_set_module_name (compute_msg_t *self, const char *format, ...)
{
    //  Format module_name from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->module_name);
    self->module_name = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the args field without transferring ownership

zhash_t *
compute_msg_args (compute_msg_t *self)
{
    assert (self);
    return self->args;
}

//  Get the args field and transfer ownership to caller

zhash_t *
compute_msg_get_args (compute_msg_t *self)
{
    zhash_t *args = self->args;
    self->args = NULL;
    return args;
}

//  Set the args field, transferring ownership from caller

void
compute_msg_set_args (compute_msg_t *self, zhash_t **args_p)
{
    assert (self);
    assert (args_p);
    zhash_destroy (&self->args);
    self->args = *args_p;
    *args_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the args dictionary

const char *
compute_msg_args_string (compute_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->args)
        value = (const char *) (zhash_lookup (self->args, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
compute_msg_args_number (compute_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->args)
        string = (char *) (zhash_lookup (self->args, key));
    if (string)
        value = atol (string);

    return value;
}

void
compute_msg_args_insert (compute_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->args) {
        self->args = zhash_new ();
        zhash_autofree (self->args);
    }
    zhash_update (self->args, key, string);
    free (string);
}

size_t
compute_msg_args_size (compute_msg_t *self)
{
    return zhash_size (self->args);
}


//  --------------------------------------------------------------------------
//  Get the params field without transferring ownership

zhash_t *
compute_msg_params (compute_msg_t *self)
{
    assert (self);
    return self->params;
}

//  Get the params field and transfer ownership to caller

zhash_t *
compute_msg_get_params (compute_msg_t *self)
{
    zhash_t *params = self->params;
    self->params = NULL;
    return params;
}

//  Set the params field, transferring ownership from caller

void
compute_msg_set_params (compute_msg_t *self, zhash_t **params_p)
{
    assert (self);
    assert (params_p);
    zhash_destroy (&self->params);
    self->params = *params_p;
    *params_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the params dictionary

const char *
compute_msg_params_string (compute_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->params)
        value = (const char *) (zhash_lookup (self->params, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
compute_msg_params_number (compute_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->params)
        string = (char *) (zhash_lookup (self->params, key));
    if (string)
        value = atol (string);

    return value;
}

void
compute_msg_params_insert (compute_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->params) {
        self->params = zhash_new ();
        zhash_autofree (self->params);
    }
    zhash_update (self->params, key, string);
    free (string);
}

size_t
compute_msg_params_size (compute_msg_t *self)
{
    return zhash_size (self->params);
}


//  --------------------------------------------------------------------------
//  Get the results field without transferring ownership

zhash_t *
compute_msg_results (compute_msg_t *self)
{
    assert (self);
    return self->results;
}

//  Get the results field and transfer ownership to caller

zhash_t *
compute_msg_get_results (compute_msg_t *self)
{
    zhash_t *results = self->results;
    self->results = NULL;
    return results;
}

//  Set the results field, transferring ownership from caller

void
compute_msg_set_results (compute_msg_t *self, zhash_t **results_p)
{
    assert (self);
    assert (results_p);
    zhash_destroy (&self->results);
    self->results = *results_p;
    *results_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the results dictionary

const char *
compute_msg_results_string (compute_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->results)
        value = (const char *) (zhash_lookup (self->results, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
compute_msg_results_number (compute_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->results)
        string = (char *) (zhash_lookup (self->results, key));
    if (string)
        value = atol (string);

    return value;
}

void
compute_msg_results_insert (compute_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->results) {
        self->results = zhash_new ();
        zhash_autofree (self->results);
    }
    zhash_update (self->results, key, string);
    free (string);
}

size_t
compute_msg_results_size (compute_msg_t *self)
{
    return zhash_size (self->results);
}



//  --------------------------------------------------------------------------
//  Selftest

int
compute_msg_test (bool verbose)
{
    printf (" * compute_msg: ");
    if (verbose) {;}	// silence an "unused" warning;
    // TODO: properly fix this in template zproto : zproto_codec_c_v1.gsl
    // so as to not lose the fix upon regeneration of code

    //  @selftest
    //  Simple create/destroy test
    compute_msg_t *self = compute_msg_new (0);
    assert (self);
    compute_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-compute_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-compute_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    compute_msg_t *copy;
    self = compute_msg_new (COMPUTE_MSG_GET_COMPUTATION);
    
    //  Check that _dup works on empty message
    copy = compute_msg_dup (self);
    assert (copy);
    compute_msg_destroy (&copy);

    compute_msg_set_module_name (self, "Life is short but Now lasts for ever");
    compute_msg_args_insert (self, "Name", "Brutus");
    compute_msg_args_insert (self, "Age", "%d", 43);
    compute_msg_params_insert (self, "Name", "Brutus");
    compute_msg_params_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    compute_msg_send_again (self, output);
    compute_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = compute_msg_recv (input);
        assert (self);
        assert (compute_msg_routing_id (self));
        
        assert (streq (compute_msg_module_name (self), "Life is short but Now lasts for ever"));
        assert (compute_msg_args_size (self) == 2);
        assert (streq (compute_msg_args_string (self, "Name", "?"), "Brutus"));
        assert (compute_msg_args_number (self, "Age", 0) == 43);
        assert (compute_msg_params_size (self) == 2);
        assert (streq (compute_msg_params_string (self, "Name", "?"), "Brutus"));
        assert (compute_msg_params_number (self, "Age", 0) == 43);
        compute_msg_destroy (&self);
    }
    self = compute_msg_new (COMPUTE_MSG_RETURN_COMPUTATION);
    
    //  Check that _dup works on empty message
    copy = compute_msg_dup (self);
    assert (copy);
    compute_msg_destroy (&copy);

    compute_msg_results_insert (self, "Name", "Brutus");
    compute_msg_results_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    compute_msg_send_again (self, output);
    compute_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = compute_msg_recv (input);
        assert (self);
        assert (compute_msg_routing_id (self));
        
        assert (compute_msg_results_size (self) == 2);
        assert (streq (compute_msg_results_string (self, "Name", "?"), "Brutus"));
        assert (compute_msg_results_number (self, "Age", 0) == 43);
        compute_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
