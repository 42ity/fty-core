/*  =========================================================================
    compute_msg - compute messages
    
    Codec header for compute_msg.

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

#ifndef __COMPUTE_MSG_H_INCLUDED__
#define __COMPUTE_MSG_H_INCLUDED__

/*  These are the compute_msg messages:

    GET_COMPUTATION - 
        module_name         string      Name of the computational module
        args                hash        A list of arguments for computation
        params              hash        A list of module parameters

    RETURN_COMPUTATION - 
        results             hash        List of results according to the list of parameters
*/

#define COMPUTE_MSG_VERSION                 1.0

#define COMPUTE_MSG_GET_COMPUTATION         1
#define COMPUTE_MSG_RETURN_COMPUTATION      2

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef COMPUTE_MSG_T_DEFINED
typedef struct _compute_msg_t compute_msg_t;
#define COMPUTE_MSG_T_DEFINED
#endif

//  @interface
//  Create a new compute_msg
compute_msg_t *
    compute_msg_new (int id);

//  Destroy the compute_msg
void
    compute_msg_destroy (compute_msg_t **self_p);

//  Parse a zmsg_t and decides whether it is compute_msg. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
    is_compute_msg (zmsg_t *msg_p);

//  Parse a compute_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.
compute_msg_t *
    compute_msg_decode (zmsg_t **msg_p);

//  Encode compute_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    compute_msg_encode (compute_msg_t **self_p);

//  Receive and parse a compute_msg from the socket. Returns new object, 
//  or NULL if error. Will block if there's no message waiting.
compute_msg_t *
    compute_msg_recv (void *input);

//  Receive and parse a compute_msg from the socket. Returns new object, 
//  or NULL either if there was no input waiting, or the recv was interrupted.
compute_msg_t *
    compute_msg_recv_nowait (void *input);

//  Send the compute_msg to the output, and destroy it
int
    compute_msg_send (compute_msg_t **self_p, void *output);

//  Send the compute_msg to the output, and do not destroy it
int
    compute_msg_send_again (compute_msg_t *self, void *output);

//  Encode the GET_COMPUTATION 
zmsg_t *
    compute_msg_encode_get_computation (
        const char *module_name,
        zhash_t *args,
        zhash_t *params);

//  Encode the RETURN_COMPUTATION 
zmsg_t *
    compute_msg_encode_return_computation (
        zhash_t *results);


//  Send the GET_COMPUTATION to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    compute_msg_send_get_computation (void *output,
        const char *module_name,
        zhash_t *args,
        zhash_t *params);
    
//  Send the RETURN_COMPUTATION to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    compute_msg_send_return_computation (void *output,
        zhash_t *results);
    
//  Duplicate the compute_msg message
compute_msg_t *
    compute_msg_dup (compute_msg_t *self);

//  Print contents of message to stdout
void
    compute_msg_print (compute_msg_t *self);

//  Get/set the message routing id
zframe_t *
    compute_msg_routing_id (compute_msg_t *self);
void
    compute_msg_set_routing_id (compute_msg_t *self, zframe_t *routing_id);

//  Get the compute_msg id and printable command
int
    compute_msg_id (compute_msg_t *self);
void
    compute_msg_set_id (compute_msg_t *self, int id);
const char *
    compute_msg_command (compute_msg_t *self);

//  Get/set the module_name field
const char *
    compute_msg_module_name (compute_msg_t *self);
void
    compute_msg_set_module_name (compute_msg_t *self, const char *format, ...);

//  Get/set the args field
zhash_t *
    compute_msg_args (compute_msg_t *self);
//  Get the args field and transfer ownership to caller
zhash_t *
    compute_msg_get_args (compute_msg_t *self);
//  Set the args field, transferring ownership from caller
void
    compute_msg_set_args (compute_msg_t *self, zhash_t **args_p);
    
//  Get/set a value in the args dictionary
const char *
    compute_msg_args_string (compute_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    compute_msg_args_number (compute_msg_t *self,
        const char *key, uint64_t default_value);
void
    compute_msg_args_insert (compute_msg_t *self,
        const char *key, const char *format, ...);
size_t
    compute_msg_args_size (compute_msg_t *self);

//  Get/set the params field
zhash_t *
    compute_msg_params (compute_msg_t *self);
//  Get the params field and transfer ownership to caller
zhash_t *
    compute_msg_get_params (compute_msg_t *self);
//  Set the params field, transferring ownership from caller
void
    compute_msg_set_params (compute_msg_t *self, zhash_t **params_p);
    
//  Get/set a value in the params dictionary
const char *
    compute_msg_params_string (compute_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    compute_msg_params_number (compute_msg_t *self,
        const char *key, uint64_t default_value);
void
    compute_msg_params_insert (compute_msg_t *self,
        const char *key, const char *format, ...);
size_t
    compute_msg_params_size (compute_msg_t *self);

//  Get/set the results field
zhash_t *
    compute_msg_results (compute_msg_t *self);
//  Get the results field and transfer ownership to caller
zhash_t *
    compute_msg_get_results (compute_msg_t *self);
//  Set the results field, transferring ownership from caller
void
    compute_msg_set_results (compute_msg_t *self, zhash_t **results_p);
    
//  Get/set a value in the results dictionary
const char *
    compute_msg_results_string (compute_msg_t *self,
        const char *key, const char *default_value);
uint64_t
    compute_msg_results_number (compute_msg_t *self,
        const char *key, uint64_t default_value);
void
    compute_msg_results_insert (compute_msg_t *self,
        const char *key, const char *format, ...);
size_t
    compute_msg_results_size (compute_msg_t *self);

//  Self test of this class
int
    compute_msg_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define compute_msg_dump    compute_msg_print

#ifdef __cplusplus
}
#endif

#endif
