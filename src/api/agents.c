/*
Copyright (C) 2014 - 2015 Eaton

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
*/

#include <czmq.h>
#include <string.h>

#include "defs.h"

#include "utils.h" 
#include "utils_ymsg.h"
#include "utils_app.h"
#include "bios_agent.h"

#include "agents.h"
#include "cleanup.h"

ymsg_t *
    bios_inventory_encode
        (const char *device_name, zhash_t **ext_attributes, const char *module_name)
{
    ymsg_t *message = ymsg_new (YMSG_SEND);
    if ( !message )
        return NULL;
   
    _scoped_app_t *request = app_new (APP_MODULE);
    // module name
    app_set_name (request, module_name);
    
    // device name
    _scoped_zlist_t *paramslist = zlist_new ();
    zlist_autofree (paramslist);
    zlist_append (paramslist, (void *)device_name);
    app_set_params (request, &paramslist); 
    zlist_destroy (&paramslist);
    
    // ext attributes
    app_set_args (request, ext_attributes);
    zhash_destroy (ext_attributes);

    _scoped_zmsg_t *request_encoded = app_encode (&request);
    _scoped_byte *buffer = NULL;
    size_t sz = zmsg_encode (request_encoded, &buffer);
    zmsg_destroy(&request_encoded);

    _scoped_zchunk_t *request_chunk = zchunk_new (buffer, sz);
    FREE0 (buffer)

    ymsg_set_request (message, &request_chunk);
    return message; 
}

int
bios_inventory_decode (ymsg_t **self_p, char **device_name, zhash_t **ext_attributes, char **module_name)
{
    if ( !self_p || !device_name || !ext_attributes )
        return -1;
    if ( ymsg_id (*self_p) != YMSG_SEND ) 
        return -6;

    if ( *self_p )
    {
        ymsg_t *self = *self_p;
       
        _scoped_zchunk_t *request = ymsg_get_request (self);
        if ( !request )
                return -2;  // no chunk to decode        
        _scoped_zmsg_t *zmsg = zmsg_decode (zchunk_data (request), zchunk_size (request));
        zchunk_destroy (&request);
        
        if ( !zmsg )
            return -2; // zmsg decode fail

        _scoped_app_t *app_msg = app_decode (&zmsg);
    
        if ( !app_msg )
            return -3; // malformed app_msg

        _scoped_zlist_t *param = app_get_params (app_msg);
        if ( zlist_size (param) != 1 )
        {
            zlist_destroy (&param);
            return -4; // unexpected data inside app_msg
        }
        *ext_attributes = app_get_args (app_msg);
        *module_name = strdup (app_name (app_msg));
        *device_name = strdup (zlist_first (param));
        
        zlist_destroy (&param);
        ymsg_destroy (self_p);
        app_destroy (&app_msg);
        return 0;
    }
    return -5; 
}


ymsg_t *
bios_measurement_encode (const char *device_name,
                         const char *quantity,
                         const char *units,
                         int32_t value,
                         int32_t scale,
                         int64_t time)
{
    if( ! device_name || ! quantity || ! units ) return NULL; 
    ymsg_t *msg = ymsg_new(YMSG_SEND);
    ymsg_set_string(msg, "device", (char *)device_name );
    ymsg_set_string(msg, "quantity", (char *)quantity );
    ymsg_set_string(msg, "units", (char *)units );
    ymsg_set_int32(msg, "value",  value );
    ymsg_set_int32(msg, "scale", scale );
    ymsg_set_int64(msg, "time", time );
    return msg;
}

// on (error) does not destroy *self_p
int
    bios_measurement_decode (ymsg_t **self_p,
                           char **device_name,
                           char **quantity,
                           char **units,
                           int32_t *value,
                           int32_t *scale,
                           int64_t *time)
{
   if( ! self_p || ! device_name || ! quantity || ! units || ! value || ! scale || ! time ) return -1;
   if (  *self_p == NULL ) return -2;

   const char *d, *q, *u, *v, *s, *t;

   d = ymsg_get_string( *self_p, "device" );
   q = ymsg_get_string( *self_p, "quantity" );
   u = ymsg_get_string( *self_p, "units" );
   v = ymsg_get_string( *self_p, "value" );
   s = ymsg_get_string( *self_p, "scale" );
   t = ymsg_get_string( *self_p, "time" );

   if( ! d || ! q || ! u || ! v || ! s || !t ) return -3;

   *device_name = strdup(d);
   *quantity = strdup(q);
   *units = strdup(u);
   
   *value = ymsg_get_int32( *self_p, "value" );
   *scale = ymsg_get_int32( *self_p, "scale" );
   *time = ymsg_get_int64( *self_p, "time" );
   ymsg_destroy (self_p);
   return 0;
}

