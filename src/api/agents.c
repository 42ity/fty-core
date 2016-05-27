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
