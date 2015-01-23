/*  =========================================================================
    netdisc_msg - network discovery protocol
    
    Codec header for netdisc_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: netdisc_msg.xml, or
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

#ifndef __NETDISC_MSG_H_INCLUDED__
#define __NETDISC_MSG_H_INCLUDED__

/*  These are the netdisc_msg messages:

    AUTO_ADD - Add automatically discovered network.
        name                string      network interface name
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length
        mac                 string      mac address

    AUTO_DEL - Remove automatically discovered netowork.
        name                string      network interface name
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length
        mac                 string      mac address

    MAN_ADD - Manually added network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    MAN_DEL - Manually excluded network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    EXCL_ADD - Revert manually added network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    EXCL_DEL - Revert manually excluded network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length
*/

#define NETDISC_MSG_VERSION                 1.0

#define NETDISC_MSG_AUTO_ADD                1
#define NETDISC_MSG_AUTO_DEL                2
#define NETDISC_MSG_MAN_ADD                 3
#define NETDISC_MSG_MAN_DEL                 4
#define NETDISC_MSG_EXCL_ADD                5
#define NETDISC_MSG_EXCL_DEL                6

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef NETDISC_MSG_T_DEFINED
typedef struct _netdisc_msg_t netdisc_msg_t;
#define NETDISC_MSG_T_DEFINED
#endif

//  @interface
//  Create a new netdisc_msg
netdisc_msg_t *
    netdisc_msg_new (int id);

//  Destroy the netdisc_msg
void
    netdisc_msg_destroy (netdisc_msg_t **self_p);

//  Parse a zmsg_t and decides whether it is netdisc_msg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
    is_netdisc_msg (zmsg_t *msg_p);

//  Parse a netdisc_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
netdisc_msg_t *
    netdisc_msg_decode (zmsg_t **msg_p);

//  Encode netdisc_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    netdisc_msg_encode (netdisc_msg_t **self_p);

//  Receive and parse a netdisc_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
netdisc_msg_t *
    netdisc_msg_recv (void *input);

//  Receive and parse a netdisc_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
netdisc_msg_t *
    netdisc_msg_recv_nowait (void *input);

//  Send the netdisc_msg to the output, and destroy it
int
    netdisc_msg_send (netdisc_msg_t **self_p, void *output);

//  Send the netdisc_msg to the output, and do not destroy it
int
    netdisc_msg_send_again (netdisc_msg_t *self, void *output);

//  Encode the AUTO_ADD 
zmsg_t *
    netdisc_msg_encode_auto_add (
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);

//  Encode the AUTO_DEL 
zmsg_t *
    netdisc_msg_encode_auto_del (
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);

//  Encode the MAN_ADD 
zmsg_t *
    netdisc_msg_encode_man_add (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the MAN_DEL 
zmsg_t *
    netdisc_msg_encode_man_del (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the EXCL_ADD 
zmsg_t *
    netdisc_msg_encode_excl_add (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the EXCL_DEL 
zmsg_t *
    netdisc_msg_encode_excl_del (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);


//  Send the AUTO_ADD to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_auto_add (void *output,
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);
    
//  Send the AUTO_DEL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_auto_del (void *output,
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);
    
//  Send the MAN_ADD to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_man_add (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the MAN_DEL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_man_del (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the EXCL_ADD to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_excl_add (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the EXCL_DEL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_excl_del (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Duplicate the netdisc_msg message
netdisc_msg_t *
    netdisc_msg_dup (netdisc_msg_t *self);

//  Print contents of message to stdout
void
    netdisc_msg_print (netdisc_msg_t *self);

//  Get/set the message routing id
zframe_t *
    netdisc_msg_routing_id (netdisc_msg_t *self);
void
    netdisc_msg_set_routing_id (netdisc_msg_t *self, zframe_t *routing_id);

//  Get the netdisc_msg id and printable command
int
    netdisc_msg_id (netdisc_msg_t *self);
void
    netdisc_msg_set_id (netdisc_msg_t *self, int id);
const char *
    netdisc_msg_command (netdisc_msg_t *self);

//  Get/set the name field
const char *
    netdisc_msg_name (netdisc_msg_t *self);
void
    netdisc_msg_set_name (netdisc_msg_t *self, const char *format, ...);

//  Get/set the ipver field
byte
    netdisc_msg_ipver (netdisc_msg_t *self);
void
    netdisc_msg_set_ipver (netdisc_msg_t *self, byte ipver);

//  Get/set the ipaddr field
const char *
    netdisc_msg_ipaddr (netdisc_msg_t *self);
void
    netdisc_msg_set_ipaddr (netdisc_msg_t *self, const char *format, ...);

//  Get/set the prefixlen field
byte
    netdisc_msg_prefixlen (netdisc_msg_t *self);
void
    netdisc_msg_set_prefixlen (netdisc_msg_t *self, byte prefixlen);

//  Get/set the mac field
const char *
    netdisc_msg_mac (netdisc_msg_t *self);
void
    netdisc_msg_set_mac (netdisc_msg_t *self, const char *format, ...);

//  Self test of this class
int
    netdisc_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define netdisc_msg_dump    netdisc_msg_print

#ifdef __cplusplus
}
#endif

#endif