ymsg_t *
bios_web_average_request_encode (int64_t start_timestamp, int64_t end_timestamp, const char *type, const char *step, uint64_t element_id, const char *source) {
    if (!type || !step || !source)
        return NULL;
    
    ymsg_t *message = ymsg_new (YMSG_SEND);
    if (!message) 
        return NULL;
    
    ymsg_aux_set_int64 (message, WEB_AVERAGE_KEY_START_TS, start_timestamp);
    ymsg_aux_set_int64 (message, WEB_AVERAGE_KEY_END_TS, end_timestamp);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_TYPE, "%s", type);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_STEP, "%s", step);
    ymsg_aux_set_uint64 (message,  WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_SOURCE, "%s", source);
    return message;
}

int
bios_web_average_request_extract (ymsg_t *self, int64_t *start_timestamp, int64_t *end_timestamp, char **type, char **step, uint64_t *element_id, char **source) {   
    if (!self || !start_timestamp || !end_timestamp || !type || !step || !element_id || !source || ymsg_id (self) != YMSG_SEND)
        return -1;

    int rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_START_TS, start_timestamp);
    if (rc != 0)
        goto bios_web_average_request_decode_err;
    rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_END_TS, end_timestamp);
    if (rc != 0)
        goto bios_web_average_request_decode_err;
    *type = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_TYPE, ""));
    *step = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_STEP, ""));
    rc = ymsg_aux_uint64 (self, WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    if (rc != 0)
        goto bios_web_average_request_decode_err;
    *source = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_SOURCE, ""));
    return 0;

bios_web_average_request_decode_err:
    FREE0 (*type);
    FREE0 (*step);
    FREE0 (*source);
    return -1;
}

ymsg_t *
bios_web_average_reply_encode (const char *json) {
    if (!json)
        return NULL;
    ymsg_t *message = ymsg_new (YMSG_REPLY);
    if (!message) 
        return NULL;
    _scoped_zchunk_t *chunk = zchunk_new ((void *) json, strlen (json));
    if (!chunk) {
        ymsg_destroy (&message);       
        return NULL;
    }
    ymsg_set_response (message, &chunk);
    return message;
}

int
bios_web_average_reply_extract (ymsg_t *self, char **json) {
    if (!self || !json)
        return -1;
    
    if (ymsg_id (self) != YMSG_REPLY)
        return -1;

    zchunk_t *chunk = ymsg_response (self);
    if (!chunk)
        return -1;

    *json = strndup ((char *) zchunk_data (chunk), zchunk_size (chunk));
    if (*json == NULL)
        return -1;
    return 0;
}

ymsg_t *
bios_db_measurements_read_request_encode (int64_t start_timestamp, int64_t end_timestamp, uint64_t element_id, const char *source, char **subject) {
    if (!source || !subject)
        return NULL;

    ymsg_t *message = ymsg_new (YMSG_SEND);
    if (!message) 
        return NULL;
    
    ymsg_aux_set_int64 (message, WEB_AVERAGE_KEY_START_TS, start_timestamp);
    ymsg_aux_set_int64 (message, WEB_AVERAGE_KEY_END_TS, end_timestamp);
    ymsg_aux_set_uint64 (message,  WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_SOURCE, "%s", source);

    *subject = strdup ("get_measurements");
    return message;
}

int
bios_db_measurements_read_request_extract (ymsg_t *self, int64_t *start_timestamp, int64_t *end_timestamp, uint64_t *element_id, char **source) {
    if (!self || !start_timestamp || !end_timestamp || !element_id || !source)
        return -1;

    if (ymsg_id (self) != YMSG_SEND)
        goto bios_db_measurements_read_request_extract_err;

    int rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_START_TS, start_timestamp);
    if (rc != 0)
        goto bios_db_measurements_read_request_extract_err;
    rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_END_TS, end_timestamp);
    if (rc != 0)
        goto bios_db_measurements_read_request_extract_err;
    rc = ymsg_aux_uint64 (self, WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    if (rc != 0)
        goto bios_db_measurements_read_request_extract_err;
    *source = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_SOURCE, ""));
    return 0;

bios_db_measurements_read_request_extract_err:
    FREE0 (*source);
    return -1;
}

ymsg_t *
bios_db_measurements_read_reply_encode (const char *json) {
    ymsg_t *message = ymsg_new (YMSG_REPLY);
    if (!message) 
        return NULL;
    _scoped_zchunk_t *chunk = zchunk_new ((void *) json, strlen (json));
    if (!chunk) {
        ymsg_destroy (&message);       
        return NULL;
    }
    ymsg_set_response (message, &chunk);
    return message;
}

int
bios_db_measurements_read_reply_extract (ymsg_t *self, char **json) {
    if (!self || !json)
        return -1;

    if (ymsg_id (self) != YMSG_REPLY)
        goto bios_db_measurements_read_reply_extract_err;

    zchunk_t *chunk = ymsg_response (self);
    if (!chunk)
        goto bios_db_measurements_read_reply_extract_err;
    *json = strndup ((char *) zchunk_data (chunk), zchunk_size (chunk));
    return 0;

bios_db_measurements_read_reply_extract_err:
    FREE0 (*json);
    return -1;
}

