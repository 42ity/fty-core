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
#include "utils_ymsg.h"
#include "bios_agent.h"
#include "agents.h"
#include "defs.h"

ymsg_t *
bios_netmon_encode
(int event, const char *interface_name, int ip_version, const char *ip_address, uint8_t prefix_length, const char *mac_address) {
    ymsg_t *message = ymsg_new (YMSG_SEND);
    if (!message)
        return NULL;

    char *s = NULL;
    switch (event) {
        case AUTO_ADD:
        {
            s = NETMON_VAL_AUTO_ADD;
            break;
        }
        case AUTO_DEL:
        {
            s = NETMON_VAL_AUTO_DEL;
            break;
        }
        case MAN_ADD:
        {
            s = NETMON_VAL_MAN_ADD;
            break;
        }
        case MAN_DEL:
        {
            s = NETMON_VAL_MAN_DEL;
            break;
        }
        case EXCL_ADD:
        {
            s = NETMON_VAL_EXCL_ADD;
            break;
        }
        case EXCL_DEL: 
        {
            s = NETMON_VAL_EXCL_DEL;
            break;
        }
        default:
            s = "";
            break;
    }
    ymsg_aux_insert (message, NETMON_KEY_EVENT, "%s", s); 
    ymsg_aux_insert (message, NETMON_KEY_IFNAME, "%s", interface_name == NULL ? "" : interface_name); 
    if (ip_version == IP_VERSION_6) {
        s = NETMON_VAL_IPV4;         
    }
    else if (ip_version == IP_VERSION_6) {
        s = NETMON_VAL_IPV6; 
    }
    else {
        s = ""; 
    }
    ymsg_aux_insert (message, NETMON_KEY_IPVER, "%s", s); 
    ymsg_aux_insert (message, NETMON_KEY_IPADDR, "%s", ip_address == NULL ? "" : ip_address);
    ymsg_aux_set_uint32 (message, NETMON_KEY_PREFIXLEN, prefix_length);
    ymsg_aux_insert (message, NETMON_KEY_MACADDR, "%s", mac_address == NULL ? "" : mac_address);
    return message; 
}

int
bios_netmon_decode
(ymsg_t **self_p, int *event, char **interface_name, int *ip_version, char **ip_address, uint8_t *prefix_length, char **mac_address) {
    if (!self_p || !event || !interface_name || !ip_version || !ip_address || !prefix_length || !mac_address)
        return -1;
    if (ymsg_id (*self_p) != YMSG_SEND) 
        return -1;
    
    if (*self_p) {
        ymsg_t *self = *self_p;
        const char *s = ymsg_aux_string (self, NETMON_KEY_EVENT, "");
        if (strcmp (s, NETMON_VAL_AUTO_ADD) == 0) {
            *event = AUTO_ADD;
        }
        else if (strcmp (s, NETMON_VAL_AUTO_DEL) == 0) {
            *event = AUTO_DEL;
        }
        else if (strcmp (s, NETMON_VAL_MAN_ADD) == 0) {
            *event = MAN_ADD;
        }
        else if (strcmp (s, NETMON_VAL_MAN_DEL) == 0) {
            *event = MAN_DEL;
        }
        else if (strcmp (s, NETMON_VAL_EXCL_ADD) == 0) {
            *event = EXCL_ADD;
        }
        else if (strcmp (s, NETMON_VAL_EXCL_DEL) == 0) {
            *event = EXCL_DEL;
        } else {
            return -1;
        }
        *interface_name = strdup (ymsg_aux_string (self, NETMON_KEY_IFNAME, ""));
        s = ymsg_aux_string (self, NETMON_KEY_IPVER, "");
        if (strcmp (s, NETMON_VAL_IPV4) == 0) {
            *ip_version = IP_VERSION_4;
        }
        else if (strcmp (s, NETMON_VAL_IPV6) == 0) {
            *ip_version = IP_VERSION_6;
        }
        else {
            return -1;
        }
        *ip_address = strdup (ymsg_aux_string (self, NETMON_KEY_IPADDR, ""));
        uint32_t ui;
        int rc = ymsg_aux_uint32 (self, NETMON_KEY_PREFIXLEN, &ui);
        if (rc != 0 || ui > 255) {
            return -1;
        }
        *prefix_length = (uint8_t) ui; 
        *mac_address = strdup (ymsg_aux_string (self, NETMON_KEY_MACADDR, ""));
        ymsg_destroy (self_p);
        return 0;
    }
    return -1; 
}


ymsg_t *
    bios_inventory_encode
        (const char *device_name, zhash_t **ext_attributes, const char *module_name)
{
    ymsg_t *message = ymsg_new (YMSG_SEND);
    if ( !message )
        return NULL;
   
    app_t *request = app_new (APP_MODULE);
    // module name
    app_set_name (request, module_name);
    
    // device name
    zlist_t *paramslist = zlist_new ();
    zlist_autofree (paramslist);
    zlist_append (paramslist, (void *)device_name);
    app_set_params (request, &paramslist); 
    zlist_destroy (&paramslist);
    
    // ext attributes
    app_set_args (request, ext_attributes);
    zhash_destroy (ext_attributes);

    zmsg_t *request_encoded = app_encode (&request);
    byte *buffer;
    size_t sz = zmsg_encode (request_encoded, &buffer);

    zchunk_t *request_chunk = zchunk_new (buffer, sz);

    ymsg_set_request (message, &request_chunk);
    return message; 
}

