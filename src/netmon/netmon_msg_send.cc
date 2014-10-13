/* netmon_msg_send.cc
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
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Karol Hrdina <karolhrdina@eaton.com>
           Michal Vyskocil <michalvyskocil@eaton.com>
 
Description: Encapsulate message sending from netmon into zproto generated api
References: BIOS-406
*/

#include <assert.h>
#include <czmq.h>

#include "netdisc_msg.h"
#include "netmon_msg_send.h"

void
netmon_msg_send(
    const char *event, const char *ifname, uint8_t ipver,
    const char *ipaddr, uint8_t prefixlen, const char *mac,
    void *socket) {

    assert (event);
    assert (ifname);
    assert ((ipver == NETDISC_IPVER_IPV4) ||
            (ipver == NETDISC_IPVER_IPV6));
    assert (ipaddr);
    assert (mac);
    assert (socket);
    assert (zsock_is (socket) == true);

    int msg_type = -1;
    if (strcmp (event, NETMON_EVENT_ADD) == 0) {
        msg_type = NETDISC_MSG_AUTO_ADD;
    }
    else if (strcmp (event, NETMON_EVENT_DEL) == 0) {
        msg_type = NETDISC_MSG_AUTO_DEL;
    }

    assert (msg_type > 0);
    netdisc_msg_t *msg = netdisc_msg_new (msg_type);
    netdisc_msg_set_name (msg, "%s", ifname);
    netdisc_msg_set_ipver (msg, ipver);
    netdisc_msg_set_ipaddr (msg, "%s",ipaddr );
    netdisc_msg_set_prefixlen (msg, prefixlen);
    netdisc_msg_set_mac (msg, "%s", mac);

    int rv = netdisc_msg_send (&msg, socket);
    assert (rv != -1); // value pulled out of **s; need to verify    
}