static app_t *
bios_asset_extra(const char *name,
                 zhash_t **ext_attributes,
                 uint32_t type_id,
                 uint32_t subtype_id,
                 uint32_t parent_id,
                 const char* status,
                 uint8_t priority,
                 int8_t operation)
{
    if ( !name )
        return NULL;

    app_t *app = app_new(APP_MODULE);
    if ( !app )
        return NULL;
    app_set_name (app, "ASSET_EXTENDED");
    if ( ext_attributes )
    {
        app_set_args  (app, ext_attributes);
        zhash_destroy (ext_attributes);
    }

    if ( type_id )
        app_args_set_uint32 (app, KEY_ASSET_TYPE_ID, type_id);
    if ( subtype_id )
        app_args_set_uint32( app, KEY_ASSET_SUBTYPE_ID, subtype_id );
    if ( parent_id )
        app_args_set_uint32 (app, KEY_ASSET_PARENT_ID, parent_id);
    if( priority )
        app_args_set_uint8( app, KEY_ASSET_PRIORITY, priority );
    if ( status )
        app_args_set_string (app, KEY_ASSET_STATUS, status);
    app_args_set_string (app, KEY_ASSET_NAME, name);
    app_args_set_int8   (app, KEY_OPERATION, operation);
    return app;
}

// encode functions destroy all * arguments
ymsg_t *
bios_asset_extra_encode(const char *name,
                   zhash_t **ext_attributes,
                   uint32_t type_id,
                   uint32_t subtype_id,
                   uint32_t parent_id,
                   const char* status,
                   uint8_t priority,
                   int8_t operation)
{
    if (!ext_attributes || !*ext_attributes || !name)
        return NULL;

    ymsg_t *message = ymsg_new (YMSG_SEND);
    assert (message);


    zframe_t *frame = zhash_pack (*ext_attributes);
    assert (frame);
    zhash_destroy (ext_attributes);

    // Create a new chunk of the specified size. If you specify the data, it
    // is copied into the chunk.
    zchunk_t *chunk = zchunk_new (zframe_data (frame), zframe_size (frame));
    assert (chunk);
    zframe_destroy (&frame);

    // Set the request field, transferring ownership from caller
    ymsg_set_request (message, &chunk);

    ymsg_aux_insert (message, "name", "%s", name);    
    ymsg_aux_insert (message, "type_id", "%" PRIu32, type_id);    
    ymsg_aux_insert (message, "subtype_id", "%" PRIu32, subtype_id);    
    ymsg_aux_insert (message, "parent_id", "%" PRIu32, parent_id);    
    ymsg_aux_insert (message, "status", "%s", status);    
    ymsg_aux_insert (message, "priority", "%" PRIu8, priority);    
    ymsg_aux_insert (message, "operation", "%" PRIi8, operation);    

    return message;
}

ymsg_t *
bios_asset_extra_encode_response(const char *name,
                   zhash_t **ext_attributes,
                   uint32_t type_id,
                   uint32_t subtype_id,
                   uint32_t parent_id,
                   const char* status,
                   uint8_t priority,
                   int8_t operation)
{
    app_t *app = bios_asset_extra(name, ext_attributes, type_id,
                 subtype_id, parent_id, status, priority, operation);
    if ( !app )
        return NULL;

    ymsg_t *msg = ymsg_new(YMSG_REPLY);
    if ( !msg )
    {
        app_destroy (&app);
        return NULL;
    }

    ymsg_response_set_app (msg, &app);
    return msg;
}

// name is a mandatory parameter
int
bios_asset_extra_extract(ymsg_t *message,
                   char **name,
                   zhash_t **ext_attributes,
                   uint32_t *type_id,
                   uint32_t *subtype_id,
                   uint32_t *parent_id,
                   char **status,
                   uint8_t *priority,
                   int8_t *operation)
{
    if (!message || !name)
        return -1;

    // Get the request field and transfer ownership to caller
    zchunk_t *chunk = ymsg_get_request (message);

    // Copies size octets from the specified data into the frame body.
    zframe_t *frame = zframe_new ((void  *) zchunk_data (chunk), zchunk_size (chunk));
    zchunk_destroy (&chunk);
    assert (frame);

    if (ext_attributes)
        *ext_attributes = zhash_unpack (frame);
    zframe_destroy (&frame);

    *name = strdup (ymsg_aux_string (message, "name", ""));
    if (type_id)
        *type_id = (uint32_t) atol (ymsg_aux_string (message, "type_id", "0"));
    if (subtype_id)
        *subtype_id = (uint32_t) atol (ymsg_aux_string (message, "subtype_id", "0"));
    if (parent_id)
        *parent_id = (uint32_t) atol (ymsg_aux_string (message, "parent_id", "0"));
    if (status)
        *status = strdup (ymsg_aux_string (message, "status", ""));
    if (priority)
        *priority = (uint8_t) atoi (ymsg_aux_string (message, "priority", "0"));
    if (operation)
        *operation = (int8_t) atoi (ymsg_aux_string (message, "operation", "0"));

    return 0;    
   
}