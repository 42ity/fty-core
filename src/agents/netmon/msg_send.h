/* netmon_msg_send.h 
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
#ifndef SRC_DRIVERS_NETMON_MSG_SEND_H_
#define SRC_DRIVERS_NETMON_MSG_SEND_H_

#define NETDISC_IPVER_IPV4  0
#define NETDISC_IPVER_IPV6  1

#define NETMON_EVENT_ADD    "add"
#define NETMON_EVENT_DEL    "del"

/*!
    \brief  TODO    
    \author
*/
#ifdef __cplusplus
extern "C"
{
#endif
void
netmon_msg_send (
    const char *event,
    const char *ifname,
    uint8_t ipver,
    const char *ipaddr,
    uint8_t prefixlen,
    const char *mac,
    void *client);

#ifdef __cplusplus
}
#endif

#endif // SRC_DRIVERS_NETMON_MSG_SEND_H_

