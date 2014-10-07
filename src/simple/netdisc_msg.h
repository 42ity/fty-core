/*  =========================================================================
    netdisc_msg - network discovery protocol
    
    Codec header for netdisc_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: netdisc_msg.xml
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

#ifndef __NETDISC_MSG_H_INCLUDED__
#define __NETDISC_MSG_H_INCLUDED__

/*  These are the netdisc_msg messages:

    OS_ADD - Add automatically discovered network.
        name                string      network interface name
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length
        mac                 string      mac address

    OS_DEL - Remove automatically discovered netowork.
        name                string      network interface name
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length
        mac                 string      mac address

    MAN_ADD - Manually added network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    MAN_EXCL - Manually excluded network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    REV_ADD - Revert manually added network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    REV_EXCL - Revert manually excluded network.
        ipver               number 1    ip version; 0 - IPV4, 1 - IPV6
        ipaddr              string      ip address
        prefixlen           number 1    ip prefix length

    TEST - Testing message.
        list                strings     list
        hash                dictionary  dict
*/

#define NETDISC_MSG_VERSION                 1.0

#define NETDISC_MSG_OS_ADD                  1
#define NETDISC_MSG_OS_DEL                  2
#define NETDISC_MSG_MAN_ADD                 3
#define NETDISC_MSG_MAN_EXCL                4
#define NETDISC_MSG_REV_ADD                 5
#define NETDISC_MSG_REV_EXCL                6
#define NETDISC_MSG_TEST                    7

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
typedef struct _netdisc_msg_t netdisc_msg_t;

//  @interface
//  Create a new netdisc_msg
netdisc_msg_t *
    netdisc_msg_new (int id);

//  Destroy the netdisc_msg
void
    netdisc_msg_destroy (netdisc_msg_t **self_p);

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

//  Encode the OS_ADD 
zmsg_t *
    netdisc_msg_encode_os_add (
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);

//  Encode the OS_DEL 
zmsg_t *
    netdisc_msg_encode_os_del (
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

//  Encode the MAN_EXCL 
zmsg_t *
    netdisc_msg_encode_man_excl (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the REV_ADD 
zmsg_t *
    netdisc_msg_encode_rev_add (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the REV_EXCL 
zmsg_t *
    netdisc_msg_encode_rev_excl (
        byte ipver,
        const char *ipaddr,
        byte prefixlen);

//  Encode the TEST 
zmsg_t *
    netdisc_msg_encode_test (
        zlist_t *list,
        zhash_t *hash);


//  Send the OS_ADD to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_os_add (void *output,
        const char *name,
        byte ipver,
        const char *ipaddr,
        byte prefixlen,
        const char *mac);
    
//  Send the OS_DEL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_os_del (void *output,
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
    
//  Send the MAN_EXCL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_man_excl (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the REV_ADD to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_rev_add (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the REV_EXCL to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_rev_excl (void *output,
        byte ipver,
        const char *ipaddr,
        byte prefixlen);
    
//  Send the TEST to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    netdisc_msg_send_test (void *output,
        zlist_t *list,
        zhash_t *hash);
    
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

//  Get/set the list field
zlist_t *
    netdisc_msg_list (netdisc_msg_t *self);
//  Get the list field and transfer ownership to caller
zlist_t *
    netdisc_msg_get_list (netdisc_msg_t *self);
//  Set the list field, transferring ownership from caller
void
    netdisc_msg_set_list (netdisc_msg_t *self, zlist_t **list_p);

//  Iterate through the list field, and append a list value
const char *
    netdisc_msg_list_first (netdisc_msg_t *self);
const char *
    netdisc_msg_list_next (netdisc_msg_t *self);
void
    netdisc_msg_list_append (netdisc_msg_t *self, const char *format, ...);
size_t
    netdisc_msg_list_size (netdisc_msg_t *self);

//  Get/set the hash field
zhash_t *
    netdisc_msg_hash (netdisc_msg_t *self);
//  Get the hash field and transfer ownership to caller
zhash_t *
    netdisc_msg_get_hash (netdisc_msg_t *self);
//  Set the hash field, transferring ownership from caller
void
    netdisc_msg_set_hash (netdisc_msg_t *self, zhash_t **hash_p);
    
//  Get/set a value in the hash dictionary
const char *
    netdisc_msg_hash_string (netdisc_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    netdisc_msg_hash_number (netdisc_msg_t *self,
        const char *key, uint64_t default_value);
void
    netdisc_msg_hash_insert (netdisc_msg_t *self,
        const char *key, const char *format, ...);
size_t
    netdisc_msg_hash_size (netdisc_msg_t *self);

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
