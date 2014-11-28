/*  =========================================================================
    nmap_msg - nmap scan results
    
    Codec header for nmap_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: nmap_msg.xml, or
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

#ifndef __NMAP_MSG_H_INCLUDED__
#define __NMAP_MSG_H_INCLUDED__

/*  These are the nmap_msg messages:

    SCAN_COMMAND - Command for scanning
        type                string      Type of a scan - default list scan, device scan, ...
        headers             hash        Aditional parameters for scanning, not used right now
        args                strings     Arguments for scanning, usually list of ip addresses

    LIST_SCAN - Results of a list scan, this parses 'host' element with a limited set of fields, see ELEMENT host in nmap DTD
        addr                string      IP address
        reason              string      Reason string (syn-ack, echo-reply, ...), see portreason.cc:reason_map_type
        hostnames           hash        dictionary of hostname : type, where type is a type of hostname (user, PTR)

    DEV_SCAN - Results of a device scan, this parses 'host' element. It is a high-level container, which encapsulates
port scan results and os scan one (if enabled).
        host_state          number 1     Status of a host (up|down|unknown|skipped)
        reason              string      Reason string (syn-ack, echo-reply, ...), see portreason.cc:reason_map_type
        addresses           hash        dictionary of address : vendor, where vendor is valid only for mac addresses
        hostnames           hash        dictionary of hostname : type, where type is a type of hostname (user, PTR)
        ports               frame       List of port_scan results. List of 'port_scan' messages
        os                  frame       List of os_scan results. List of 'os_scan' messages
        scripts             frame       List of script results. List of 'script' messages

    PORT_SCAN - Results of a device scan, this parses 'port' + 'port/state' elements. It encapsulates informations
about service(s) running on port and result of scan scripts if any.
        protocol            string      Name of protocol
        portid              number 2    Port number (1-65535), uint16_t
        port_state          number 1    Port status (open, filtered, unfiltered, closed, open|filtered, closed|filtered, unknown), see nmap.cc:statenum2str
        reason              string      Reason string (syn-ack, echo-reply, ...), see portreason.cc:reason_map_type
        reason_ttl          number 1    reason_ttl
        reason_ip           string      reason_ip (optional)
        service_name        string      Name of a service
        service_conf        number 1    confidence in result's correcntess (0-10)
        service_method      number 1    How nmap got it (table|probed)
        service_version     string      version (optional)
        service_product     string      product (optional)
        service_extrainfo   string      product (optional)
        service_tunnel      number 1    tunnel (ssl) (optional)
        service_proto       number 1    proto  (rpc) (optional)
        service_rpcnum      number 4    rpcnum (optional)
        service_lowver      number 4    lowver (optional)
        service_highver     number 4    highver (optional)
        service_hostname    string      hostname (optional)
        service_ostype      string      ostype (optional)
        service_devicetype  string      devicetype (optional)
        service_servicefp   string      servicefp (optional)
        service_cpes        strings     List of CPE(optional)
        scripts             frame       List of script results. Each frame is nmap_msg_script_t. Encapsulated list of 'script' messages

    SCRIPT - Output of NSE script
        script_id           string      Name of a script (like ssh-hostkeys)
        data                chunk       Data - raw XML

    OS_SCAN - Operating system scan results (if available)
        portused            frame       List of portused results. Encapsulated list of 'portused' messages.
        osmatch             frame       List of osmatch results. Encapsulated list of 'osmatch' messages
        osfingerprints      strings     List of OS fingerprints

    PORTUSED - Content of element 'os/portused', see nmap dtd ELEMENT portused
        port_state          number 1    Port status (open, filtered, unfiltered, closed, open|filtered, closed|filtered, unknown), see nmap.cc:statenum2str
        proto               string      Protocol name
        portid              number 2    Port number (1-65535), uint16_t

    OSMATCH - Content of element 'os/osmatch', see nmap dtd ELEMENT portused
        name                string      OS name
        accuracy            number 1    Match accuracy, uint16_t
        osclass             frame       List of osclass results. Encapsulated list of 'osclass' messages
        line                number 4    ?

    OSCLASS - Content of element 'os/osmatch/osclass', see nmap dtd ELEMENT portused
        vendor              string      Vendor name
        osgen               string      OS generation (optional)
        type                string      OS type (optional)
        osaccuracy          string      accuracy
        osfamily            string      osfamily
        cpes                strings     List of CPE

    SCAN_ERROR - Nmap fails with an error
        return_code         number 2    Return code of a program
        args                strings     Program command line
        stderr              longstr     Program's standard error output
*/

