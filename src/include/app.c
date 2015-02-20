/*  =========================================================================
    app - draft

    Codec class for app.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: application.xml, or
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
    app - draft
@discuss
@end
*/

#include "./app.h"

//  Structure of our class

struct _app_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  app message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *name;                         //  
    zlist_t *params;                    //  List of parameters for module
    zhash_t *args;                      //  
    size_t args_bytes;                  //  Size of dictionary content
    uint32_t op;                        //  
    zchunk_t *bin;                      //  In case we want to store pictures in db, etc...
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
//  Create a new app

app_t *
app_new (int id)
{
    app_t *self = (app_t *) zmalloc (sizeof (app_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the app

void
app_destroy (app_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        app_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->name);
        if (self->params)
            zlist_destroy (&self->params);
        zhash_destroy (&self->args);
        zchunk_destroy (&self->bin);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

//  Parse a zmsg_t and decides whether it is app. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
is_app (zmsg_t *msg)
{
    if (msg == NULL)
        return false;

    zframe_t *frame = zmsg_first (msg);

    //  Get and check protocol signature
    app_t *self = app_new (0);
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 1))
        goto fail;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case APP_MODULE:
        case APP_DB:
            app_destroy (&self);
            return true;
        default:
            goto fail;
    }
    fail:
    malformed:
        app_destroy (&self);
        return false;
}

//  --------------------------------------------------------------------------
//  Parse a app from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

app_t *
app_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    app_t *self = app_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 1))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case APP_MODULE:
            GET_STRING (self->name);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->params = zlist_new ();
                zlist_autofree (self->params);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->params, string);
                    free (string);
                }
            }
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
            break;

        case APP_DB:
            GET_NUMBER4 (self->op);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->params = zlist_new ();
                zlist_autofree (self->params);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->params, string);
                    free (string);
                }
            }
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
                size_t chunk_size;
                GET_NUMBER4 (chunk_size);
                if (self->needle + chunk_size > (self->ceiling))
                    goto malformed;
                self->bin = zchunk_new (self->needle, chunk_size);
                self->needle += chunk_size;
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
        app_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode app into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
