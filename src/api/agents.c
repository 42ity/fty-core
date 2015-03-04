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
#include "utils_ymsg.h"
#include "agents.h"
#include "defs.h"

BIOS_EXPORT ymsg_t *
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
    ymsg_aux_set_unsigned (message, NETMON_KEY_PREFIXLEN, prefix_length);
    ymsg_aux_insert (message, NETMON_KEY_MACADDR, "%s", mac_address == NULL ? "" : mac_address);
    return message; 
}

BIOS_EXPORT int
bios_netmon_decode
(ymsg_t **self_p, int *event, char *interface_name, int *ip_version, char *ip_address, uint8_t *prefix_length, char *mac_address) {
    if (!self_p || !event || !interface_name || !ip_version || !ip_address || !mac_address)
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
        interface_name = strdup (ymsg_aux_string (self, NETMON_KEY_IFNAME, ""));
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
        ip_address = strdup (ymsg_aux_string (self, NETMON_KEY_IPADDR, ""));
        uint64_t ui;
        int rc = ymsg_aux_unsigned (self, NETMON_KEY_PREFIXLEN, &ui);
        if (rc != 0 || ui > 255) {
            return -1;
        }
        *prefix_length = (uint8_t) ui; 
        mac_address = strdup (ymsg_aux_string (self, NETMON_KEY_MACADDR, ""));
        ymsg_destroy (self_p);
        return 0;
    }
    return -1; 
}