#define NMAP_MSG_VERSION                    1.0

#define NMAP_MSG_SCAN_COMMAND               1
#define NMAP_MSG_LIST_SCAN                  2
#define NMAP_MSG_DEV_SCAN                   3
#define NMAP_MSG_PORT_SCAN                  4
#define NMAP_MSG_SCRIPT                     6
#define NMAP_MSG_OS_SCAN                    7
#define NMAP_MSG_PORTUSED                   8
#define NMAP_MSG_OSMATCH                    9
#define NMAP_MSG_OSCLASS                    10
#define NMAP_MSG_SCAN_ERROR                 42

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef NMAP_MSG_T_DEFINED
typedef struct _nmap_msg_t nmap_msg_t;
#define NMAP_MSG_T_DEFINED
#endif

//  @interface
//  Create a new nmap_msg
nmap_msg_t *
    nmap_msg_new (int id);

//  Destroy the nmap_msg
void
    nmap_msg_destroy (nmap_msg_t **self_p);

//  Parse a zmsg_t and decides whether it is nmap_msg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
    is_nmap_msg (zmsg_t *msg_p);

//  Parse a nmap_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
nmap_msg_t *
    nmap_msg_decode (zmsg_t **msg_p);

//  Encode nmap_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    nmap_msg_encode (nmap_msg_t **self_p);

//  Receive and parse a nmap_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
nmap_msg_t *
    nmap_msg_recv (void *input);

//  Receive and parse a nmap_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
nmap_msg_t *
    nmap_msg_recv_nowait (void *input);

//  Send the nmap_msg to the output, and destroy it
int
    nmap_msg_send (nmap_msg_t **self_p, void *output);

//  Send the nmap_msg to the output, and do not destroy it
int
    nmap_msg_send_again (nmap_msg_t *self, void *output);

//  Encode the SCAN_COMMAND 
zmsg_t *
    nmap_msg_encode_scan_command (
        const char *type,
        zhash_t *headers,
        zlist_t *args);

//  Encode the LIST_SCAN 
zmsg_t *
    nmap_msg_encode_list_scan (
        const char *addr,
        const char *reason,
        zhash_t *hostnames);

//  Encode the DEV_SCAN 
zmsg_t *
    nmap_msg_encode_dev_scan (
        byte host_state,
        const char *reason,
        zhash_t *addresses,
        zhash_t *hostnames,
        zframe_t *ports,
        zframe_t *os,
        zframe_t *scripts);

//  Encode the PORT_SCAN 
zmsg_t *
    nmap_msg_encode_port_scan (
        const char *protocol,
        uint16_t portid,
        byte port_state,
        const char *reason,
        byte reason_ttl,
        const char *reason_ip,
        const char *service_name,
        byte service_conf,
        byte service_method,
        const char *service_version,
        const char *service_product,
        const char *service_extrainfo,
        byte service_tunnel,
        byte service_proto,
        uint32_t service_rpcnum,
        uint32_t service_lowver,
        uint32_t service_highver,
        const char *service_hostname,
        const char *service_ostype,
        const char *service_devicetype,
        const char *service_servicefp,
        zlist_t *service_cpes,
        zframe_t *scripts);

//  Encode the SCRIPT 
zmsg_t *
    nmap_msg_encode_script (
        const char *script_id,
        zchunk_t *data);

//  Encode the OS_SCAN 
zmsg_t *
    nmap_msg_encode_os_scan (
        zframe_t *portused,
        zframe_t *osmatch,
        zlist_t *osfingerprints);

//  Encode the PORTUSED 
zmsg_t *
    nmap_msg_encode_portused (
        byte port_state,
        const char *proto,
        uint16_t portid);

//  Encode the OSMATCH 
zmsg_t *
    nmap_msg_encode_osmatch (
        const char *name,
        byte accuracy,
        zframe_t *osclass,
        uint32_t line);

//  Encode the OSCLASS 
zmsg_t *
    nmap_msg_encode_osclass (
        const char *vendor,
        const char *osgen,
        const char *type,
        const char *osaccuracy,
        const char *osfamily,
        zlist_t *cpes);

//  Encode the SCAN_ERROR 
zmsg_t *
    nmap_msg_encode_scan_error (
        uint16_t return_code,
        zlist_t *args,
        const char *stderr);


