/*  =========================================================================
    nmap_msg - nmap scan results

    Codec class for nmap_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: nmap_msg.xml, or
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
    nmap_msg - nmap scan results
@discuss
@end
*/

#include "./nmap_msg.h"

//  Structure of our class

struct _nmap_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  nmap_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *type;                         //  Type of a scan - default list scan, device scan, ...
    zhash_t *headers;                   //  Aditional parameters for scanning, not used right now
    size_t headers_bytes;               //  Size of dictionary content
    zlist_t *args;                      //  Arguments for scanning, usually list of ip addresses
    char *addr;                         //  IP address
    byte host_state;                    //   Status of a host (up|down|unknown|skipped)
    char *reason;                       //  Reason string (syn-ack, echo-reply, ...), see portreason.cc:reason_map_type
    zhash_t *hostnames;                 //  dictionary of hostname : type, where type is a type of hostname (user, PTR)
    size_t hostnames_bytes;             //  Size of dictionary content
    zhash_t *addresses;                 //  dictionary of address : vendor, where vendor is valid only for mac addresses
    size_t addresses_bytes;             //  Size of dictionary content
    zframe_t *ports;                    //  List of port_scan results. List of 'port_scan' messages
    zframe_t *os;                       //  List of os_scan results. List of 'os_scan' messages
    zframe_t *scripts;                  //  List of script results. List of 'script' messages
    char *protocol;                     //  Name of protocol
    uint16_t portid;                    //  Port number (1-65535), uint16_t
    byte port_state;                    //  Port status (open, filtered, unfiltered, closed, open|filtered, closed|filtered, unknown), see nmap.cc:statenum2str
    byte reason_ttl;                    //  reason_ttl
    char *reason_ip;                    //  reason_ip (optional)
    zframe_t *service;                  //  Service detected on a port (optional, nmap_msg_service_scan_t). Encapsulated list of 'service_scan' messages
    char *name;                         //  Name of a service
    byte conf;                          //  confidence in result's correcntess (0-10)
    byte method;                        //  How nmap got it (table|probed)
    char *version;                      //  version (optional)
    char *product;                      //  product (optional)
    char *extrainfo;                    //  product (optional)
    byte tunnel;                        //  tunnel (ssl) (optional)
    byte service_proto;                 //  proto  (rpc) (optional)
    uint32_t rpcnum;                    //  rpcnum (optional)
    uint32_t lowver;                    //  lowver (optional)
    uint32_t highver;                   //  highver (optional)
    char *hostname;                     //  hostname (optional)
    char *ostype;                       //  ostype (optional)
    char *devicetype;                   //  devicetype (optional)
    char *servicefp;                    //  servicefp (optional)
    char *cpe;                          //  cpe (optional)
    char *script_id;                    //  Name of a script (like ssh-hostkeys)
    zchunk_t *data;                     //  Data - raw XML
    zframe_t *portused;                 //  List of portused results. Encapsulated list of 'portused' messages.
    zframe_t *osmatch;                  //  List of osmatch results. Encapsulated list of 'osmatch' messages
    zlist_t *osfingerprints;            //  List of OS fingerprints
    char *proto;                        //  Protocol name
    byte accuracy;                      //  Match accuracy, uint16_t
    zframe_t *osclass;                  //  List of osclass results. Encapsulated list of 'osclass' messages
    char *vendor;                       //  Vendor name
    char *osgen;                        //  OS generation (optional)
    char *osaccuracy;                   //  accuracy
    char *osfamily;                     //  osfamily
    zlist_t *cpes;                      //  List of CPE
    uint16_t return_code;               //  Return code of a program
    char *stderr;                       //  Program's standard error output
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
//  Create a new nmap_msg

nmap_msg_t *
nmap_msg_new (int id)
{
    nmap_msg_t *self = (nmap_msg_t *) zmalloc (sizeof (nmap_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the nmap_msg

void
nmap_msg_destroy (nmap_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        nmap_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->type);
        zhash_destroy (&self->headers);
        if (self->args)
            zlist_destroy (&self->args);
        free (self->addr);
        free (self->reason);
        zhash_destroy (&self->hostnames);
        zhash_destroy (&self->addresses);
        zframe_destroy (&self->ports);
        zframe_destroy (&self->os);
        zframe_destroy (&self->scripts);
        free (self->protocol);
        free (self->reason_ip);
        zframe_destroy (&self->service);
        free (self->name);
        free (self->version);
        free (self->product);
        free (self->extrainfo);
        free (self->hostname);
        free (self->ostype);
        free (self->devicetype);
        free (self->servicefp);
        free (self->cpe);
        free (self->script_id);
        zchunk_destroy (&self->data);
        zframe_destroy (&self->portused);
        zframe_destroy (&self->osmatch);
        if (self->osfingerprints)
            zlist_destroy (&self->osfingerprints);
        free (self->proto);
        zframe_destroy (&self->osclass);
        free (self->vendor);
        free (self->osgen);
        free (self->osaccuracy);
        free (self->osfamily);
        if (self->cpes)
            zlist_destroy (&self->cpes);
        free (self->stderr);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a nmap_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

nmap_msg_t *
nmap_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    nmap_msg_t *self = nmap_msg_new (0);
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
        case NMAP_MSG_SCAN_COMMAND:
            GET_STRING (self->type);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->headers = zhash_new ();
                zhash_autofree (self->headers);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->headers, key, value);
                    free (key);
                    free (value);
                }
            }
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->args = zlist_new ();
                zlist_autofree (self->args);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->args, string);
                    free (string);
                }
            }
            break;

        case NMAP_MSG_LIST_SCAN:
            GET_STRING (self->addr);
            GET_NUMBER1 (self->host_state);
            GET_STRING (self->reason);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->hostnames = zhash_new ();
                zhash_autofree (self->hostnames);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->hostnames, key, value);
                    free (key);
                    free (value);
                }
            }
            break;

        case NMAP_MSG_DEV_SCAN:
            GET_NUMBER1 (self->host_state);
            GET_STRING (self->reason);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->addresses = zhash_new ();
                zhash_autofree (self->addresses);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->addresses, key, value);
                    free (key);
                    free (value);
                }
            }
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                self->hostnames = zhash_new ();
                zhash_autofree (self->hostnames);
                while (hash_size--) {
                    char *key, *value;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->hostnames, key, value);
                    free (key);
                    free (value);
                }
            }
            {
                //  Get next frame, leave current untouched
                zframe_t *ports = zmsg_pop (msg);
                if (!ports)
                    goto malformed;
                self->ports = ports;
            }
            {
                //  Get next frame, leave current untouched
                zframe_t *os = zmsg_pop (msg);
                if (!os)
                    goto malformed;
                self->os = os;
            }
            {
                //  Get next frame, leave current untouched
                zframe_t *scripts = zmsg_pop (msg);
                if (!scripts)
                    goto malformed;
                self->scripts = scripts;
            }
            break;

        case NMAP_MSG_PORT_SCAN:
            GET_STRING (self->protocol);
            GET_NUMBER2 (self->portid);
            GET_NUMBER1 (self->port_state);
            GET_STRING (self->reason);
            GET_NUMBER1 (self->reason_ttl);
            GET_STRING (self->reason_ip);
            {
                //  Get next frame, leave current untouched
                zframe_t *service = zmsg_pop (msg);
                if (!service)
                    goto malformed;
                self->service = service;
            }
            {
                //  Get next frame, leave current untouched
                zframe_t *scripts = zmsg_pop (msg);
                if (!scripts)
                    goto malformed;
                self->scripts = scripts;
            }
            break;

        case NMAP_MSG_SERVICE_SCAN:
            GET_STRING (self->name);
            GET_NUMBER1 (self->conf);
            GET_NUMBER1 (self->method);
            GET_STRING (self->version);
            GET_STRING (self->product);
            GET_STRING (self->extrainfo);
            GET_NUMBER1 (self->tunnel);
            GET_NUMBER1 (self->service_proto);
            GET_NUMBER4 (self->rpcnum);
            GET_NUMBER4 (self->lowver);
            GET_NUMBER4 (self->highver);
            GET_STRING (self->hostname);
            GET_STRING (self->ostype);
            GET_STRING (self->devicetype);
            GET_STRING (self->servicefp);
            GET_STRING (self->cpe);
            break;

        case NMAP_MSG_SCRIPT:
            GET_STRING (self->script_id);
            {
                size_t chunk_size;
                GET_NUMBER4 (chunk_size);
                if (self->needle + chunk_size > (self->ceiling))
                    goto malformed;
                self->data = zchunk_new (self->needle, chunk_size);
                self->needle += chunk_size;
            }
            break;

        case NMAP_MSG_OS_SCAN:
            {
                //  Get next frame, leave current untouched
                zframe_t *portused = zmsg_pop (msg);
                if (!portused)
                    goto malformed;
                self->portused = portused;
            }
            {
                //  Get next frame, leave current untouched
                zframe_t *osmatch = zmsg_pop (msg);
                if (!osmatch)
                    goto malformed;
                self->osmatch = osmatch;
            }
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->osfingerprints = zlist_new ();
                zlist_autofree (self->osfingerprints);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->osfingerprints, string);
                    free (string);
                }
            }
            break;

        case NMAP_MSG_PORTUSED:
            GET_NUMBER1 (self->port_state);
            GET_STRING (self->proto);
            GET_NUMBER2 (self->portid);
            break;

        case NMAP_MSG_OSMATCH:
            GET_STRING (self->name);
            GET_NUMBER1 (self->accuracy);
            {
                //  Get next frame, leave current untouched
                zframe_t *osclass = zmsg_pop (msg);
                if (!osclass)
                    goto malformed;
                self->osclass = osclass;
            }
            break;

        case NMAP_MSG_OSCLASS:
            GET_STRING (self->vendor);
            GET_STRING (self->osgen);
            GET_STRING (self->type);
            GET_STRING (self->osaccuracy);
            GET_STRING (self->osfamily);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->cpes = zlist_new ();
                zlist_autofree (self->cpes);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->cpes, string);
                    free (string);
                }
            }
            break;

        case NMAP_MSG_SCAN_ERROR:
            GET_NUMBER2 (self->return_code);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->args = zlist_new ();
                zlist_autofree (self->args);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->args, string);
                    free (string);
                }
            }
            GET_LONGSTR (self->stderr);
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
        nmap_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode nmap_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
