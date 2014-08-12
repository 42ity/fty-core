/* 
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
Author: Karol Hrdina <karolhrdina@eaton.com>
 
Description: netmon module mock implementation
References: BIOS-248
*/
#ifndef MOCK_NETMON_H_
#define MOCK_NETMON_H_

#define IFCOUNT 4 // number of interfaces to generate
#define IPVERSION_IPV4_STR     "IPV4"
#define IPVERSION_IPV6_STR     "IPV6"
#define EVENT_NETWORK_ADD_STR  "add"
#define EVENT_NETWORK_DEL_STR  "del"

namespace mocks {

/*!
\brief Mock the netmon module behaviour.
       Send an event every (min, max) seconds.

Currently sends the following events: ADD, REMOVE
A std::map keeps track of added networks in order to have realistic REMOVE
events. In case an adrress is generated that is already present, the cycle is
simply skipped.

\param[in]	min	minimum number of seconds to sleep before sending next event
\param[in]	max	maximum number of seconds to sleep before sending next event
\param[in]	connection zeromq endpoint, format [transport]://[address]
*/
void netmon(short min, short max, const char *connection);

} // namespace mocks

#endif // MOCK_NETMON_H_