int
    bios_inventory_decode
        (ymsg_t **self_p, char **device_name, zhash_t **ext_attributes, char **module_name)
{
    if ( !self_p || !device_name || !ext_attributes )
        return -1;
    if ( ymsg_id (*self_p) != YMSG_SEND ) 
        return -6;

    if ( *self_p )
    {
        ymsg_t *self = *self_p;
       
        zchunk_t *request = ymsg_get_request (self);
        zmsg_t *zmsg = zmsg_decode (zchunk_data (request), zchunk_size (request));
        
        if ( !zmsg )
            return -2; // zmsg decode fail

        app_t *app_msg = app_decode (&zmsg);
    
        if ( !app_msg )
            return -3; // malformed app_msg

        zlist_t *param = app_get_params (app_msg);
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
    if (!start_timestamp || !end_timestamp || !type || !step || !source)
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
bios_web_average_request_decode (ymsg_t **self_p, int64_t *start_timestamp, int64_t *end_timestamp, char **type, char **step, uint64_t *element_id, char **source) {   
    if (!self_p || !*self_p || !start_timestamp || !end_timestamp || !type || !step || !element_id || !source)
        return -1;

    assert (*self_p);
    ymsg_t *self = *self_p;
    int rc = 0;

    if (ymsg_id (self) != YMSG_SEND) 
        return -1;

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

    ymsg_destroy (self_p);
    return 0;

bios_web_average_request_decode_err:
    if(*type)
        free(*type);
    if(*step)
        free(*step);
    if(*source)
        free(*source);
    ymsg_destroy (self_p);
    return -1;
}

ymsg_t *
bios_web_average_reply_encode (const char *json) {
    ymsg_t *message = ymsg_new (YMSG_REPLY);
    if (!message) 
        return NULL;
    zchunk_t *chunk = zchunk_new ((void *) json, strlen (json));
    if (!chunk) {
        ymsg_destroy (&message);       
        return NULL;
    }
    ymsg_set_response (message, &chunk);
    return message;
}

int
bios_web_average_reply_decode (ymsg_t **self_p, char **json) {
    if (!self_p || !*self_p || !json)
        return -1;
    
    assert (*self_p);
    ymsg_t *self = *self_p;

    if (ymsg_id (self) != YMSG_REPLY) {
        ymsg_destroy (self_p);
        return -1;
    }

    zchunk_t *chunk = ymsg_response (self);
    if (!chunk) {
        ymsg_destroy (self_p);
        return -1;
    }
    *json = (char*)malloc(zchunk_size (chunk) * sizeof(char));
    if(json == NULL) {
        ymsg_destroy (self_p);
        return -1;
    }
    *json = strndup ((char *) zchunk_data (chunk), zchunk_size (chunk));
    ymsg_destroy (self_p);
    return 0;
}

ymsg_t *
bios_db_measurements_read_request_encode (const char *start_timestamp, const char *end_timestamp, uint64_t element_id, const char *source, char **subject) {
    if (!start_timestamp || !end_timestamp || !source)
        return NULL;

    ymsg_t *message = ymsg_new (YMSG_SEND);
    if (!message) 
        return NULL;
    
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_START_TS, "%s", start_timestamp);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_END_TS, "%s", end_timestamp);
    ymsg_aux_set_uint64 (message,  WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    ymsg_aux_insert (message, WEB_AVERAGE_KEY_SOURCE, "%s", source);

    if (subject)
        *subject = strdup ("get_measurements");
    return message;
}

int
bios_db_measurements_read_request_decode (ymsg_t **self_p, char **start_timestamp, char **end_timestamp, uint64_t *element_id, char **source) {
    if (!self_p || !*self_p || !start_timestamp || !end_timestamp || !element_id || !source)
        return -1;

    assert (*self_p);
    ymsg_t *self = *self_p;

    if (ymsg_id (self) != YMSG_SEND) 
        return -1;

    *start_timestamp = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_START_TS, ""));
    *end_timestamp = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_END_TS, ""));
    int rc = ymsg_aux_uint64 (self, WEB_AVERAGE_KEY_ELEMENT_ID, element_id);
    if (rc != 0)
        return -1;
    *source = strdup (ymsg_aux_string (self, WEB_AVERAGE_KEY_SOURCE, ""));

    ymsg_destroy (self_p);
    return 0;
}


ymsg_t *
bios_db_measurements_read_reply_encode (const char *json) {
    ymsg_t *message = ymsg_new (YMSG_REPLY);
    if (!message) 
        return NULL;
    zchunk_t *chunk = zchunk_new ((void *) json, strlen (json));
    if (!chunk) {
        ymsg_destroy (&message);       
        return NULL;
    }
    ymsg_set_response (message, &chunk);
    return message;
}

int
bios_db_measurements_read_reply_decode (ymsg_t **self_p, char **json) {
    if (!self_p || !*self_p || !json)
        return -1;
    
    assert (*self_p);
    ymsg_t *self = *self_p;

    if (ymsg_id (self) != YMSG_REPLY) 
        return -1;

    zchunk_t *chunk = ymsg_get_response (self);
    if (!chunk)
        return -1;
    *json = strndup ((char *) zchunk_data (chunk), zchunk_size (chunk));
    ymsg_destroy (self_p);
    return 0;
}
