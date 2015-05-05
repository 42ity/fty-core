/*
Copyright (C) 2014 - 2015 Eaton

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
bios_netmon_encode
(int event, const char *interface_name, int ip_version, const char *ip_address, uint8_t prefix_length, const char *mac_address) {
    if (!interface_name || !ip_address || !mac_address ||
        (event < NETWORK_EVENT_AUTO_ADD) || (event >= NETWORK_EVENT_TERMINATOR) ||
        (ip_version != IP_VERSION_4 && ip_version != IP_VERSION_6))
        return NULL;
    ymsg_t *message = ymsg_new (YMSG_SEND);
    if (!message)
        return NULL;

    ymsg_aux_set_int32 (message, NETMON_KEY_EVENT, event);
    ymsg_aux_insert (message, NETMON_KEY_IFNAME, "%s", interface_name);
    ymsg_aux_set_int32 (message, NETMON_KEY_IPVER, ip_version);
    ymsg_aux_insert (message, NETMON_KEY_IPADDR, "%s", ip_address);
    ymsg_aux_set_uint32 (message, NETMON_KEY_PREFIXLEN, prefix_length);
    ymsg_aux_insert (message, NETMON_KEY_MACADDR, "%s", mac_address);
    return message; 
}

int
bios_netmon_extract
(ymsg_t *self, int *event, char **interface_name, int *ip_version, char **ip_address, uint8_t *prefix_length, char **mac_address) {
    if (!self || !event || !interface_name || !ip_version || !ip_address || !prefix_length || !mac_address || ymsg_id (self) != YMSG_SEND)
        return -1;

    int rv = ymsg_aux_int32 (self, NETMON_KEY_EVENT, event);
    if (rv != 0)
        goto bios_netmon_extract_err;
    *interface_name = strdup (ymsg_aux_string (self, NETMON_KEY_IFNAME, ""));
    rv = ymsg_aux_int32 (self, NETMON_KEY_IPVER, ip_version);
    if (rv != 0)
        goto bios_netmon_extract_err;
    *ip_address = strdup (ymsg_aux_string (self, NETMON_KEY_IPADDR, ""));
    uint32_t ui;
    rv = ymsg_aux_uint32 (self, NETMON_KEY_PREFIXLEN, &ui);
    if (rv != 0 || ui > 255)
        goto bios_netmon_extract_err;
    *prefix_length = (uint8_t) ui; 
    *mac_address = strdup (ymsg_aux_string (self, NETMON_KEY_MACADDR, ""));
    return 0;
        
bios_netmon_extract_err:
    FREE0 (interface_name);
    FREE0 (ip_address);
    FREE0 (mac_address);
    return -1; 
}


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
    byte *buffer;
    size_t sz = zmsg_encode (request_encoded, &buffer);
    zmsg_destroy(&request_encoded);

    _scoped_zchunk_t *request_chunk = zchunk_new (buffer, sz);
    free(buffer);

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

    int rc = -1;

    rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_START_TS, start_timestamp);
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

    int rc = -1;
    rc = ymsg_aux_int64 (self, WEB_AVERAGE_KEY_START_TS, start_timestamp);
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

ymsg_t *
bios_alert_encode (const char *rule_name,
                   alert_priority_t priority,
                   alert_state_t state,
                   const char *devices,
                   const char *alert_description,
                   time_t since)
{
    if(
        ! rule_name || ! devices ||
        ( state < ALERT_STATE_NO_ALERT ) ||
        ( state > ALERT_STATE_ONGOING_ALERT ) ||
        ( priority < ALERT_PRIORITY_P1 ) ||
        ( priority > ALERT_PRIORITY_P5 )
    ) return NULL;
    ymsg_t *msg = ymsg_new(YMSG_SEND);
    _scoped_app_t *app = app_new(APP_MODULE);
    app_set_name( app, "ALERT" );
    app_args_set_string( app, "rule", rule_name );
    app_args_set_int32( app, "priority", priority );
    app_args_set_int32( app, "state", state );
    app_args_set_string( app, "devices",  devices );
    if(alert_description) app_args_set_string(app, "description",  alert_description );
    app_args_set_int64( app, "since", since );
    ymsg_request_set_app( msg, &app );
    return msg;
}


int
bios_alert_decode (ymsg_t **self_p,
                   char **rule_name,
                   alert_priority_t *priority,
                   alert_state_t *state,
                   char **devices,
                   char **description,
                   time_t *since)
{
   if( ! self_p || ! *self_p || ! rule_name || ! priority || ! state || ! devices ) return -1;

   const char *nam, *dev, *pri, *sta, *sin, *des;
   int32_t tmp;

   app_t *app = ymsg_request_app(*self_p);
   if( ! app ) return -3;
       
   nam = app_args_string( app, "rule", NULL );
   pri = app_args_string( app, "priority", NULL );
   sta = app_args_string( app, "state", NULL );
   dev = app_args_string( app, "devices", NULL );
   des = app_args_string( app, "description", NULL );
   sin = app_args_string( app, "since", NULL );
   

   if( ! nam || ! pri || ! sta || ! dev || ! sin ) {
       app_destroy( &app );
       return -3;
   }
   tmp = app_args_int32( app, "priority" );
   if( tmp < ALERT_PRIORITY_P1 || tmp > ALERT_PRIORITY_P5 ) {
       app_destroy( &app );
       return -4;
   }
   *priority = (alert_priority_t)tmp;
   tmp = app_args_int32( app, "state" );
   if( tmp < ALERT_STATE_NO_ALERT || tmp > ALERT_STATE_ONGOING_ALERT ) {
       app_destroy( &app );
       return -5;
   }
   *state = (alert_state_t)tmp;
   if( since ) {
       *since = string_to_int64( sin );
       if( errno ) {
           app_destroy(&app);
           return -6;
       }
   }
   *rule_name = strdup(nam);
   *devices = strdup(dev);
   if( description ) {
       if( des ) {
           *description = strdup(des);
       } else {
           *description = NULL;
       }
   }
   app_destroy(&app);
   ymsg_destroy(self_p);
   return 0;
}
