/*  =========================================================================
    netdisc_msg - network discovery protocol

    Codec class for netdisc_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: netdisc_msg.xml, or
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
    netdisc_msg - network discovery protocol
@discuss
@end
*/

#include "./netdisc_msg.h"

//  Structure of our class

struct _netdisc_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  netdisc_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *name;                         //  network interface name
    byte ipver;                         //  ip version; 0 - IPV4, 1 - IPV6
    char *ipaddr;                       //  ip address
    byte prefixlen;                     //  ip prefix length
    char *mac;                          //  mac address
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
//  Create a new netdisc_msg

netdisc_msg_t *
netdisc_msg_new (int id)
{
    netdisc_msg_t *self = (netdisc_msg_t *) zmalloc (sizeof (netdisc_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the netdisc_msg

void
netdisc_msg_destroy (netdisc_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        netdisc_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->name);
        free (self->ipaddr);
        free (self->mac);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a netdisc_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

netdisc_msg_t *
netdisc_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    netdisc_msg_t *self = netdisc_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 0))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            GET_STRING (self->name);
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
            GET_STRING (self->mac);
            break;

        case NETDISC_MSG_AUTO_DEL:
            GET_STRING (self->name);
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
            GET_STRING (self->mac);
            break;

        case NETDISC_MSG_MAN_ADD:
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_MAN_DEL:
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_EXCL_ADD:
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_EXCL_DEL:
            GET_NUMBER1 (self->ipver);
            GET_STRING (self->ipaddr);
            GET_NUMBER1 (self->prefixlen);
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
        netdisc_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode netdisc_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
netdisc_msg_encode (netdisc_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    netdisc_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            //  mac is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mac)
                frame_size += strlen (self->mac);
            break;
            
        case NETDISC_MSG_AUTO_DEL:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            //  mac is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->mac)
                frame_size += strlen (self->mac);
            break;
            
        case NETDISC_MSG_MAN_ADD:
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            break;
            
        case NETDISC_MSG_MAN_DEL:
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            break;
            
        case NETDISC_MSG_EXCL_ADD:
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            break;
            
        case NETDISC_MSG_EXCL_DEL:
            //  ipver is a 1-byte integer
            frame_size += 1;
            //  ipaddr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ipaddr)
                frame_size += strlen (self->ipaddr);
            //  prefixlen is a 1-byte integer
            frame_size += 1;
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 0);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            if (self->mac) {
                PUT_STRING (self->mac);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case NETDISC_MSG_AUTO_DEL:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            if (self->mac) {
                PUT_STRING (self->mac);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case NETDISC_MSG_MAN_ADD:
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_MAN_DEL:
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_EXCL_ADD:
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            break;

        case NETDISC_MSG_EXCL_DEL:
            PUT_NUMBER1 (self->ipver);
            if (self->ipaddr) {
                PUT_STRING (self->ipaddr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->prefixlen);
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        netdisc_msg_destroy (self_p);
        return NULL;
    }
    //  Destroy netdisc_msg object
    netdisc_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a netdisc_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

netdisc_msg_t *
netdisc_msg_recv (void *input)
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
    netdisc_msg_t *netdisc_msg = netdisc_msg_decode (&msg);
    if (netdisc_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        netdisc_msg->routing_id = routing_id;

    return netdisc_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a netdisc_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

netdisc_msg_t *
netdisc_msg_recv_nowait (void *input)
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
    netdisc_msg_t *netdisc_msg = netdisc_msg_decode (&msg);
    if (netdisc_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        netdisc_msg->routing_id = routing_id;

    return netdisc_msg;
}


//  --------------------------------------------------------------------------
//  Send the netdisc_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
netdisc_msg_send (netdisc_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    netdisc_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode netdisc_msg message to a single zmsg
    zmsg_t *msg = netdisc_msg_encode (self_p);
    
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
//  Send the netdisc_msg to the output, and do not destroy it

int
netdisc_msg_send_again (netdisc_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = netdisc_msg_dup (self);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode AUTO_ADD message

zmsg_t * 
netdisc_msg_encode_auto_add (
    const char *name,
    byte ipver,
    const char *ipaddr,
    byte prefixlen,
    const char *mac)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_AUTO_ADD);
    netdisc_msg_set_name (self, name);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    netdisc_msg_set_mac (self, mac);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode AUTO_DEL message

zmsg_t * 
netdisc_msg_encode_auto_del (
    const char *name,
    byte ipver,
    const char *ipaddr,
    byte prefixlen,
    const char *mac)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_AUTO_DEL);
    netdisc_msg_set_name (self, name);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    netdisc_msg_set_mac (self, mac);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode MAN_ADD message

zmsg_t * 
netdisc_msg_encode_man_add (
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_MAN_ADD);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode MAN_DEL message

zmsg_t * 
netdisc_msg_encode_man_del (
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_MAN_DEL);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode EXCL_ADD message

zmsg_t * 
netdisc_msg_encode_excl_add (
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_EXCL_ADD);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode EXCL_DEL message

zmsg_t * 
netdisc_msg_encode_excl_del (
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_EXCL_DEL);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the AUTO_ADD to the socket in one step

int
netdisc_msg_send_auto_add (
    void *output,
    const char *name,
    byte ipver,
    const char *ipaddr,
    byte prefixlen,
    const char *mac)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_AUTO_ADD);
    netdisc_msg_set_name (self, name);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    netdisc_msg_set_mac (self, mac);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the AUTO_DEL to the socket in one step

int
netdisc_msg_send_auto_del (
    void *output,
    const char *name,
    byte ipver,
    const char *ipaddr,
    byte prefixlen,
    const char *mac)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_AUTO_DEL);
    netdisc_msg_set_name (self, name);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    netdisc_msg_set_mac (self, mac);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the MAN_ADD to the socket in one step

int
netdisc_msg_send_man_add (
    void *output,
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_MAN_ADD);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the MAN_DEL to the socket in one step

int
netdisc_msg_send_man_del (
    void *output,
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_MAN_DEL);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the EXCL_ADD to the socket in one step

int
netdisc_msg_send_excl_add (
    void *output,
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_EXCL_ADD);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the EXCL_DEL to the socket in one step

int
netdisc_msg_send_excl_del (
    void *output,
    byte ipver,
    const char *ipaddr,
    byte prefixlen)
{
    netdisc_msg_t *self = netdisc_msg_new (NETDISC_MSG_EXCL_DEL);
    netdisc_msg_set_ipver (self, ipver);
    netdisc_msg_set_ipaddr (self, ipaddr);
    netdisc_msg_set_prefixlen (self, prefixlen);
    return netdisc_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the netdisc_msg message

netdisc_msg_t *
netdisc_msg_dup (netdisc_msg_t *self)
{
    if (!self)
        return NULL;
        
    netdisc_msg_t *copy = netdisc_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            copy->name = self->name? strdup (self->name): NULL;
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            copy->mac = self->mac? strdup (self->mac): NULL;
            break;

        case NETDISC_MSG_AUTO_DEL:
            copy->name = self->name? strdup (self->name): NULL;
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            copy->mac = self->mac? strdup (self->mac): NULL;
            break;

        case NETDISC_MSG_MAN_ADD:
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            break;

        case NETDISC_MSG_MAN_DEL:
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            break;

        case NETDISC_MSG_EXCL_ADD:
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            break;

        case NETDISC_MSG_EXCL_DEL:
            copy->ipver = self->ipver;
            copy->ipaddr = self->ipaddr? strdup (self->ipaddr): NULL;
            copy->prefixlen = self->prefixlen;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
netdisc_msg_print (netdisc_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            zsys_debug ("NETDISC_MSG_AUTO_ADD:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            if (self->mac)
                zsys_debug ("    mac='%s'", self->mac);
            else
                zsys_debug ("    mac=");
            break;
            
        case NETDISC_MSG_AUTO_DEL:
            zsys_debug ("NETDISC_MSG_AUTO_DEL:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            if (self->mac)
                zsys_debug ("    mac='%s'", self->mac);
            else
                zsys_debug ("    mac=");
            break;
            
        case NETDISC_MSG_MAN_ADD:
            zsys_debug ("NETDISC_MSG_MAN_ADD:");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            break;
            
        case NETDISC_MSG_MAN_DEL:
            zsys_debug ("NETDISC_MSG_MAN_DEL:");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            break;
            
        case NETDISC_MSG_EXCL_ADD:
            zsys_debug ("NETDISC_MSG_EXCL_ADD:");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            break;
            
        case NETDISC_MSG_EXCL_DEL:
            zsys_debug ("NETDISC_MSG_EXCL_DEL:");
            zsys_debug ("    ipver=%ld", (long) self->ipver);
            if (self->ipaddr)
                zsys_debug ("    ipaddr='%s'", self->ipaddr);
            else
                zsys_debug ("    ipaddr=");
            zsys_debug ("    prefixlen=%ld", (long) self->prefixlen);
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
netdisc_msg_routing_id (netdisc_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
netdisc_msg_set_routing_id (netdisc_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the netdisc_msg id

int
netdisc_msg_id (netdisc_msg_t *self)
{
    assert (self);
    return self->id;
}

void
netdisc_msg_set_id (netdisc_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
netdisc_msg_command (netdisc_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case NETDISC_MSG_AUTO_ADD:
            return ("AUTO_ADD");
            break;
        case NETDISC_MSG_AUTO_DEL:
            return ("AUTO_DEL");
            break;
        case NETDISC_MSG_MAN_ADD:
            return ("MAN_ADD");
            break;
        case NETDISC_MSG_MAN_DEL:
            return ("MAN_DEL");
            break;
        case NETDISC_MSG_EXCL_ADD:
            return ("EXCL_ADD");
            break;
        case NETDISC_MSG_EXCL_DEL:
            return ("EXCL_DEL");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the name field

const char *
netdisc_msg_name (netdisc_msg_t *self)
{
    assert (self);
    return self->name;
}

void
netdisc_msg_set_name (netdisc_msg_t *self, const char *format, ...)
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
//  Get/set the ipver field

byte
netdisc_msg_ipver (netdisc_msg_t *self)
{
    assert (self);
    return self->ipver;
}

void
netdisc_msg_set_ipver (netdisc_msg_t *self, byte ipver)
{
    assert (self);
    self->ipver = ipver;
}


//  --------------------------------------------------------------------------
//  Get/set the ipaddr field

const char *
netdisc_msg_ipaddr (netdisc_msg_t *self)
{
    assert (self);
    return self->ipaddr;
}

void
netdisc_msg_set_ipaddr (netdisc_msg_t *self, const char *format, ...)
{
    //  Format ipaddr from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->ipaddr);
    self->ipaddr = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the prefixlen field

byte
netdisc_msg_prefixlen (netdisc_msg_t *self)
{
    assert (self);
    return self->prefixlen;
}

void
netdisc_msg_set_prefixlen (netdisc_msg_t *self, byte prefixlen)
{
    assert (self);
    self->prefixlen = prefixlen;
}


//  --------------------------------------------------------------------------
//  Get/set the mac field

const char *
netdisc_msg_mac (netdisc_msg_t *self)
{
    assert (self);
    return self->mac;
}

void
netdisc_msg_set_mac (netdisc_msg_t *self, const char *format, ...)
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
//  Selftest

int
netdisc_msg_test (bool verbose)
{
    printf (" * netdisc_msg: ");

    //  @selftest
    //  Simple create/destroy test
    netdisc_msg_t *self = netdisc_msg_new (0);
    assert (self);
    netdisc_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-netdisc_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-netdisc_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    netdisc_msg_t *copy;
    self = netdisc_msg_new (NETDISC_MSG_AUTO_ADD);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_name (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    netdisc_msg_set_mac (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (streq (netdisc_msg_name (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        assert (streq (netdisc_msg_mac (self), "Life is short but Now lasts for ever"));
        netdisc_msg_destroy (&self);
    }
    self = netdisc_msg_new (NETDISC_MSG_AUTO_DEL);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_name (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    netdisc_msg_set_mac (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (streq (netdisc_msg_name (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        assert (streq (netdisc_msg_mac (self), "Life is short but Now lasts for ever"));
        netdisc_msg_destroy (&self);
    }
    self = netdisc_msg_new (NETDISC_MSG_MAN_ADD);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        netdisc_msg_destroy (&self);
    }
    self = netdisc_msg_new (NETDISC_MSG_MAN_DEL);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        netdisc_msg_destroy (&self);
    }
    self = netdisc_msg_new (NETDISC_MSG_EXCL_ADD);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        netdisc_msg_destroy (&self);
    }
    self = netdisc_msg_new (NETDISC_MSG_EXCL_DEL);
    
    //  Check that _dup works on empty message
    copy = netdisc_msg_dup (self);
    assert (copy);
    netdisc_msg_destroy (&copy);

    netdisc_msg_set_ipver (self, 123);
    netdisc_msg_set_ipaddr (self, "Life is short but Now lasts for ever");
    netdisc_msg_set_prefixlen (self, 123);
    //  Send twice from same object
    netdisc_msg_send_again (self, output);
    netdisc_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = netdisc_msg_recv (input);
        assert (self);
        assert (netdisc_msg_routing_id (self));
        
        assert (netdisc_msg_ipver (self) == 123);
        assert (streq (netdisc_msg_ipaddr (self), "Life is short but Now lasts for ever"));
        assert (netdisc_msg_prefixlen (self) == 123);
        netdisc_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