app_encode (app_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    app_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case APP_MODULE:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  params is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->params) {
                //  Add up size of list contents
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    frame_size += 4 + strlen (params);
                    params = (char *) zlist_next (self->params);
                }
            }
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
            break;
            
        case APP_DB:
            //  op is a 4-byte integer
            frame_size += 4;
            //  params is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->params) {
                //  Add up size of list contents
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    frame_size += 4 + strlen (params);
                    params = (char *) zlist_next (self->params);
                }
            }
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
            //  bin is a chunk with 4-byte length
            frame_size += 4;
            if (self->bin)
                frame_size += zchunk_size (self->bin);
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 1);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case APP_MODULE:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->params) {
                PUT_NUMBER4 (zlist_size (self->params));
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    PUT_LONGSTR (params);
                    params = (char *) zlist_next (self->params);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
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
            break;

        case APP_DB:
            PUT_NUMBER4 (self->op);
            if (self->params) {
                PUT_NUMBER4 (zlist_size (self->params));
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    PUT_LONGSTR (params);
                    params = (char *) zlist_next (self->params);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
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
            if (self->bin) {
                PUT_NUMBER4 (zchunk_size (self->bin));
                memcpy (self->needle,
                        zchunk_data (self->bin),
                        zchunk_size (self->bin));
                self->needle += zchunk_size (self->bin);
            }
            else
                PUT_NUMBER4 (0);    //  Empty chunk
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        app_destroy (self_p);
        return NULL;
    }
    //  Destroy app object
    app_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a app from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

app_t *
app_recv (void *input)
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
    app_t *app = app_decode (&msg);
    if (app && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        app->routing_id = routing_id;

    return app;
}


//  --------------------------------------------------------------------------
//  Receive and parse a app from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

app_t *
app_recv_nowait (void *input)
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
    app_t *app = app_decode (&msg);
    if (app && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        app->routing_id = routing_id;

    return app;
}


//  --------------------------------------------------------------------------
//  Send the app to the socket, and destroy it
//  Returns 0 if OK, else -1

int
app_send (app_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    app_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode app message to a single zmsg
    zmsg_t *msg = app_encode (self_p);
    
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
//  Send the app to the output, and do not destroy it

int
app_send_again (app_t *self, void *output)
{
    assert (self);
    assert (output);
    self = app_dup (self);
    return app_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode MODULE message

zmsg_t * 
app_encode_module (
    const char *name,
    zlist_t *params,
    zhash_t *args)
{
    app_t *self = app_new (APP_MODULE);
    app_set_name (self, "%s", name);
    zlist_t *params_copy = zlist_dup (params);
    app_set_params (self, &params_copy);
    zhash_t *args_copy = zhash_dup (args);
    app_set_args (self, &args_copy);
    return app_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DB message

zmsg_t * 
app_encode_db (
    uint32_t op,
    zlist_t *params,
    zhash_t *args,
    zchunk_t *bin)
{
    app_t *self = app_new (APP_DB);
    app_set_op (self, op);
    zlist_t *params_copy = zlist_dup (params);
    app_set_params (self, &params_copy);
    zhash_t *args_copy = zhash_dup (args);
    app_set_args (self, &args_copy);
    zchunk_t *bin_copy = zchunk_dup (bin);
    app_set_bin (self, &bin_copy);
    return app_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the MODULE to the socket in one step

int
app_send_module (
    void *output,
    const char *name,
    zlist_t *params,
    zhash_t *args)
{
    app_t *self = app_new (APP_MODULE);
    app_set_name (self, name);
    zlist_t *params_copy = zlist_dup (params);
    app_set_params (self, &params_copy);
    zhash_t *args_copy = zhash_dup (args);
    app_set_args (self, &args_copy);
    return app_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DB to the socket in one step

int
app_send_db (
    void *output,
    uint32_t op,
    zlist_t *params,
    zhash_t *args,
    zchunk_t *bin)
{
    app_t *self = app_new (APP_DB);
    app_set_op (self, op);
    zlist_t *params_copy = zlist_dup (params);
    app_set_params (self, &params_copy);
    zhash_t *args_copy = zhash_dup (args);
    app_set_args (self, &args_copy);
    zchunk_t *bin_copy = zchunk_dup (bin);
    app_set_bin (self, &bin_copy);
    return app_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the app message

app_t *
app_dup (app_t *self)
{
    if (!self)
        return NULL;
        
    app_t *copy = app_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case APP_MODULE:
            copy->name = self->name? strdup (self->name): NULL;
            copy->params = self->params? zlist_dup (self->params): NULL;
            copy->args = self->args? zhash_dup (self->args): NULL;
            break;

        case APP_DB:
            copy->op = self->op;
            copy->params = self->params? zlist_dup (self->params): NULL;
            copy->args = self->args? zhash_dup (self->args): NULL;
            copy->bin = self->bin? zchunk_dup (self->bin): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
app_print (app_t *self)
{
    assert (self);
    switch (self->id) {
        case APP_MODULE:
            zsys_debug ("APP_MODULE:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    params=");
            if (self->params) {
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    zsys_debug ("        '%s'", params);
                    params = (char *) zlist_next (self->params);
                }
            }
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
            break;
            
        case APP_DB:
            zsys_debug ("APP_DB:");
            zsys_debug ("    op=%ld", (long) self->op);
            zsys_debug ("    params=");
            if (self->params) {
                char *params = (char *) zlist_first (self->params);
                while (params) {
                    zsys_debug ("        '%s'", params);
                    params = (char *) zlist_next (self->params);
                }
            }
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
            zsys_debug ("    bin=[ ... ]");
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
app_routing_id (app_t *self)
{
    assert (self);
    return self->routing_id;
}

void
app_set_routing_id (app_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the app id

int
app_id (app_t *self)
{
    assert (self);
    return self->id;
}

void
app_set_id (app_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
app_command (app_t *self)
{
    assert (self);
    switch (self->id) {
        case APP_MODULE:
            return ("MODULE");
            break;
        case APP_DB:
            return ("DB");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the name field

const char *
app_name (app_t *self)
{
    assert (self);
    return self->name;
}

void
app_set_name (app_t *self, const char *format, ...)
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
//  Get the params field, without transferring ownership

zlist_t *
app_params (app_t *self)
{
    assert (self);
    return self->params;
}

//  Get the params field and transfer ownership to caller

zlist_t *
app_get_params (app_t *self)
{
    assert (self);
    zlist_t *params = self->params;
    self->params = NULL;
    return params;
}

//  Set the params field, transferring ownership from caller

void
app_set_params (app_t *self, zlist_t **params_p)
{
    assert (self);
    assert (params_p);
    zlist_destroy (&self->params);
    self->params = *params_p;
    *params_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the params field, and append a params value

const char *
app_params_first (app_t *self)
{
    assert (self);
    if (self->params)
        return (char *) (zlist_first (self->params));
    else
        return NULL;
}

const char *
app_params_next (app_t *self)
{
    assert (self);
    if (self->params)
        return (char *) (zlist_next (self->params));
    else
        return NULL;
}

void
app_params_append (app_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->params) {
        self->params = zlist_new ();
        zlist_autofree (self->params);
    }
    zlist_append (self->params, string);
    free (string);
}

size_t
app_params_size (app_t *self)
{
    return zlist_size (self->params);
}


//  --------------------------------------------------------------------------
//  Get the args field without transferring ownership

zhash_t *
app_args (app_t *self)
{
    assert (self);
    return self->args;
}

//  Get the args field and transfer ownership to caller

zhash_t *
app_get_args (app_t *self)
{
    zhash_t *args = self->args;
    self->args = NULL;
    return args;
}

//  Set the args field, transferring ownership from caller

void
app_set_args (app_t *self, zhash_t **args_p)
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
app_args_string (app_t *self, const char *key, const char *default_value)
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
app_args_number (app_t *self, const char *key, uint64_t default_value)
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
app_args_insert (app_t *self, const char *key, const char *format, ...)
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
app_args_size (app_t *self)
{
    return zhash_size (self->args);
}


//  --------------------------------------------------------------------------
//  Get/set the op field

uint32_t
app_op (app_t *self)
{
    assert (self);
    return self->op;
}

void
app_set_op (app_t *self, uint32_t op)
{
    assert (self);
    self->op = op;
}


//  --------------------------------------------------------------------------
//  Get the bin field without transferring ownership

zchunk_t *
app_bin (app_t *self)
{
    assert (self);
    return self->bin;
}

//  Get the bin field and transfer ownership to caller

zchunk_t *
app_get_bin (app_t *self)
{
    zchunk_t *bin = self->bin;
    self->bin = NULL;
    return bin;
}

//  Set the bin field, transferring ownership from caller

void
app_set_bin (app_t *self, zchunk_t **chunk_p)
{
    assert (self);
    assert (chunk_p);
    zchunk_destroy (&self->bin);
    self->bin = *chunk_p;
    *chunk_p = NULL;
}



//  --------------------------------------------------------------------------
//  Selftest

int
app_test (bool verbose)
{
    printf (" * app: ");

    //  Silence an "unused" warning by "using" the verbose variable
    if (verbose) {;}

    //  @selftest
    //  Simple create/destroy test
    app_t *self = app_new (0);
    assert (self);
    app_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-app");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-app");

    //  Encode/send/decode and verify each message type
    int instance;
    app_t *copy;
    self = app_new (APP_MODULE);
    
    //  Check that _dup works on empty message
    copy = app_dup (self);
    assert (copy);
    app_destroy (&copy);

    app_set_name (self, "Life is short but Now lasts for ever");
    app_params_append (self, "Name: %s", "Brutus");
    app_params_append (self, "Age: %d", 43);
    app_args_insert (self, "Name", "Brutus");
    app_args_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    app_send_again (self, output);
    app_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = app_recv (input);
        assert (self);
        assert (app_routing_id (self));
        
        assert (streq (app_name (self), "Life is short but Now lasts for ever"));
        assert (app_params_size (self) == 2);
        assert (streq (app_params_first (self), "Name: Brutus"));
        assert (streq (app_params_next (self), "Age: 43"));
        assert (app_args_size (self) == 2);
        assert (streq (app_args_string (self, "Name", "?"), "Brutus"));
        assert (app_args_number (self, "Age", 0) == 43);
        app_destroy (&self);
    }
    self = app_new (APP_DB);
    
    //  Check that _dup works on empty message
    copy = app_dup (self);
    assert (copy);
    app_destroy (&copy);

    app_set_op (self, 123);
    app_params_append (self, "Name: %s", "Brutus");
    app_params_append (self, "Age: %d", 43);
    app_args_insert (self, "Name", "Brutus");
    app_args_insert (self, "Age", "%d", 43);
    zchunk_t *db_bin = zchunk_new ("Captcha Diem", 12);
    app_set_bin (self, &db_bin);
    //  Send twice from same object
    app_send_again (self, output);
    app_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = app_recv (input);
        assert (self);
        assert (app_routing_id (self));
        
        assert (app_op (self) == 123);
        assert (app_params_size (self) == 2);
        assert (streq (app_params_first (self), "Name: Brutus"));
        assert (streq (app_params_next (self), "Age: 43"));
        assert (app_args_size (self) == 2);
        assert (streq (app_args_string (self, "Name", "?"), "Brutus"));
        assert (app_args_number (self, "Age", 0) == 43);
        assert (memcmp (zchunk_data (app_bin (self)), "Captcha Diem", 12) == 0);
        app_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