nmap_msg_encode (nmap_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    nmap_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case NMAP_MSG_SCAN_COMMAND:
            //  type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->type)
                frame_size += strlen (self->type);
            //  headers is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->headers) {
                self->headers_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->headers);
                while (item) {
                    self->headers_bytes += 1 + strlen ((const char *) zhash_cursor (self->headers));
                    self->headers_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->headers);
                }
            }
            frame_size += self->headers_bytes;
            //  args is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->args) {
                //  Add up size of list contents
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    frame_size += 4 + strlen (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            break;
            
        case NMAP_MSG_LIST_SCAN:
            //  addr is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->addr)
                frame_size += strlen (self->addr);
            //  host_state is a 1-byte integer
            frame_size += 1;
            //  reason is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason)
                frame_size += strlen (self->reason);
            //  hostnames is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->hostnames) {
                self->hostnames_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    self->hostnames_bytes += 1 + strlen ((const char *) zhash_cursor (self->hostnames));
                    self->hostnames_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            frame_size += self->hostnames_bytes;
            break;
            
        case NMAP_MSG_DEV_SCAN:
            //  host_state is a 1-byte integer
            frame_size += 1;
            //  reason is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason)
                frame_size += strlen (self->reason);
            //  addresses is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->addresses) {
                self->addresses_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->addresses);
                while (item) {
                    self->addresses_bytes += 1 + strlen ((const char *) zhash_cursor (self->addresses));
                    self->addresses_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->addresses);
                }
            }
            frame_size += self->addresses_bytes;
            //  hostnames is an array of key=value strings
            frame_size += 4;    //  Size is 4 octets
            if (self->hostnames) {
                self->hostnames_bytes = 0;
                //  Add up size of dictionary contents
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    self->hostnames_bytes += 1 + strlen ((const char *) zhash_cursor (self->hostnames));
                    self->hostnames_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            frame_size += self->hostnames_bytes;
            break;
            
        case NMAP_MSG_PORT_SCAN:
            //  protocol is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->protocol)
                frame_size += strlen (self->protocol);
            //  portid is a 2-byte integer
            frame_size += 2;
            //  port_state is a 1-byte integer
            frame_size += 1;
            //  reason is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason)
                frame_size += strlen (self->reason);
            //  reason_ttl is a 1-byte integer
            frame_size += 1;
            //  reason_ip is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason_ip)
                frame_size += strlen (self->reason_ip);
            break;
            
        case NMAP_MSG_SERVICE_SCAN:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  conf is a 1-byte integer
            frame_size += 1;
            //  method is a 1-byte integer
            frame_size += 1;
            //  version is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->version)
                frame_size += strlen (self->version);
            //  product is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->product)
                frame_size += strlen (self->product);
            //  extrainfo is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->extrainfo)
                frame_size += strlen (self->extrainfo);
            //  tunnel is a 1-byte integer
            frame_size += 1;
            //  service_proto is a 1-byte integer
            frame_size += 1;
            //  rpcnum is a 4-byte integer
            frame_size += 4;
            //  lowver is a 4-byte integer
            frame_size += 4;
            //  highver is a 4-byte integer
            frame_size += 4;
            //  hostname is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->hostname)
                frame_size += strlen (self->hostname);
            //  ostype is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->ostype)
                frame_size += strlen (self->ostype);
            //  devicetype is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->devicetype)
                frame_size += strlen (self->devicetype);
            //  servicefp is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->servicefp)
                frame_size += strlen (self->servicefp);
            //  cpe is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->cpe)
                frame_size += strlen (self->cpe);
            break;
            
        case NMAP_MSG_SCRIPT:
            //  script_id is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->script_id)
                frame_size += strlen (self->script_id);
            //  data is a chunk with 4-byte length
            frame_size += 4;
            if (self->data)
                frame_size += zchunk_size (self->data);
            break;
            
        case NMAP_MSG_OS_SCAN:
            //  osfingerprints is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->osfingerprints) {
                //  Add up size of list contents
                char *osfingerprints = (char *) zlist_first (self->osfingerprints);
                while (osfingerprints) {
                    frame_size += 4 + strlen (osfingerprints);
                    osfingerprints = (char *) zlist_next (self->osfingerprints);
                }
            }
            break;
            
        case NMAP_MSG_PORTUSED:
            //  port_state is a 1-byte integer
            frame_size += 1;
            //  proto is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->proto)
                frame_size += strlen (self->proto);
            //  portid is a 2-byte integer
            frame_size += 2;
            break;
            
        case NMAP_MSG_OSMATCH:
            //  name is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->name)
                frame_size += strlen (self->name);
            //  accuracy is a 1-byte integer
            frame_size += 1;
            break;
            
        case NMAP_MSG_OSCLASS:
            //  vendor is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->vendor)
                frame_size += strlen (self->vendor);
            //  osgen is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->osgen)
                frame_size += strlen (self->osgen);
            //  type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->type)
                frame_size += strlen (self->type);
            //  osaccuracy is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->osaccuracy)
                frame_size += strlen (self->osaccuracy);
            //  osfamily is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->osfamily)
                frame_size += strlen (self->osfamily);
            //  cpes is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->cpes) {
                //  Add up size of list contents
                char *cpes = (char *) zlist_first (self->cpes);
                while (cpes) {
                    frame_size += 4 + strlen (cpes);
                    cpes = (char *) zlist_next (self->cpes);
                }
            }
            break;
            
        case NMAP_MSG_SCAN_ERROR:
            //  return_code is a 2-byte integer
            frame_size += 2;
            //  args is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->args) {
                //  Add up size of list contents
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    frame_size += 4 + strlen (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            //  stderr is a string with 4-byte length
            frame_size += 4;
            if (self->stderr)
                frame_size += strlen (self->stderr);
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
        case NMAP_MSG_SCAN_COMMAND:
            if (self->type) {
                PUT_STRING (self->type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->headers) {
                PUT_NUMBER4 (zhash_size (self->headers));
                char *item = (char *) zhash_first (self->headers);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->headers));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->headers);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            if (self->args) {
                PUT_NUMBER4 (zlist_size (self->args));
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    PUT_LONGSTR (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

        case NMAP_MSG_LIST_SCAN:
            if (self->addr) {
                PUT_STRING (self->addr);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->host_state);
            if (self->reason) {
                PUT_STRING (self->reason);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->hostnames) {
                PUT_NUMBER4 (zhash_size (self->hostnames));
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->hostnames));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case NMAP_MSG_DEV_SCAN:
            PUT_NUMBER1 (self->host_state);
            if (self->reason) {
                PUT_STRING (self->reason);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->addresses) {
                PUT_NUMBER4 (zhash_size (self->addresses));
                char *item = (char *) zhash_first (self->addresses);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->addresses));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->addresses);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            if (self->hostnames) {
                PUT_NUMBER4 (zhash_size (self->hostnames));
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    PUT_STRING ((const char *) zhash_cursor ((zhash_t *) self->hostnames));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty dictionary
            break;

        case NMAP_MSG_PORT_SCAN:
            if (self->protocol) {
                PUT_STRING (self->protocol);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER2 (self->portid);
            PUT_NUMBER1 (self->port_state);
            if (self->reason) {
                PUT_STRING (self->reason);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->reason_ttl);
            if (self->reason_ip) {
                PUT_STRING (self->reason_ip);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case NMAP_MSG_SERVICE_SCAN:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->conf);
            PUT_NUMBER1 (self->method);
            if (self->version) {
                PUT_STRING (self->version);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->product) {
                PUT_STRING (self->product);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->extrainfo) {
                PUT_STRING (self->extrainfo);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->tunnel);
            PUT_NUMBER1 (self->service_proto);
            PUT_NUMBER4 (self->rpcnum);
            PUT_NUMBER4 (self->lowver);
            PUT_NUMBER4 (self->highver);
            if (self->hostname) {
                PUT_STRING (self->hostname);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->ostype) {
                PUT_STRING (self->ostype);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->devicetype) {
                PUT_STRING (self->devicetype);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->servicefp) {
                PUT_STRING (self->servicefp);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->cpe) {
                PUT_STRING (self->cpe);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case NMAP_MSG_SCRIPT:
            if (self->script_id) {
                PUT_STRING (self->script_id);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->data) {
                PUT_NUMBER4 (zchunk_size (self->data));
                memcpy (self->needle,
                        zchunk_data (self->data),
                        zchunk_size (self->data));
                self->needle += zchunk_size (self->data);
            }
            else
                PUT_NUMBER4 (0);    //  Empty chunk
            break;

        case NMAP_MSG_OS_SCAN:
            if (self->osfingerprints) {
                PUT_NUMBER4 (zlist_size (self->osfingerprints));
                char *osfingerprints = (char *) zlist_first (self->osfingerprints);
                while (osfingerprints) {
                    PUT_LONGSTR (osfingerprints);
                    osfingerprints = (char *) zlist_next (self->osfingerprints);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

        case NMAP_MSG_PORTUSED:
            PUT_NUMBER1 (self->port_state);
            if (self->proto) {
                PUT_STRING (self->proto);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER2 (self->portid);
            break;

        case NMAP_MSG_OSMATCH:
            if (self->name) {
                PUT_STRING (self->name);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            PUT_NUMBER1 (self->accuracy);
            break;

        case NMAP_MSG_OSCLASS:
            if (self->vendor) {
                PUT_STRING (self->vendor);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->osgen) {
                PUT_STRING (self->osgen);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->type) {
                PUT_STRING (self->type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->osaccuracy) {
                PUT_STRING (self->osaccuracy);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->osfamily) {
                PUT_STRING (self->osfamily);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->cpes) {
                PUT_NUMBER4 (zlist_size (self->cpes));
                char *cpes = (char *) zlist_first (self->cpes);
                while (cpes) {
                    PUT_LONGSTR (cpes);
                    cpes = (char *) zlist_next (self->cpes);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

        case NMAP_MSG_SCAN_ERROR:
            PUT_NUMBER2 (self->return_code);
            if (self->args) {
                PUT_NUMBER4 (zlist_size (self->args));
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    PUT_LONGSTR (args);
                    args = (char *) zlist_next (self->args);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            if (self->stderr) {
                PUT_LONGSTR (self->stderr);
            }
            else
                PUT_NUMBER4 (0);    //  Empty string
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        nmap_msg_destroy (self_p);
        return NULL;
    }
    //  Now send any frame fields, in order
    if (self->id == NMAP_MSG_DEV_SCAN) {
        //  If ports isn't set, send an empty frame
        if (!self->ports)
            self->ports = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->ports)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
        //  If os isn't set, send an empty frame
        if (!self->os)
            self->os = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->os)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
        //  If scripts isn't set, send an empty frame
        if (!self->scripts)
            self->scripts = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->scripts)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
    }
    //  Now send any frame fields, in order
    if (self->id == NMAP_MSG_PORT_SCAN) {
        //  If service isn't set, send an empty frame
        if (!self->service)
            self->service = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->service)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
        //  If scripts isn't set, send an empty frame
        if (!self->scripts)
            self->scripts = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->scripts)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
    }
    //  Now send any frame fields, in order
    if (self->id == NMAP_MSG_OS_SCAN) {
        //  If portused isn't set, send an empty frame
        if (!self->portused)
            self->portused = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->portused)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
        //  If osmatch isn't set, send an empty frame
        if (!self->osmatch)
            self->osmatch = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->osmatch)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
    }
    //  Now send any frame fields, in order
    if (self->id == NMAP_MSG_OSMATCH) {
        //  If osclass isn't set, send an empty frame
        if (!self->osclass)
            self->osclass = zframe_new (NULL, 0);
        if (zmsg_append (msg, &self->osclass)) {
            zmsg_destroy (&msg);
            nmap_msg_destroy (self_p);
            return NULL;
        }
    }
    //  Destroy nmap_msg object
    nmap_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a nmap_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

nmap_msg_t *
nmap_msg_recv (void *input)
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
    nmap_msg_t *nmap_msg = nmap_msg_decode (&msg);
    if (nmap_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        nmap_msg->routing_id = routing_id;

    return nmap_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a nmap_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

nmap_msg_t *
nmap_msg_recv_nowait (void *input)
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
    nmap_msg_t *nmap_msg = nmap_msg_decode (&msg);
    if (nmap_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        nmap_msg->routing_id = routing_id;

    return nmap_msg;
}


//  --------------------------------------------------------------------------
//  Send the nmap_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
nmap_msg_send (nmap_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    nmap_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode nmap_msg message to a single zmsg
    zmsg_t *msg = nmap_msg_encode (self_p);
    
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
//  Send the nmap_msg to the output, and do not destroy it

int
nmap_msg_send_again (nmap_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = nmap_msg_dup (self);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode SCAN_COMMAND message

zmsg_t * 
nmap_msg_encode_scan_command (
    const char *type,
    zhash_t *headers,
    zlist_t *args)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
    nmap_msg_set_type (self, type);
    zhash_t *headers_copy = zhash_dup (headers);
    nmap_msg_set_headers (self, &headers_copy);
    zlist_t *args_copy = zlist_dup (args);
    nmap_msg_set_args (self, &args_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode LIST_SCAN message

zmsg_t * 
nmap_msg_encode_list_scan (
    const char *addr,
    byte host_state,
    const char *reason,
    zhash_t *hostnames)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_LIST_SCAN);
    nmap_msg_set_addr (self, addr);
    nmap_msg_set_host_state (self, host_state);
    nmap_msg_set_reason (self, reason);
    zhash_t *hostnames_copy = zhash_dup (hostnames);
    nmap_msg_set_hostnames (self, &hostnames_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode DEV_SCAN message

zmsg_t * 
nmap_msg_encode_dev_scan (
    byte host_state,
    const char *reason,
    zhash_t *addresses,
    zhash_t *hostnames,
    zframe_t *ports,
    zframe_t *os,
    zframe_t *scripts)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_DEV_SCAN);
    nmap_msg_set_host_state (self, host_state);
    nmap_msg_set_reason (self, reason);
    zhash_t *addresses_copy = zhash_dup (addresses);
    nmap_msg_set_addresses (self, &addresses_copy);
    zhash_t *hostnames_copy = zhash_dup (hostnames);
    nmap_msg_set_hostnames (self, &hostnames_copy);
    zframe_t *ports_copy = zframe_dup (ports);
    nmap_msg_set_ports (self, &ports_copy);
    zframe_t *os_copy = zframe_dup (os);
    nmap_msg_set_os (self, &os_copy);
    zframe_t *scripts_copy = zframe_dup (scripts);
    nmap_msg_set_scripts (self, &scripts_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode PORT_SCAN message

zmsg_t * 
nmap_msg_encode_port_scan (
    const char *protocol,
    uint16_t portid,
    byte port_state,
    const char *reason,
    byte reason_ttl,
    const char *reason_ip,
    zframe_t *service,
    zframe_t *scripts)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_PORT_SCAN);
    nmap_msg_set_protocol (self, protocol);
    nmap_msg_set_portid (self, portid);
    nmap_msg_set_port_state (self, port_state);
    nmap_msg_set_reason (self, reason);
    nmap_msg_set_reason_ttl (self, reason_ttl);
    nmap_msg_set_reason_ip (self, reason_ip);
    zframe_t *service_copy = zframe_dup (service);
    nmap_msg_set_service (self, &service_copy);
    zframe_t *scripts_copy = zframe_dup (scripts);
    nmap_msg_set_scripts (self, &scripts_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode SERVICE_SCAN message

zmsg_t * 
nmap_msg_encode_service_scan (
    const char *name,
    byte conf,
    byte method,
    const char *version,
    const char *product,
    const char *extrainfo,
    byte tunnel,
    byte service_proto,
    uint32_t rpcnum,
    uint32_t lowver,
    uint32_t highver,
    const char *hostname,
    const char *ostype,
    const char *devicetype,
    const char *servicefp,
    const char *cpe)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SERVICE_SCAN);
    nmap_msg_set_name (self, name);
    nmap_msg_set_conf (self, conf);
    nmap_msg_set_method (self, method);
    nmap_msg_set_version (self, version);
    nmap_msg_set_product (self, product);
    nmap_msg_set_extrainfo (self, extrainfo);
    nmap_msg_set_tunnel (self, tunnel);
    nmap_msg_set_service_proto (self, service_proto);
    nmap_msg_set_rpcnum (self, rpcnum);
    nmap_msg_set_lowver (self, lowver);
    nmap_msg_set_highver (self, highver);
    nmap_msg_set_hostname (self, hostname);
    nmap_msg_set_ostype (self, ostype);
    nmap_msg_set_devicetype (self, devicetype);
    nmap_msg_set_servicefp (self, servicefp);
    nmap_msg_set_cpe (self, cpe);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode SCRIPT message

zmsg_t * 
nmap_msg_encode_script (
    const char *script_id,
    zchunk_t *data)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCRIPT);
    nmap_msg_set_script_id (self, script_id);
    zchunk_t *data_copy = zchunk_dup (data);
    nmap_msg_set_data (self, &data_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode OS_SCAN message

zmsg_t * 
nmap_msg_encode_os_scan (
    zframe_t *portused,
    zframe_t *osmatch,
    zlist_t *osfingerprints)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OS_SCAN);
    zframe_t *portused_copy = zframe_dup (portused);
    nmap_msg_set_portused (self, &portused_copy);
    zframe_t *osmatch_copy = zframe_dup (osmatch);
    nmap_msg_set_osmatch (self, &osmatch_copy);
    zlist_t *osfingerprints_copy = zlist_dup (osfingerprints);
    nmap_msg_set_osfingerprints (self, &osfingerprints_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode PORTUSED message

zmsg_t * 
nmap_msg_encode_portused (
    byte port_state,
    const char *proto,
    uint16_t portid)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_PORTUSED);
    nmap_msg_set_port_state (self, port_state);
    nmap_msg_set_proto (self, proto);
    nmap_msg_set_portid (self, portid);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode OSMATCH message

zmsg_t * 
nmap_msg_encode_osmatch (
    const char *name,
    byte accuracy,
    zframe_t *osclass)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OSMATCH);
    nmap_msg_set_name (self, name);
    nmap_msg_set_accuracy (self, accuracy);
    zframe_t *osclass_copy = zframe_dup (osclass);
    nmap_msg_set_osclass (self, &osclass_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode OSCLASS message

zmsg_t * 
nmap_msg_encode_osclass (
    const char *vendor,
    const char *osgen,
    const char *type,
    const char *osaccuracy,
    const char *osfamily,
    zlist_t *cpes)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OSCLASS);
    nmap_msg_set_vendor (self, vendor);
    nmap_msg_set_osgen (self, osgen);
    nmap_msg_set_type (self, type);
    nmap_msg_set_osaccuracy (self, osaccuracy);
    nmap_msg_set_osfamily (self, osfamily);
    zlist_t *cpes_copy = zlist_dup (cpes);
    nmap_msg_set_cpes (self, &cpes_copy);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode SCAN_ERROR message

zmsg_t * 
nmap_msg_encode_scan_error (
    uint16_t return_code,
    zlist_t *args,
    const char *stderr)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCAN_ERROR);
    nmap_msg_set_return_code (self, return_code);
    zlist_t *args_copy = zlist_dup (args);
    nmap_msg_set_args (self, &args_copy);
    nmap_msg_set_stderr (self, stderr);
    return nmap_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the SCAN_COMMAND to the socket in one step

int
nmap_msg_send_scan_command (
    void *output,
    const char *type,
    zhash_t *headers,
    zlist_t *args)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
    nmap_msg_set_type (self, type);
    zhash_t *headers_copy = zhash_dup (headers);
    nmap_msg_set_headers (self, &headers_copy);
    zlist_t *args_copy = zlist_dup (args);
    nmap_msg_set_args (self, &args_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the LIST_SCAN to the socket in one step

int
nmap_msg_send_list_scan (
    void *output,
    const char *addr,
    byte host_state,
    const char *reason,
    zhash_t *hostnames)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_LIST_SCAN);
    nmap_msg_set_addr (self, addr);
    nmap_msg_set_host_state (self, host_state);
    nmap_msg_set_reason (self, reason);
    zhash_t *hostnames_copy = zhash_dup (hostnames);
    nmap_msg_set_hostnames (self, &hostnames_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the DEV_SCAN to the socket in one step

int
nmap_msg_send_dev_scan (
    void *output,
    byte host_state,
    const char *reason,
    zhash_t *addresses,
    zhash_t *hostnames,
    zframe_t *ports,
    zframe_t *os,
    zframe_t *scripts)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_DEV_SCAN);
    nmap_msg_set_host_state (self, host_state);
    nmap_msg_set_reason (self, reason);
    zhash_t *addresses_copy = zhash_dup (addresses);
    nmap_msg_set_addresses (self, &addresses_copy);
    zhash_t *hostnames_copy = zhash_dup (hostnames);
    nmap_msg_set_hostnames (self, &hostnames_copy);
    zframe_t *ports_copy = zframe_dup (ports);
    nmap_msg_set_ports (self, &ports_copy);
    zframe_t *os_copy = zframe_dup (os);
    nmap_msg_set_os (self, &os_copy);
    zframe_t *scripts_copy = zframe_dup (scripts);
    nmap_msg_set_scripts (self, &scripts_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the PORT_SCAN to the socket in one step

int
nmap_msg_send_port_scan (
    void *output,
    const char *protocol,
    uint16_t portid,
    byte port_state,
    const char *reason,
    byte reason_ttl,
    const char *reason_ip,
    zframe_t *service,
    zframe_t *scripts)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_PORT_SCAN);
    nmap_msg_set_protocol (self, protocol);
    nmap_msg_set_portid (self, portid);
    nmap_msg_set_port_state (self, port_state);
    nmap_msg_set_reason (self, reason);
    nmap_msg_set_reason_ttl (self, reason_ttl);
    nmap_msg_set_reason_ip (self, reason_ip);
    zframe_t *service_copy = zframe_dup (service);
    nmap_msg_set_service (self, &service_copy);
    zframe_t *scripts_copy = zframe_dup (scripts);
    nmap_msg_set_scripts (self, &scripts_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the SERVICE_SCAN to the socket in one step

int
nmap_msg_send_service_scan (
    void *output,
    const char *name,
    byte conf,
    byte method,
    const char *version,
    const char *product,
    const char *extrainfo,
    byte tunnel,
    byte service_proto,
    uint32_t rpcnum,
    uint32_t lowver,
    uint32_t highver,
    const char *hostname,
    const char *ostype,
    const char *devicetype,
    const char *servicefp,
    const char *cpe)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SERVICE_SCAN);
    nmap_msg_set_name (self, name);
    nmap_msg_set_conf (self, conf);
    nmap_msg_set_method (self, method);
    nmap_msg_set_version (self, version);
    nmap_msg_set_product (self, product);
    nmap_msg_set_extrainfo (self, extrainfo);
    nmap_msg_set_tunnel (self, tunnel);
    nmap_msg_set_service_proto (self, service_proto);
    nmap_msg_set_rpcnum (self, rpcnum);
    nmap_msg_set_lowver (self, lowver);
    nmap_msg_set_highver (self, highver);
    nmap_msg_set_hostname (self, hostname);
    nmap_msg_set_ostype (self, ostype);
    nmap_msg_set_devicetype (self, devicetype);
    nmap_msg_set_servicefp (self, servicefp);
    nmap_msg_set_cpe (self, cpe);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the SCRIPT to the socket in one step

int
nmap_msg_send_script (
    void *output,
    const char *script_id,
    zchunk_t *data)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCRIPT);
    nmap_msg_set_script_id (self, script_id);
    zchunk_t *data_copy = zchunk_dup (data);
    nmap_msg_set_data (self, &data_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the OS_SCAN to the socket in one step

int
nmap_msg_send_os_scan (
    void *output,
    zframe_t *portused,
    zframe_t *osmatch,
    zlist_t *osfingerprints)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OS_SCAN);
    zframe_t *portused_copy = zframe_dup (portused);
    nmap_msg_set_portused (self, &portused_copy);
    zframe_t *osmatch_copy = zframe_dup (osmatch);
    nmap_msg_set_osmatch (self, &osmatch_copy);
    zlist_t *osfingerprints_copy = zlist_dup (osfingerprints);
    nmap_msg_set_osfingerprints (self, &osfingerprints_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the PORTUSED to the socket in one step

int
nmap_msg_send_portused (
    void *output,
    byte port_state,
    const char *proto,
    uint16_t portid)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_PORTUSED);
    nmap_msg_set_port_state (self, port_state);
    nmap_msg_set_proto (self, proto);
    nmap_msg_set_portid (self, portid);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the OSMATCH to the socket in one step

int
nmap_msg_send_osmatch (
    void *output,
    const char *name,
    byte accuracy,
    zframe_t *osclass)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OSMATCH);
    nmap_msg_set_name (self, name);
    nmap_msg_set_accuracy (self, accuracy);
    zframe_t *osclass_copy = zframe_dup (osclass);
    nmap_msg_set_osclass (self, &osclass_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the OSCLASS to the socket in one step

int
nmap_msg_send_osclass (
    void *output,
    const char *vendor,
    const char *osgen,
    const char *type,
    const char *osaccuracy,
    const char *osfamily,
    zlist_t *cpes)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_OSCLASS);
    nmap_msg_set_vendor (self, vendor);
    nmap_msg_set_osgen (self, osgen);
    nmap_msg_set_type (self, type);
    nmap_msg_set_osaccuracy (self, osaccuracy);
    nmap_msg_set_osfamily (self, osfamily);
    zlist_t *cpes_copy = zlist_dup (cpes);
    nmap_msg_set_cpes (self, &cpes_copy);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the SCAN_ERROR to the socket in one step

int
nmap_msg_send_scan_error (
    void *output,
    uint16_t return_code,
    zlist_t *args,
    const char *stderr)
{
    nmap_msg_t *self = nmap_msg_new (NMAP_MSG_SCAN_ERROR);
    nmap_msg_set_return_code (self, return_code);
    zlist_t *args_copy = zlist_dup (args);
    nmap_msg_set_args (self, &args_copy);
    nmap_msg_set_stderr (self, stderr);
    return nmap_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the nmap_msg message

nmap_msg_t *
nmap_msg_dup (nmap_msg_t *self)
{
    if (!self)
        return NULL;
        
    nmap_msg_t *copy = nmap_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case NMAP_MSG_SCAN_COMMAND:
            copy->type = self->type? strdup (self->type): NULL;
            copy->headers = self->headers? zhash_dup (self->headers): NULL;
            copy->args = self->args? zlist_dup (self->args): NULL;
            break;

        case NMAP_MSG_LIST_SCAN:
            copy->addr = self->addr? strdup (self->addr): NULL;
            copy->host_state = self->host_state;
            copy->reason = self->reason? strdup (self->reason): NULL;
            copy->hostnames = self->hostnames? zhash_dup (self->hostnames): NULL;
            break;

        case NMAP_MSG_DEV_SCAN:
            copy->host_state = self->host_state;
            copy->reason = self->reason? strdup (self->reason): NULL;
            copy->addresses = self->addresses? zhash_dup (self->addresses): NULL;
            copy->hostnames = self->hostnames? zhash_dup (self->hostnames): NULL;
            copy->ports = self->ports? zframe_dup (self->ports): NULL;
            copy->os = self->os? zframe_dup (self->os): NULL;
            copy->scripts = self->scripts? zframe_dup (self->scripts): NULL;
            break;

        case NMAP_MSG_PORT_SCAN:
            copy->protocol = self->protocol? strdup (self->protocol): NULL;
            copy->portid = self->portid;
            copy->port_state = self->port_state;
            copy->reason = self->reason? strdup (self->reason): NULL;
            copy->reason_ttl = self->reason_ttl;
            copy->reason_ip = self->reason_ip? strdup (self->reason_ip): NULL;
            copy->service = self->service? zframe_dup (self->service): NULL;
            copy->scripts = self->scripts? zframe_dup (self->scripts): NULL;
            break;

        case NMAP_MSG_SERVICE_SCAN:
            copy->name = self->name? strdup (self->name): NULL;
            copy->conf = self->conf;
            copy->method = self->method;
            copy->version = self->version? strdup (self->version): NULL;
            copy->product = self->product? strdup (self->product): NULL;
            copy->extrainfo = self->extrainfo? strdup (self->extrainfo): NULL;
            copy->tunnel = self->tunnel;
            copy->service_proto = self->service_proto;
            copy->rpcnum = self->rpcnum;
            copy->lowver = self->lowver;
            copy->highver = self->highver;
            copy->hostname = self->hostname? strdup (self->hostname): NULL;
            copy->ostype = self->ostype? strdup (self->ostype): NULL;
            copy->devicetype = self->devicetype? strdup (self->devicetype): NULL;
            copy->servicefp = self->servicefp? strdup (self->servicefp): NULL;
            copy->cpe = self->cpe? strdup (self->cpe): NULL;
            break;

        case NMAP_MSG_SCRIPT:
            copy->script_id = self->script_id? strdup (self->script_id): NULL;
            copy->data = self->data? zchunk_dup (self->data): NULL;
            break;

        case NMAP_MSG_OS_SCAN:
            copy->portused = self->portused? zframe_dup (self->portused): NULL;
            copy->osmatch = self->osmatch? zframe_dup (self->osmatch): NULL;
            copy->osfingerprints = self->osfingerprints? zlist_dup (self->osfingerprints): NULL;
            break;

        case NMAP_MSG_PORTUSED:
            copy->port_state = self->port_state;
            copy->proto = self->proto? strdup (self->proto): NULL;
            copy->portid = self->portid;
            break;

        case NMAP_MSG_OSMATCH:
            copy->name = self->name? strdup (self->name): NULL;
            copy->accuracy = self->accuracy;
            copy->osclass = self->osclass? zframe_dup (self->osclass): NULL;
            break;

        case NMAP_MSG_OSCLASS:
            copy->vendor = self->vendor? strdup (self->vendor): NULL;
            copy->osgen = self->osgen? strdup (self->osgen): NULL;
            copy->type = self->type? strdup (self->type): NULL;
            copy->osaccuracy = self->osaccuracy? strdup (self->osaccuracy): NULL;
            copy->osfamily = self->osfamily? strdup (self->osfamily): NULL;
            copy->cpes = self->cpes? zlist_dup (self->cpes): NULL;
            break;

        case NMAP_MSG_SCAN_ERROR:
            copy->return_code = self->return_code;
            copy->args = self->args? zlist_dup (self->args): NULL;
            copy->stderr = self->stderr? strdup (self->stderr): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
nmap_msg_print (nmap_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case NMAP_MSG_SCAN_COMMAND:
            zsys_debug ("NMAP_MSG_SCAN_COMMAND:");
            if (self->type)
                zsys_debug ("    type='%s'", self->type);
            else
                zsys_debug ("    type=");
            zsys_debug ("    headers=");
            if (self->headers) {
                char *item = (char *) zhash_first (self->headers);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->headers), item);
                    item = (char *) zhash_next (self->headers);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    args=");
            if (self->args) {
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    zsys_debug ("        '%s'", args);
                    args = (char *) zlist_next (self->args);
                }
            }
            break;
            
        case NMAP_MSG_LIST_SCAN:
            zsys_debug ("NMAP_MSG_LIST_SCAN:");
            if (self->addr)
                zsys_debug ("    addr='%s'", self->addr);
            else
                zsys_debug ("    addr=");
            zsys_debug ("    host_state=%ld", (long) self->host_state);
            if (self->reason)
                zsys_debug ("    reason='%s'", self->reason);
            else
                zsys_debug ("    reason=");
            zsys_debug ("    hostnames=");
            if (self->hostnames) {
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->hostnames), item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;
            
        case NMAP_MSG_DEV_SCAN:
            zsys_debug ("NMAP_MSG_DEV_SCAN:");
            zsys_debug ("    host_state=%ld", (long) self->host_state);
            if (self->reason)
                zsys_debug ("    reason='%s'", self->reason);
            else
                zsys_debug ("    reason=");
            zsys_debug ("    addresses=");
            if (self->addresses) {
                char *item = (char *) zhash_first (self->addresses);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->addresses), item);
                    item = (char *) zhash_next (self->addresses);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    hostnames=");
            if (self->hostnames) {
                char *item = (char *) zhash_first (self->hostnames);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->hostnames), item);
                    item = (char *) zhash_next (self->hostnames);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    ports=");
            if (self->ports)
                zframe_print (self->ports, NULL);
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    os=");
            if (self->os)
                zframe_print (self->os, NULL);
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    scripts=");
            if (self->scripts)
                zframe_print (self->scripts, NULL);
            else
                zsys_debug ("(NULL)");
            break;
            
        case NMAP_MSG_PORT_SCAN:
            zsys_debug ("NMAP_MSG_PORT_SCAN:");
            if (self->protocol)
                zsys_debug ("    protocol='%s'", self->protocol);
            else
                zsys_debug ("    protocol=");
            zsys_debug ("    portid=%ld", (long) self->portid);
            zsys_debug ("    port_state=%ld", (long) self->port_state);
            if (self->reason)
                zsys_debug ("    reason='%s'", self->reason);
            else
                zsys_debug ("    reason=");
            zsys_debug ("    reason_ttl=%ld", (long) self->reason_ttl);
            if (self->reason_ip)
                zsys_debug ("    reason_ip='%s'", self->reason_ip);
            else
                zsys_debug ("    reason_ip=");
            zsys_debug ("    service=");
            if (self->service)
                zframe_print (self->service, NULL);
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    scripts=");
            if (self->scripts)
                zframe_print (self->scripts, NULL);
            else
                zsys_debug ("(NULL)");
            break;
            
        case NMAP_MSG_SERVICE_SCAN:
            zsys_debug ("NMAP_MSG_SERVICE_SCAN:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    conf=%ld", (long) self->conf);
            zsys_debug ("    method=%ld", (long) self->method);
            if (self->version)
                zsys_debug ("    version='%s'", self->version);
            else
                zsys_debug ("    version=");
            if (self->product)
                zsys_debug ("    product='%s'", self->product);
            else
                zsys_debug ("    product=");
            if (self->extrainfo)
                zsys_debug ("    extrainfo='%s'", self->extrainfo);
            else
                zsys_debug ("    extrainfo=");
            zsys_debug ("    tunnel=%ld", (long) self->tunnel);
            zsys_debug ("    service_proto=%ld", (long) self->service_proto);
            zsys_debug ("    rpcnum=%ld", (long) self->rpcnum);
            zsys_debug ("    lowver=%ld", (long) self->lowver);
            zsys_debug ("    highver=%ld", (long) self->highver);
            if (self->hostname)
                zsys_debug ("    hostname='%s'", self->hostname);
            else
                zsys_debug ("    hostname=");
            if (self->ostype)
                zsys_debug ("    ostype='%s'", self->ostype);
            else
                zsys_debug ("    ostype=");
            if (self->devicetype)
                zsys_debug ("    devicetype='%s'", self->devicetype);
            else
                zsys_debug ("    devicetype=");
            if (self->servicefp)
                zsys_debug ("    servicefp='%s'", self->servicefp);
            else
                zsys_debug ("    servicefp=");
            if (self->cpe)
                zsys_debug ("    cpe='%s'", self->cpe);
            else
                zsys_debug ("    cpe=");
            break;
            
        case NMAP_MSG_SCRIPT:
            zsys_debug ("NMAP_MSG_SCRIPT:");
            if (self->script_id)
                zsys_debug ("    script_id='%s'", self->script_id);
            else
                zsys_debug ("    script_id=");
            zsys_debug ("    data=[ ... ]");
            break;
            
        case NMAP_MSG_OS_SCAN:
            zsys_debug ("NMAP_MSG_OS_SCAN:");
            zsys_debug ("    portused=");
            if (self->portused)
                zframe_print (self->portused, NULL);
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    osmatch=");
            if (self->osmatch)
                zframe_print (self->osmatch, NULL);
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    osfingerprints=");
            if (self->osfingerprints) {
                char *osfingerprints = (char *) zlist_first (self->osfingerprints);
                while (osfingerprints) {
                    zsys_debug ("        '%s'", osfingerprints);
                    osfingerprints = (char *) zlist_next (self->osfingerprints);
                }
            }
            break;
            
        case NMAP_MSG_PORTUSED:
            zsys_debug ("NMAP_MSG_PORTUSED:");
            zsys_debug ("    port_state=%ld", (long) self->port_state);
            if (self->proto)
                zsys_debug ("    proto='%s'", self->proto);
            else
                zsys_debug ("    proto=");
            zsys_debug ("    portid=%ld", (long) self->portid);
            break;
            
        case NMAP_MSG_OSMATCH:
            zsys_debug ("NMAP_MSG_OSMATCH:");
            if (self->name)
                zsys_debug ("    name='%s'", self->name);
            else
                zsys_debug ("    name=");
            zsys_debug ("    accuracy=%ld", (long) self->accuracy);
            zsys_debug ("    osclass=");
            if (self->osclass)
                zframe_print (self->osclass, NULL);
            else
                zsys_debug ("(NULL)");
            break;
            
        case NMAP_MSG_OSCLASS:
            zsys_debug ("NMAP_MSG_OSCLASS:");
            if (self->vendor)
                zsys_debug ("    vendor='%s'", self->vendor);
            else
                zsys_debug ("    vendor=");
            if (self->osgen)
                zsys_debug ("    osgen='%s'", self->osgen);
            else
                zsys_debug ("    osgen=");
            if (self->type)
                zsys_debug ("    type='%s'", self->type);
            else
                zsys_debug ("    type=");
            if (self->osaccuracy)
                zsys_debug ("    osaccuracy='%s'", self->osaccuracy);
            else
                zsys_debug ("    osaccuracy=");
            if (self->osfamily)
                zsys_debug ("    osfamily='%s'", self->osfamily);
            else
                zsys_debug ("    osfamily=");
            zsys_debug ("    cpes=");
            if (self->cpes) {
                char *cpes = (char *) zlist_first (self->cpes);
                while (cpes) {
                    zsys_debug ("        '%s'", cpes);
                    cpes = (char *) zlist_next (self->cpes);
                }
            }
            break;
            
        case NMAP_MSG_SCAN_ERROR:
            zsys_debug ("NMAP_MSG_SCAN_ERROR:");
            zsys_debug ("    return_code=%ld", (long) self->return_code);
            zsys_debug ("    args=");
            if (self->args) {
                char *args = (char *) zlist_first (self->args);
                while (args) {
                    zsys_debug ("        '%s'", args);
                    args = (char *) zlist_next (self->args);
                }
            }
            if (self->stderr)
                zsys_debug ("    stderr='%s'", self->stderr);
            else
                zsys_debug ("    stderr=");
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
nmap_msg_routing_id (nmap_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
nmap_msg_set_routing_id (nmap_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the nmap_msg id

int
nmap_msg_id (nmap_msg_t *self)
{
    assert (self);
    return self->id;
}

void
nmap_msg_set_id (nmap_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
nmap_msg_command (nmap_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case NMAP_MSG_SCAN_COMMAND:
            return ("SCAN_COMMAND");
            break;
        case NMAP_MSG_LIST_SCAN:
            return ("LIST_SCAN");
            break;
        case NMAP_MSG_DEV_SCAN:
            return ("DEV_SCAN");
            break;
        case NMAP_MSG_PORT_SCAN:
            return ("PORT_SCAN");
            break;
        case NMAP_MSG_SERVICE_SCAN:
            return ("SERVICE_SCAN");
            break;
        case NMAP_MSG_SCRIPT:
            return ("SCRIPT");
            break;
        case NMAP_MSG_OS_SCAN:
            return ("OS_SCAN");
            break;
        case NMAP_MSG_PORTUSED:
            return ("PORTUSED");
            break;
        case NMAP_MSG_OSMATCH:
            return ("OSMATCH");
            break;
        case NMAP_MSG_OSCLASS:
            return ("OSCLASS");
            break;
        case NMAP_MSG_SCAN_ERROR:
            return ("SCAN_ERROR");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the type field

const char *
nmap_msg_type (nmap_msg_t *self)
{
    assert (self);
    return self->type;
}

void
nmap_msg_set_type (nmap_msg_t *self, const char *format, ...)
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
//  Get the headers field without transferring ownership

zhash_t *
nmap_msg_headers (nmap_msg_t *self)
{
    assert (self);
    return self->headers;
}

//  Get the headers field and transfer ownership to caller

zhash_t *
nmap_msg_get_headers (nmap_msg_t *self)
{
    zhash_t *headers = self->headers;
    self->headers = NULL;
    return headers;
}

//  Set the headers field, transferring ownership from caller

void
nmap_msg_set_headers (nmap_msg_t *self, zhash_t **headers_p)
{
    assert (self);
    assert (headers_p);
    zhash_destroy (&self->headers);
    self->headers = *headers_p;
    *headers_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the headers dictionary

const char *
nmap_msg_headers_string (nmap_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->headers)
        value = (const char *) (zhash_lookup (self->headers, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
nmap_msg_headers_number (nmap_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->headers)
        string = (char *) (zhash_lookup (self->headers, key));
    if (string)
        value = atol (string);

    return value;
}

void
nmap_msg_headers_insert (nmap_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->headers) {
        self->headers = zhash_new ();
        zhash_autofree (self->headers);
    }
    zhash_update (self->headers, key, string);
    free (string);
}

size_t
nmap_msg_headers_size (nmap_msg_t *self)
{
    return zhash_size (self->headers);
}


//  --------------------------------------------------------------------------
//  Get the args field, without transferring ownership

zlist_t *
nmap_msg_args (nmap_msg_t *self)
{
    assert (self);
    return self->args;
}

//  Get the args field and transfer ownership to caller

zlist_t *
nmap_msg_get_args (nmap_msg_t *self)
{
    assert (self);
    zlist_t *args = self->args;
    self->args = NULL;
    return args;
}

//  Set the args field, transferring ownership from caller

void
nmap_msg_set_args (nmap_msg_t *self, zlist_t **args_p)
{
    assert (self);
    assert (args_p);
    zlist_destroy (&self->args);
    self->args = *args_p;
    *args_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the args field, and append a args value

const char *
nmap_msg_args_first (nmap_msg_t *self)
{
    assert (self);
    if (self->args)
        return (char *) (zlist_first (self->args));
    else
        return NULL;
}

const char *
nmap_msg_args_next (nmap_msg_t *self)
{
    assert (self);
    if (self->args)
        return (char *) (zlist_next (self->args));
    else
        return NULL;
}

void
nmap_msg_args_append (nmap_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->args) {
        self->args = zlist_new ();
        zlist_autofree (self->args);
    }
    zlist_append (self->args, string);
    free (string);
}

size_t
nmap_msg_args_size (nmap_msg_t *self)
{
    return zlist_size (self->args);
}


//  --------------------------------------------------------------------------
//  Get/set the addr field

const char *
nmap_msg_addr (nmap_msg_t *self)
{
    assert (self);
    return self->addr;
}

void
nmap_msg_set_addr (nmap_msg_t *self, const char *format, ...)
{
    //  Format addr from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->addr);
    self->addr = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the host_state field

byte
nmap_msg_host_state (nmap_msg_t *self)
{
    assert (self);
    return self->host_state;
}

void
nmap_msg_set_host_state (nmap_msg_t *self, byte host_state)
{
    assert (self);
    self->host_state = host_state;
}


//  --------------------------------------------------------------------------
//  Get/set the reason field

const char *
nmap_msg_reason (nmap_msg_t *self)
{
    assert (self);
    return self->reason;
}

void
nmap_msg_set_reason (nmap_msg_t *self, const char *format, ...)
{
    //  Format reason from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->reason);
    self->reason = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the hostnames field without transferring ownership

zhash_t *
nmap_msg_hostnames (nmap_msg_t *self)
{
    assert (self);
    return self->hostnames;
}

//  Get the hostnames field and transfer ownership to caller

zhash_t *
nmap_msg_get_hostnames (nmap_msg_t *self)
{
    zhash_t *hostnames = self->hostnames;
    self->hostnames = NULL;
    return hostnames;
}

//  Set the hostnames field, transferring ownership from caller

void
nmap_msg_set_hostnames (nmap_msg_t *self, zhash_t **hostnames_p)
{
    assert (self);
    assert (hostnames_p);
    zhash_destroy (&self->hostnames);
    self->hostnames = *hostnames_p;
    *hostnames_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the hostnames dictionary

const char *
nmap_msg_hostnames_string (nmap_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->hostnames)
        value = (const char *) (zhash_lookup (self->hostnames, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
nmap_msg_hostnames_number (nmap_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->hostnames)
        string = (char *) (zhash_lookup (self->hostnames, key));
    if (string)
        value = atol (string);

    return value;
}

void
nmap_msg_hostnames_insert (nmap_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->hostnames) {
        self->hostnames = zhash_new ();
        zhash_autofree (self->hostnames);
    }
    zhash_update (self->hostnames, key, string);
    free (string);
}

size_t
nmap_msg_hostnames_size (nmap_msg_t *self)
{
    return zhash_size (self->hostnames);
}


//  --------------------------------------------------------------------------
//  Get the addresses field without transferring ownership

zhash_t *
nmap_msg_addresses (nmap_msg_t *self)
{
    assert (self);
    return self->addresses;
}

//  Get the addresses field and transfer ownership to caller

zhash_t *
nmap_msg_get_addresses (nmap_msg_t *self)
{
    zhash_t *addresses = self->addresses;
    self->addresses = NULL;
    return addresses;
}

//  Set the addresses field, transferring ownership from caller

void
nmap_msg_set_addresses (nmap_msg_t *self, zhash_t **addresses_p)
{
    assert (self);
    assert (addresses_p);
    zhash_destroy (&self->addresses);
    self->addresses = *addresses_p;
    *addresses_p = NULL;
}

//  --------------------------------------------------------------------------
//  Get/set a value in the addresses dictionary

const char *
nmap_msg_addresses_string (nmap_msg_t *self, const char *key, const char *default_value)
{
    assert (self);
    const char *value = NULL;
    if (self->addresses)
        value = (const char *) (zhash_lookup (self->addresses, key));
    if (!value)
        value = default_value;

    return value;
}

uint64_t
nmap_msg_addresses_number (nmap_msg_t *self, const char *key, uint64_t default_value)
{
    assert (self);
    uint64_t value = default_value;
    char *string = NULL;
    if (self->addresses)
        string = (char *) (zhash_lookup (self->addresses, key));
    if (string)
        value = atol (string);

    return value;
}

void
nmap_msg_addresses_insert (nmap_msg_t *self, const char *key, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Store string in hash table
    if (!self->addresses) {
        self->addresses = zhash_new ();
        zhash_autofree (self->addresses);
    }
    zhash_update (self->addresses, key, string);
    free (string);
}

size_t
nmap_msg_addresses_size (nmap_msg_t *self)
{
    return zhash_size (self->addresses);
}


//  --------------------------------------------------------------------------
//  Get the ports field without transferring ownership

zframe_t *
nmap_msg_ports (nmap_msg_t *self)
{
    assert (self);
    return self->ports;
}

//  Get the ports field and transfer ownership to caller

zframe_t *
nmap_msg_get_ports (nmap_msg_t *self)
{
    zframe_t *ports = self->ports;
    self->ports = NULL;
    return ports;
}

//  Set the ports field, transferring ownership from caller

void
nmap_msg_set_ports (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->ports);
    self->ports = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the os field without transferring ownership

zframe_t *
nmap_msg_os (nmap_msg_t *self)
{
    assert (self);
    return self->os;
}

//  Get the os field and transfer ownership to caller

zframe_t *
nmap_msg_get_os (nmap_msg_t *self)
{
    zframe_t *os = self->os;
    self->os = NULL;
    return os;
}

//  Set the os field, transferring ownership from caller

void
nmap_msg_set_os (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->os);
    self->os = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the scripts field without transferring ownership

zframe_t *
nmap_msg_scripts (nmap_msg_t *self)
{
    assert (self);
    return self->scripts;
}

//  Get the scripts field and transfer ownership to caller

zframe_t *
nmap_msg_get_scripts (nmap_msg_t *self)
{
    zframe_t *scripts = self->scripts;
    self->scripts = NULL;
    return scripts;
}

//  Set the scripts field, transferring ownership from caller

void
nmap_msg_set_scripts (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->scripts);
    self->scripts = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the protocol field

const char *
nmap_msg_protocol (nmap_msg_t *self)
{
    assert (self);
    return self->protocol;
}

void
nmap_msg_set_protocol (nmap_msg_t *self, const char *format, ...)
{
    //  Format protocol from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->protocol);
    self->protocol = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the portid field

uint16_t
nmap_msg_portid (nmap_msg_t *self)
{
    assert (self);
    return self->portid;
}

void
nmap_msg_set_portid (nmap_msg_t *self, uint16_t portid)
{
    assert (self);
    self->portid = portid;
}


//  --------------------------------------------------------------------------
//  Get/set the port_state field

byte
nmap_msg_port_state (nmap_msg_t *self)
{
    assert (self);
    return self->port_state;
}

void
nmap_msg_set_port_state (nmap_msg_t *self, byte port_state)
{
    assert (self);
    self->port_state = port_state;
}


//  --------------------------------------------------------------------------
//  Get/set the reason_ttl field

byte
nmap_msg_reason_ttl (nmap_msg_t *self)
{
    assert (self);
    return self->reason_ttl;
}

void
nmap_msg_set_reason_ttl (nmap_msg_t *self, byte reason_ttl)
{
    assert (self);
    self->reason_ttl = reason_ttl;
}


//  --------------------------------------------------------------------------
//  Get/set the reason_ip field

const char *
nmap_msg_reason_ip (nmap_msg_t *self)
{
    assert (self);
    return self->reason_ip;
}

void
nmap_msg_set_reason_ip (nmap_msg_t *self, const char *format, ...)
{
    //  Format reason_ip from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->reason_ip);
    self->reason_ip = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the service field without transferring ownership

zframe_t *
nmap_msg_service (nmap_msg_t *self)
{
    assert (self);
    return self->service;
}

//  Get the service field and transfer ownership to caller

zframe_t *
nmap_msg_get_service (nmap_msg_t *self)
{
    zframe_t *service = self->service;
    self->service = NULL;
    return service;
}

//  Set the service field, transferring ownership from caller

void
nmap_msg_set_service (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->service);
    self->service = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the name field

const char *
nmap_msg_name (nmap_msg_t *self)
{
    assert (self);
    return self->name;
}

void
nmap_msg_set_name (nmap_msg_t *self, const char *format, ...)
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
//  Get/set the conf field

byte
nmap_msg_conf (nmap_msg_t *self)
{
    assert (self);
    return self->conf;
}

void
nmap_msg_set_conf (nmap_msg_t *self, byte conf)
{
    assert (self);
    self->conf = conf;
}


//  --------------------------------------------------------------------------
//  Get/set the method field

byte
nmap_msg_method (nmap_msg_t *self)
{
    assert (self);
    return self->method;
}

void
nmap_msg_set_method (nmap_msg_t *self, byte method)
{
    assert (self);
    self->method = method;
}


//  --------------------------------------------------------------------------
//  Get/set the version field

const char *
nmap_msg_version (nmap_msg_t *self)
{
    assert (self);
    return self->version;
}

void
nmap_msg_set_version (nmap_msg_t *self, const char *format, ...)
{
    //  Format version from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->version);
    self->version = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the product field

const char *
nmap_msg_product (nmap_msg_t *self)
{
    assert (self);
    return self->product;
}

void
nmap_msg_set_product (nmap_msg_t *self, const char *format, ...)
{
    //  Format product from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->product);
    self->product = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the extrainfo field

const char *
nmap_msg_extrainfo (nmap_msg_t *self)
{
    assert (self);
    return self->extrainfo;
}

void
nmap_msg_set_extrainfo (nmap_msg_t *self, const char *format, ...)
{
    //  Format extrainfo from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->extrainfo);
    self->extrainfo = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the tunnel field

byte
nmap_msg_tunnel (nmap_msg_t *self)
{
    assert (self);
    return self->tunnel;
}

void
nmap_msg_set_tunnel (nmap_msg_t *self, byte tunnel)
{
    assert (self);
    self->tunnel = tunnel;
}


//  --------------------------------------------------------------------------
//  Get/set the service_proto field

byte
nmap_msg_service_proto (nmap_msg_t *self)
{
    assert (self);
    return self->service_proto;
}

void
nmap_msg_set_service_proto (nmap_msg_t *self, byte service_proto)
{
    assert (self);
    self->service_proto = service_proto;
}


//  --------------------------------------------------------------------------
//  Get/set the rpcnum field

uint32_t
nmap_msg_rpcnum (nmap_msg_t *self)
{
    assert (self);
    return self->rpcnum;
}

void
nmap_msg_set_rpcnum (nmap_msg_t *self, uint32_t rpcnum)
{
    assert (self);
    self->rpcnum = rpcnum;
}


//  --------------------------------------------------------------------------
//  Get/set the lowver field

uint32_t
nmap_msg_lowver (nmap_msg_t *self)
{
    assert (self);
    return self->lowver;
}

void
nmap_msg_set_lowver (nmap_msg_t *self, uint32_t lowver)
{
    assert (self);
    self->lowver = lowver;
}


//  --------------------------------------------------------------------------
//  Get/set the highver field

uint32_t
nmap_msg_highver (nmap_msg_t *self)
{
    assert (self);
    return self->highver;
}

void
nmap_msg_set_highver (nmap_msg_t *self, uint32_t highver)
{
    assert (self);
    self->highver = highver;
}


//  --------------------------------------------------------------------------
//  Get/set the hostname field

const char *
nmap_msg_hostname (nmap_msg_t *self)
{
    assert (self);
    return self->hostname;
}

void
nmap_msg_set_hostname (nmap_msg_t *self, const char *format, ...)
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
//  Get/set the ostype field

const char *
nmap_msg_ostype (nmap_msg_t *self)
{
    assert (self);
    return self->ostype;
}

void
nmap_msg_set_ostype (nmap_msg_t *self, const char *format, ...)
{
    //  Format ostype from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->ostype);
    self->ostype = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the devicetype field

const char *
nmap_msg_devicetype (nmap_msg_t *self)
{
    assert (self);
    return self->devicetype;
}

void
nmap_msg_set_devicetype (nmap_msg_t *self, const char *format, ...)
{
    //  Format devicetype from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->devicetype);
    self->devicetype = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the servicefp field

const char *
nmap_msg_servicefp (nmap_msg_t *self)
{
    assert (self);
    return self->servicefp;
}

void
nmap_msg_set_servicefp (nmap_msg_t *self, const char *format, ...)
{
    //  Format servicefp from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->servicefp);
    self->servicefp = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the cpe field

const char *
nmap_msg_cpe (nmap_msg_t *self)
{
    assert (self);
    return self->cpe;
}

void
nmap_msg_set_cpe (nmap_msg_t *self, const char *format, ...)
{
    //  Format cpe from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->cpe);
    self->cpe = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the script_id field

const char *
nmap_msg_script_id (nmap_msg_t *self)
{
    assert (self);
    return self->script_id;
}

void
nmap_msg_set_script_id (nmap_msg_t *self, const char *format, ...)
{
    //  Format script_id from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->script_id);
    self->script_id = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the data field without transferring ownership

zchunk_t *
nmap_msg_data (nmap_msg_t *self)
{
    assert (self);
    return self->data;
}

//  Get the data field and transfer ownership to caller

zchunk_t *
nmap_msg_get_data (nmap_msg_t *self)
{
    zchunk_t *data = self->data;
    self->data = NULL;
    return data;
}

//  Set the data field, transferring ownership from caller

void
nmap_msg_set_data (nmap_msg_t *self, zchunk_t **chunk_p)
{
    assert (self);
    assert (chunk_p);
    zchunk_destroy (&self->data);
    self->data = *chunk_p;
    *chunk_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the portused field without transferring ownership

zframe_t *
nmap_msg_portused (nmap_msg_t *self)
{
    assert (self);
    return self->portused;
}

//  Get the portused field and transfer ownership to caller

zframe_t *
nmap_msg_get_portused (nmap_msg_t *self)
{
    zframe_t *portused = self->portused;
    self->portused = NULL;
    return portused;
}

//  Set the portused field, transferring ownership from caller

void
nmap_msg_set_portused (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->portused);
    self->portused = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the osmatch field without transferring ownership

zframe_t *
nmap_msg_osmatch (nmap_msg_t *self)
{
    assert (self);
    return self->osmatch;
}

//  Get the osmatch field and transfer ownership to caller

zframe_t *
nmap_msg_get_osmatch (nmap_msg_t *self)
{
    zframe_t *osmatch = self->osmatch;
    self->osmatch = NULL;
    return osmatch;
}

//  Set the osmatch field, transferring ownership from caller

void
nmap_msg_set_osmatch (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->osmatch);
    self->osmatch = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get the osfingerprints field, without transferring ownership

zlist_t *
nmap_msg_osfingerprints (nmap_msg_t *self)
{
    assert (self);
    return self->osfingerprints;
}

//  Get the osfingerprints field and transfer ownership to caller

zlist_t *
nmap_msg_get_osfingerprints (nmap_msg_t *self)
{
    assert (self);
    zlist_t *osfingerprints = self->osfingerprints;
    self->osfingerprints = NULL;
    return osfingerprints;
}

//  Set the osfingerprints field, transferring ownership from caller

void
nmap_msg_set_osfingerprints (nmap_msg_t *self, zlist_t **osfingerprints_p)
{
    assert (self);
    assert (osfingerprints_p);
    zlist_destroy (&self->osfingerprints);
    self->osfingerprints = *osfingerprints_p;
    *osfingerprints_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the osfingerprints field, and append a osfingerprints value

const char *
nmap_msg_osfingerprints_first (nmap_msg_t *self)
{
    assert (self);
    if (self->osfingerprints)
        return (char *) (zlist_first (self->osfingerprints));
    else
        return NULL;
}

const char *
nmap_msg_osfingerprints_next (nmap_msg_t *self)
{
    assert (self);
    if (self->osfingerprints)
        return (char *) (zlist_next (self->osfingerprints));
    else
        return NULL;
}

void
nmap_msg_osfingerprints_append (nmap_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->osfingerprints) {
        self->osfingerprints = zlist_new ();
        zlist_autofree (self->osfingerprints);
    }
    zlist_append (self->osfingerprints, string);
    free (string);
}

size_t
nmap_msg_osfingerprints_size (nmap_msg_t *self)
{
    return zlist_size (self->osfingerprints);
}


//  --------------------------------------------------------------------------
//  Get/set the proto field

const char *
nmap_msg_proto (nmap_msg_t *self)
{
    assert (self);
    return self->proto;
}

void
nmap_msg_set_proto (nmap_msg_t *self, const char *format, ...)
{
    //  Format proto from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->proto);
    self->proto = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the accuracy field

byte
nmap_msg_accuracy (nmap_msg_t *self)
{
    assert (self);
    return self->accuracy;
}

void
nmap_msg_set_accuracy (nmap_msg_t *self, byte accuracy)
{
    assert (self);
    self->accuracy = accuracy;
}


//  --------------------------------------------------------------------------
//  Get the osclass field without transferring ownership

zframe_t *
nmap_msg_osclass (nmap_msg_t *self)
{
    assert (self);
    return self->osclass;
}

//  Get the osclass field and transfer ownership to caller

zframe_t *
nmap_msg_get_osclass (nmap_msg_t *self)
{
    zframe_t *osclass = self->osclass;
    self->osclass = NULL;
    return osclass;
}

//  Set the osclass field, transferring ownership from caller

void
nmap_msg_set_osclass (nmap_msg_t *self, zframe_t **frame_p)
{
    assert (self);
    assert (frame_p);
    zframe_destroy (&self->osclass);
    self->osclass = *frame_p;
    *frame_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the vendor field

const char *
nmap_msg_vendor (nmap_msg_t *self)
{
    assert (self);
    return self->vendor;
}

void
nmap_msg_set_vendor (nmap_msg_t *self, const char *format, ...)
{
    //  Format vendor from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->vendor);
    self->vendor = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the osgen field

const char *
nmap_msg_osgen (nmap_msg_t *self)
{
    assert (self);
    return self->osgen;
}

void
nmap_msg_set_osgen (nmap_msg_t *self, const char *format, ...)
{
    //  Format osgen from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->osgen);
    self->osgen = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the osaccuracy field

const char *
nmap_msg_osaccuracy (nmap_msg_t *self)
{
    assert (self);
    return self->osaccuracy;
}

void
nmap_msg_set_osaccuracy (nmap_msg_t *self, const char *format, ...)
{
    //  Format osaccuracy from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->osaccuracy);
    self->osaccuracy = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the osfamily field

const char *
nmap_msg_osfamily (nmap_msg_t *self)
{
    assert (self);
    return self->osfamily;
}

void
nmap_msg_set_osfamily (nmap_msg_t *self, const char *format, ...)
{
    //  Format osfamily from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->osfamily);
    self->osfamily = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the cpes field, without transferring ownership

zlist_t *
nmap_msg_cpes (nmap_msg_t *self)
{
    assert (self);
    return self->cpes;
}

//  Get the cpes field and transfer ownership to caller

zlist_t *
nmap_msg_get_cpes (nmap_msg_t *self)
{
    assert (self);
    zlist_t *cpes = self->cpes;
    self->cpes = NULL;
    return cpes;
}

//  Set the cpes field, transferring ownership from caller

void
nmap_msg_set_cpes (nmap_msg_t *self, zlist_t **cpes_p)
{
    assert (self);
    assert (cpes_p);
    zlist_destroy (&self->cpes);
    self->cpes = *cpes_p;
    *cpes_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the cpes field, and append a cpes value

const char *
nmap_msg_cpes_first (nmap_msg_t *self)
{
    assert (self);
    if (self->cpes)
        return (char *) (zlist_first (self->cpes));
    else
        return NULL;
}

const char *
nmap_msg_cpes_next (nmap_msg_t *self)
{
    assert (self);
    if (self->cpes)
        return (char *) (zlist_next (self->cpes));
    else
        return NULL;
}

void
nmap_msg_cpes_append (nmap_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->cpes) {
        self->cpes = zlist_new ();
        zlist_autofree (self->cpes);
    }
    zlist_append (self->cpes, string);
    free (string);
}

size_t
nmap_msg_cpes_size (nmap_msg_t *self)
{
    return zlist_size (self->cpes);
}


//  --------------------------------------------------------------------------
//  Get/set the return_code field

uint16_t
nmap_msg_return_code (nmap_msg_t *self)
{
    assert (self);
    return self->return_code;
}

void
nmap_msg_set_return_code (nmap_msg_t *self, uint16_t return_code)
{
    assert (self);
    self->return_code = return_code;
}


//  --------------------------------------------------------------------------
//  Get/set the stderr field

const char *
nmap_msg_stderr (nmap_msg_t *self)
{
    assert (self);
    return self->stderr;
}

void
nmap_msg_set_stderr (nmap_msg_t *self, const char *format, ...)
{
    //  Format stderr from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->stderr);
    self->stderr = zsys_vprintf (format, argptr);
    va_end (argptr);
}



//  --------------------------------------------------------------------------
//  Selftest

int
nmap_msg_test (bool verbose)
{
    printf (" * nmap_msg: ");

    //  @selftest
    //  Simple create/destroy test
    nmap_msg_t *self = nmap_msg_new (0);
    assert (self);
    nmap_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-nmap_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-nmap_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    nmap_msg_t *copy;
    self = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_type (self, "Life is short but Now lasts for ever");
    nmap_msg_headers_insert (self, "Name", "Brutus");
    nmap_msg_headers_insert (self, "Age", "%d", 43);
    nmap_msg_args_append (self, "Name: %s", "Brutus");
    nmap_msg_args_append (self, "Age: %d", 43);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_type (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_headers_size (self) == 2);
        assert (streq (nmap_msg_headers_string (self, "Name", "?"), "Brutus"));
        assert (nmap_msg_headers_number (self, "Age", 0) == 43);
        assert (nmap_msg_args_size (self) == 2);
        assert (streq (nmap_msg_args_first (self), "Name: Brutus"));
        assert (streq (nmap_msg_args_next (self), "Age: 43"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_LIST_SCAN);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_addr (self, "Life is short but Now lasts for ever");
    nmap_msg_set_host_state (self, 123);
    nmap_msg_set_reason (self, "Life is short but Now lasts for ever");
    nmap_msg_hostnames_insert (self, "Name", "Brutus");
    nmap_msg_hostnames_insert (self, "Age", "%d", 43);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_addr (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_host_state (self) == 123);
        assert (streq (nmap_msg_reason (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_hostnames_size (self) == 2);
        assert (streq (nmap_msg_hostnames_string (self, "Name", "?"), "Brutus"));
        assert (nmap_msg_hostnames_number (self, "Age", 0) == 43);
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_DEV_SCAN);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_host_state (self, 123);
    nmap_msg_set_reason (self, "Life is short but Now lasts for ever");
    nmap_msg_addresses_insert (self, "Name", "Brutus");
    nmap_msg_addresses_insert (self, "Age", "%d", 43);
    nmap_msg_hostnames_insert (self, "Name", "Brutus");
    nmap_msg_hostnames_insert (self, "Age", "%d", 43);
    zframe_t *dev_scan_ports = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_ports (self, &dev_scan_ports);
    zframe_t *dev_scan_os = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_os (self, &dev_scan_os);
    zframe_t *dev_scan_scripts = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_scripts (self, &dev_scan_scripts);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (nmap_msg_host_state (self) == 123);
        assert (streq (nmap_msg_reason (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_addresses_size (self) == 2);
        assert (streq (nmap_msg_addresses_string (self, "Name", "?"), "Brutus"));
        assert (nmap_msg_addresses_number (self, "Age", 0) == 43);
        assert (nmap_msg_hostnames_size (self) == 2);
        assert (streq (nmap_msg_hostnames_string (self, "Name", "?"), "Brutus"));
        assert (nmap_msg_hostnames_number (self, "Age", 0) == 43);
        assert (zframe_streq (nmap_msg_ports (self), "Captcha Diem"));
        assert (zframe_streq (nmap_msg_os (self), "Captcha Diem"));
        assert (zframe_streq (nmap_msg_scripts (self), "Captcha Diem"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_PORT_SCAN);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_protocol (self, "Life is short but Now lasts for ever");
    nmap_msg_set_portid (self, 123);
    nmap_msg_set_port_state (self, 123);
    nmap_msg_set_reason (self, "Life is short but Now lasts for ever");
    nmap_msg_set_reason_ttl (self, 123);
    nmap_msg_set_reason_ip (self, "Life is short but Now lasts for ever");
    zframe_t *port_scan_service = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_service (self, &port_scan_service);
    zframe_t *port_scan_scripts = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_scripts (self, &port_scan_scripts);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_protocol (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_portid (self) == 123);
        assert (nmap_msg_port_state (self) == 123);
        assert (streq (nmap_msg_reason (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_reason_ttl (self) == 123);
        assert (streq (nmap_msg_reason_ip (self), "Life is short but Now lasts for ever"));
        assert (zframe_streq (nmap_msg_service (self), "Captcha Diem"));
        assert (zframe_streq (nmap_msg_scripts (self), "Captcha Diem"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_SERVICE_SCAN);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_name (self, "Life is short but Now lasts for ever");
    nmap_msg_set_conf (self, 123);
    nmap_msg_set_method (self, 123);
    nmap_msg_set_version (self, "Life is short but Now lasts for ever");
    nmap_msg_set_product (self, "Life is short but Now lasts for ever");
    nmap_msg_set_extrainfo (self, "Life is short but Now lasts for ever");
    nmap_msg_set_tunnel (self, 123);
    nmap_msg_set_service_proto (self, 123);
    nmap_msg_set_rpcnum (self, 123);
    nmap_msg_set_lowver (self, 123);
    nmap_msg_set_highver (self, 123);
    nmap_msg_set_hostname (self, "Life is short but Now lasts for ever");
    nmap_msg_set_ostype (self, "Life is short but Now lasts for ever");
    nmap_msg_set_devicetype (self, "Life is short but Now lasts for ever");
    nmap_msg_set_servicefp (self, "Life is short but Now lasts for ever");
    nmap_msg_set_cpe (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_name (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_conf (self) == 123);
        assert (nmap_msg_method (self) == 123);
        assert (streq (nmap_msg_version (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_product (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_extrainfo (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_tunnel (self) == 123);
        assert (nmap_msg_service_proto (self) == 123);
        assert (nmap_msg_rpcnum (self) == 123);
        assert (nmap_msg_lowver (self) == 123);
        assert (nmap_msg_highver (self) == 123);
        assert (streq (nmap_msg_hostname (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_ostype (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_devicetype (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_servicefp (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_cpe (self), "Life is short but Now lasts for ever"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_SCRIPT);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_script_id (self, "Life is short but Now lasts for ever");
    zchunk_t *script_data = zchunk_new ("Captcha Diem", 12);
    nmap_msg_set_data (self, &script_data);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_script_id (self), "Life is short but Now lasts for ever"));
        assert (memcmp (zchunk_data (nmap_msg_data (self)), "Captcha Diem", 12) == 0);
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_OS_SCAN);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    zframe_t *os_scan_portused = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_portused (self, &os_scan_portused);
    zframe_t *os_scan_osmatch = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_osmatch (self, &os_scan_osmatch);
    nmap_msg_osfingerprints_append (self, "Name: %s", "Brutus");
    nmap_msg_osfingerprints_append (self, "Age: %d", 43);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (zframe_streq (nmap_msg_portused (self), "Captcha Diem"));
        assert (zframe_streq (nmap_msg_osmatch (self), "Captcha Diem"));
        assert (nmap_msg_osfingerprints_size (self) == 2);
        assert (streq (nmap_msg_osfingerprints_first (self), "Name: Brutus"));
        assert (streq (nmap_msg_osfingerprints_next (self), "Age: 43"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_PORTUSED);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_port_state (self, 123);
    nmap_msg_set_proto (self, "Life is short but Now lasts for ever");
    nmap_msg_set_portid (self, 123);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (nmap_msg_port_state (self) == 123);
        assert (streq (nmap_msg_proto (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_portid (self) == 123);
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_OSMATCH);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_name (self, "Life is short but Now lasts for ever");
    nmap_msg_set_accuracy (self, 123);
    zframe_t *osmatch_osclass = zframe_new ("Captcha Diem", 12);
    nmap_msg_set_osclass (self, &osmatch_osclass);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_name (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_accuracy (self) == 123);
        assert (zframe_streq (nmap_msg_osclass (self), "Captcha Diem"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_OSCLASS);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_vendor (self, "Life is short but Now lasts for ever");
    nmap_msg_set_osgen (self, "Life is short but Now lasts for ever");
    nmap_msg_set_type (self, "Life is short but Now lasts for ever");
    nmap_msg_set_osaccuracy (self, "Life is short but Now lasts for ever");
    nmap_msg_set_osfamily (self, "Life is short but Now lasts for ever");
    nmap_msg_cpes_append (self, "Name: %s", "Brutus");
    nmap_msg_cpes_append (self, "Age: %d", 43);
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (streq (nmap_msg_vendor (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_osgen (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_type (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_osaccuracy (self), "Life is short but Now lasts for ever"));
        assert (streq (nmap_msg_osfamily (self), "Life is short but Now lasts for ever"));
        assert (nmap_msg_cpes_size (self) == 2);
        assert (streq (nmap_msg_cpes_first (self), "Name: Brutus"));
        assert (streq (nmap_msg_cpes_next (self), "Age: 43"));
        nmap_msg_destroy (&self);
    }
    self = nmap_msg_new (NMAP_MSG_SCAN_ERROR);
    
    //  Check that _dup works on empty message
    copy = nmap_msg_dup (self);
    assert (copy);
    nmap_msg_destroy (&copy);

    nmap_msg_set_return_code (self, 123);
    nmap_msg_args_append (self, "Name: %s", "Brutus");
    nmap_msg_args_append (self, "Age: %d", 43);
    nmap_msg_set_stderr (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    nmap_msg_send_again (self, output);
    nmap_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = nmap_msg_recv (input);
        assert (self);
        assert (nmap_msg_routing_id (self));
        
        assert (nmap_msg_return_code (self) == 123);
        assert (nmap_msg_args_size (self) == 2);
        assert (streq (nmap_msg_args_first (self), "Name: Brutus"));
        assert (streq (nmap_msg_args_next (self), "Age: 43"));
        assert (streq (nmap_msg_stderr (self), "Life is short but Now lasts for ever"));
        nmap_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