//  Send the SCAN_COMMAND to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_scan_command (void *output,
        const char *type,
        zhash_t *headers,
        zlist_t *args);
    
//  Send the LIST_SCAN to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_list_scan (void *output,
        const char *addr,
        const char *reason,
        zhash_t *hostnames);
    
//  Send the DEV_SCAN to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_dev_scan (void *output,
        byte host_state,
        const char *reason,
        zhash_t *addresses,
        zhash_t *hostnames,
        zframe_t *ports,
        zframe_t *os,
        zframe_t *scripts);
    
//  Send the PORT_SCAN to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_port_scan (void *output,
        const char *protocol,
        uint16_t portid,
        byte port_state,
        const char *reason,
        byte reason_ttl,
        const char *reason_ip,
        const char *service_name,
        byte service_conf,
        byte service_method,
        const char *service_version,
        const char *service_product,
        const char *service_extrainfo,
        byte service_tunnel,
        byte service_proto,
        uint32_t service_rpcnum,
        uint32_t service_lowver,
        uint32_t service_highver,
        const char *service_hostname,
        const char *service_ostype,
        const char *service_devicetype,
        const char *service_servicefp,
        zlist_t *service_cpes,
        zframe_t *scripts);
    
//  Send the SCRIPT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_script (void *output,
        const char *script_id,
        zchunk_t *data);
    
//  Send the OS_SCAN to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_os_scan (void *output,
        zframe_t *portused,
        zframe_t *osmatch,
        zlist_t *osfingerprints);
    
//  Send the PORTUSED to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_portused (void *output,
        byte port_state,
        const char *proto,
        uint16_t portid);
    
//  Send the OSMATCH to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_osmatch (void *output,
        const char *name,
        byte accuracy,
        zframe_t *osclass,
        uint32_t line);
    
//  Send the OSCLASS to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_osclass (void *output,
        const char *vendor,
        const char *osgen,
        const char *type,
        const char *osaccuracy,
        const char *osfamily,
        zlist_t *cpes);
    
//  Send the SCAN_ERROR to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    nmap_msg_send_scan_error (void *output,
        uint16_t return_code,
        zlist_t *args,
        const char *stderr);
    
//  Duplicate the nmap_msg message
nmap_msg_t *
    nmap_msg_dup (nmap_msg_t *self);

//  Print contents of message to stdout
void
    nmap_msg_print (nmap_msg_t *self);

//  Get/set the message routing id
zframe_t *
    nmap_msg_routing_id (nmap_msg_t *self);
void
    nmap_msg_set_routing_id (nmap_msg_t *self, zframe_t *routing_id);

//  Get the nmap_msg id and printable command
int
    nmap_msg_id (nmap_msg_t *self);
void
    nmap_msg_set_id (nmap_msg_t *self, int id);
const char *
    nmap_msg_command (nmap_msg_t *self);

//  Get/set the type field
const char *
    nmap_msg_type (nmap_msg_t *self);
void
    nmap_msg_set_type (nmap_msg_t *self, const char *format, ...);

//  Get/set the headers field
zhash_t *
    nmap_msg_headers (nmap_msg_t *self);
//  Get the headers field and transfer ownership to caller
zhash_t *
    nmap_msg_get_headers (nmap_msg_t *self);
//  Set the headers field, transferring ownership from caller
void
    nmap_msg_set_headers (nmap_msg_t *self, zhash_t **headers_p);
    
//  Get/set a value in the headers dictionary
const char *
    nmap_msg_headers_string (nmap_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    nmap_msg_headers_number (nmap_msg_t *self,
        const char *key, uint64_t default_value);
void
    nmap_msg_headers_insert (nmap_msg_t *self,
        const char *key, const char *format, ...);
size_t
    nmap_msg_headers_size (nmap_msg_t *self);

//  Get/set the args field
zlist_t *
    nmap_msg_args (nmap_msg_t *self);
//  Get the args field and transfer ownership to caller
zlist_t *
    nmap_msg_get_args (nmap_msg_t *self);
//  Set the args field, transferring ownership from caller
void
    nmap_msg_set_args (nmap_msg_t *self, zlist_t **args_p);

//  Iterate through the args field, and append a args value
const char *
    nmap_msg_args_first (nmap_msg_t *self);
const char *
    nmap_msg_args_next (nmap_msg_t *self);
void
    nmap_msg_args_append (nmap_msg_t *self, const char *format, ...);
size_t
    nmap_msg_args_size (nmap_msg_t *self);

//  Get/set the addr field
const char *
    nmap_msg_addr (nmap_msg_t *self);
void
    nmap_msg_set_addr (nmap_msg_t *self, const char *format, ...);

//  Get/set the reason field
const char *
    nmap_msg_reason (nmap_msg_t *self);
void
    nmap_msg_set_reason (nmap_msg_t *self, const char *format, ...);

//  Get/set the hostnames field
zhash_t *
    nmap_msg_hostnames (nmap_msg_t *self);
//  Get the hostnames field and transfer ownership to caller
zhash_t *
    nmap_msg_get_hostnames (nmap_msg_t *self);
//  Set the hostnames field, transferring ownership from caller
void
    nmap_msg_set_hostnames (nmap_msg_t *self, zhash_t **hostnames_p);
    
//  Get/set a value in the hostnames dictionary
const char *
    nmap_msg_hostnames_string (nmap_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    nmap_msg_hostnames_number (nmap_msg_t *self,
        const char *key, uint64_t default_value);
void
    nmap_msg_hostnames_insert (nmap_msg_t *self,
        const char *key, const char *format, ...);
size_t
    nmap_msg_hostnames_size (nmap_msg_t *self);

//  Get/set the host_state field
byte
    nmap_msg_host_state (nmap_msg_t *self);
void
    nmap_msg_set_host_state (nmap_msg_t *self, byte host_state);

//  Get/set the addresses field
zhash_t *
    nmap_msg_addresses (nmap_msg_t *self);
//  Get the addresses field and transfer ownership to caller
zhash_t *
    nmap_msg_get_addresses (nmap_msg_t *self);
//  Set the addresses field, transferring ownership from caller
void
    nmap_msg_set_addresses (nmap_msg_t *self, zhash_t **addresses_p);
    
//  Get/set a value in the addresses dictionary
const char *
    nmap_msg_addresses_string (nmap_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    nmap_msg_addresses_number (nmap_msg_t *self,
        const char *key, uint64_t default_value);
void
    nmap_msg_addresses_insert (nmap_msg_t *self,
        const char *key, const char *format, ...);
size_t
    nmap_msg_addresses_size (nmap_msg_t *self);

//  Get a copy of the ports field
zframe_t *
    nmap_msg_ports (nmap_msg_t *self);
//  Get the ports field and transfer ownership to caller
zframe_t *
    nmap_msg_get_ports (nmap_msg_t *self);
//  Set the ports field, transferring ownership from caller
void
    nmap_msg_set_ports (nmap_msg_t *self, zframe_t **frame_p);

//  Get a copy of the os field
zframe_t *
    nmap_msg_os (nmap_msg_t *self);
//  Get the os field and transfer ownership to caller
zframe_t *
    nmap_msg_get_os (nmap_msg_t *self);
//  Set the os field, transferring ownership from caller
void
    nmap_msg_set_os (nmap_msg_t *self, zframe_t **frame_p);

//  Get a copy of the scripts field
zframe_t *
    nmap_msg_scripts (nmap_msg_t *self);
//  Get the scripts field and transfer ownership to caller
zframe_t *
    nmap_msg_get_scripts (nmap_msg_t *self);
//  Set the scripts field, transferring ownership from caller
void
    nmap_msg_set_scripts (nmap_msg_t *self, zframe_t **frame_p);

//  Get/set the protocol field
const char *
    nmap_msg_protocol (nmap_msg_t *self);
void
    nmap_msg_set_protocol (nmap_msg_t *self, const char *format, ...);

//  Get/set the portid field
uint16_t
    nmap_msg_portid (nmap_msg_t *self);
void
    nmap_msg_set_portid (nmap_msg_t *self, uint16_t portid);

//  Get/set the port_state field
byte
    nmap_msg_port_state (nmap_msg_t *self);
void
    nmap_msg_set_port_state (nmap_msg_t *self, byte port_state);

//  Get/set the reason_ttl field
byte
    nmap_msg_reason_ttl (nmap_msg_t *self);
void
    nmap_msg_set_reason_ttl (nmap_msg_t *self, byte reason_ttl);

//  Get/set the reason_ip field
const char *
    nmap_msg_reason_ip (nmap_msg_t *self);
void
    nmap_msg_set_reason_ip (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_name field
const char *
    nmap_msg_service_name (nmap_msg_t *self);
void
    nmap_msg_set_service_name (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_conf field
byte
    nmap_msg_service_conf (nmap_msg_t *self);
void
    nmap_msg_set_service_conf (nmap_msg_t *self, byte service_conf);

//  Get/set the service_method field
byte
    nmap_msg_service_method (nmap_msg_t *self);
void
    nmap_msg_set_service_method (nmap_msg_t *self, byte service_method);

//  Get/set the service_version field
const char *
    nmap_msg_service_version (nmap_msg_t *self);
void
    nmap_msg_set_service_version (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_product field
const char *
    nmap_msg_service_product (nmap_msg_t *self);
void
    nmap_msg_set_service_product (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_extrainfo field
const char *
    nmap_msg_service_extrainfo (nmap_msg_t *self);
void
    nmap_msg_set_service_extrainfo (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_tunnel field
byte
    nmap_msg_service_tunnel (nmap_msg_t *self);
void
    nmap_msg_set_service_tunnel (nmap_msg_t *self, byte service_tunnel);

//  Get/set the service_proto field
byte
    nmap_msg_service_proto (nmap_msg_t *self);
void
    nmap_msg_set_service_proto (nmap_msg_t *self, byte service_proto);

//  Get/set the service_rpcnum field
uint32_t
    nmap_msg_service_rpcnum (nmap_msg_t *self);
void
    nmap_msg_set_service_rpcnum (nmap_msg_t *self, uint32_t service_rpcnum);

//  Get/set the service_lowver field
uint32_t
    nmap_msg_service_lowver (nmap_msg_t *self);
void
    nmap_msg_set_service_lowver (nmap_msg_t *self, uint32_t service_lowver);

//  Get/set the service_highver field
uint32_t
    nmap_msg_service_highver (nmap_msg_t *self);
void
    nmap_msg_set_service_highver (nmap_msg_t *self, uint32_t service_highver);

//  Get/set the service_hostname field
const char *
    nmap_msg_service_hostname (nmap_msg_t *self);
void
    nmap_msg_set_service_hostname (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_ostype field
const char *
    nmap_msg_service_ostype (nmap_msg_t *self);
void
    nmap_msg_set_service_ostype (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_devicetype field
const char *
    nmap_msg_service_devicetype (nmap_msg_t *self);
void
    nmap_msg_set_service_devicetype (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_servicefp field
const char *
    nmap_msg_service_servicefp (nmap_msg_t *self);
void
    nmap_msg_set_service_servicefp (nmap_msg_t *self, const char *format, ...);

//  Get/set the service_cpes field
zlist_t *
    nmap_msg_service_cpes (nmap_msg_t *self);
//  Get the service_cpes field and transfer ownership to caller
zlist_t *
    nmap_msg_get_service_cpes (nmap_msg_t *self);
//  Set the service_cpes field, transferring ownership from caller
void
    nmap_msg_set_service_cpes (nmap_msg_t *self, zlist_t **service_cpes_p);

//  Iterate through the service_cpes field, and append a service_cpes value
const char *
    nmap_msg_service_cpes_first (nmap_msg_t *self);
const char *
    nmap_msg_service_cpes_next (nmap_msg_t *self);
void
    nmap_msg_service_cpes_append (nmap_msg_t *self, const char *format, ...);
size_t
    nmap_msg_service_cpes_size (nmap_msg_t *self);

//  Get/set the script_id field
const char *
    nmap_msg_script_id (nmap_msg_t *self);
void
    nmap_msg_set_script_id (nmap_msg_t *self, const char *format, ...);

//  Get a copy of the data field
zchunk_t *
    nmap_msg_data (nmap_msg_t *self);
//  Get the data field and transfer ownership to caller
zchunk_t *
    nmap_msg_get_data (nmap_msg_t *self);
//  Set the data field, transferring ownership from caller
void
    nmap_msg_set_data (nmap_msg_t *self, zchunk_t **chunk_p);

//  Get a copy of the portused field
zframe_t *
    nmap_msg_portused (nmap_msg_t *self);
//  Get the portused field and transfer ownership to caller
zframe_t *
    nmap_msg_get_portused (nmap_msg_t *self);
//  Set the portused field, transferring ownership from caller
void
    nmap_msg_set_portused (nmap_msg_t *self, zframe_t **frame_p);

//  Get a copy of the osmatch field
zframe_t *
    nmap_msg_osmatch (nmap_msg_t *self);
//  Get the osmatch field and transfer ownership to caller
zframe_t *
    nmap_msg_get_osmatch (nmap_msg_t *self);
//  Set the osmatch field, transferring ownership from caller
void
    nmap_msg_set_osmatch (nmap_msg_t *self, zframe_t **frame_p);

//  Get/set the osfingerprints field
zlist_t *
    nmap_msg_osfingerprints (nmap_msg_t *self);
//  Get the osfingerprints field and transfer ownership to caller
zlist_t *
    nmap_msg_get_osfingerprints (nmap_msg_t *self);
//  Set the osfingerprints field, transferring ownership from caller
void
    nmap_msg_set_osfingerprints (nmap_msg_t *self, zlist_t **osfingerprints_p);

//  Iterate through the osfingerprints field, and append a osfingerprints value
const char *
    nmap_msg_osfingerprints_first (nmap_msg_t *self);
const char *
    nmap_msg_osfingerprints_next (nmap_msg_t *self);
void
    nmap_msg_osfingerprints_append (nmap_msg_t *self, const char *format, ...);
size_t
    nmap_msg_osfingerprints_size (nmap_msg_t *self);

//  Get/set the proto field
const char *
    nmap_msg_proto (nmap_msg_t *self);
void
    nmap_msg_set_proto (nmap_msg_t *self, const char *format, ...);

//  Get/set the name field
const char *
    nmap_msg_name (nmap_msg_t *self);
void
    nmap_msg_set_name (nmap_msg_t *self, const char *format, ...);

//  Get/set the accuracy field
byte
    nmap_msg_accuracy (nmap_msg_t *self);
void
    nmap_msg_set_accuracy (nmap_msg_t *self, byte accuracy);

//  Get a copy of the osclass field
zframe_t *
    nmap_msg_osclass (nmap_msg_t *self);
//  Get the osclass field and transfer ownership to caller
zframe_t *
    nmap_msg_get_osclass (nmap_msg_t *self);
//  Set the osclass field, transferring ownership from caller
void
    nmap_msg_set_osclass (nmap_msg_t *self, zframe_t **frame_p);

//  Get/set the line field
uint32_t
    nmap_msg_line (nmap_msg_t *self);
void
    nmap_msg_set_line (nmap_msg_t *self, uint32_t line);

//  Get/set the vendor field
const char *
    nmap_msg_vendor (nmap_msg_t *self);
void
    nmap_msg_set_vendor (nmap_msg_t *self, const char *format, ...);

//  Get/set the osgen field
const char *
    nmap_msg_osgen (nmap_msg_t *self);
void
    nmap_msg_set_osgen (nmap_msg_t *self, const char *format, ...);

//  Get/set the osaccuracy field
const char *
    nmap_msg_osaccuracy (nmap_msg_t *self);
void
    nmap_msg_set_osaccuracy (nmap_msg_t *self, const char *format, ...);

//  Get/set the osfamily field
const char *
    nmap_msg_osfamily (nmap_msg_t *self);
void
    nmap_msg_set_osfamily (nmap_msg_t *self, const char *format, ...);

//  Get/set the cpes field
zlist_t *
    nmap_msg_cpes (nmap_msg_t *self);
//  Get the cpes field and transfer ownership to caller
zlist_t *
    nmap_msg_get_cpes (nmap_msg_t *self);
//  Set the cpes field, transferring ownership from caller
void
    nmap_msg_set_cpes (nmap_msg_t *self, zlist_t **cpes_p);

//  Iterate through the cpes field, and append a cpes value
const char *
    nmap_msg_cpes_first (nmap_msg_t *self);
const char *
    nmap_msg_cpes_next (nmap_msg_t *self);
void
    nmap_msg_cpes_append (nmap_msg_t *self, const char *format, ...);
size_t
    nmap_msg_cpes_size (nmap_msg_t *self);

//  Get/set the return_code field
uint16_t
    nmap_msg_return_code (nmap_msg_t *self);
void
    nmap_msg_set_return_code (nmap_msg_t *self, uint16_t return_code);

//  Get/set the stderr field
const char *
    nmap_msg_stderr (nmap_msg_t *self);
void
    nmap_msg_set_stderr (nmap_msg_t *self, const char *format, ...);

//  Self test of this class
int
    nmap_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define nmap_msg_dump       nmap_msg_print

#ifdef __cplusplus
}
#endif

#endif
