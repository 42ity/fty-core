/* zmq.c: json backend for netmon
 
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
 
 
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>
 
Description: packages netmon's messages to JSON format for controller
References: BIOS-247, BIOS-244
*/

#include <iostream>
#include <sstream>
#include <cstdio>

#include "jsoncpp/json/json.h"

#include "jsonf.h"

#ifdef __cplusplus
extern "C"
{
#endif
const char *json_pack(
       const char *event,
       const char *name,
       const char *ipver,
       const char *ipaddr,
       int prefixlen,
       const char *mac) {

     //TODO: check why Json::FastWriter() does not work!
     Json::FastWriter wr{};
     Json::Value arr(Json::arrayValue);
     Json::Value entry(Json::objectValue);

     entry["event"] = event;
     entry["name"]  = name;
     entry["ipver"] = ipver;
     entry["ipaddr"] = ipaddr;
     entry["prefixlen"] = prefixlen;
     entry["mac"] = mac;

     arr.append(entry);

     /*
     std::basic_ostringstream<char> os;
     os << arr;
     return os.str().c_str();
     */
     
     return wr.write(arr).c_str();

}
#ifdef __cplusplus
}
#endif


/*TODO: move to test ...
int main() {

    puts(json_pack(
        "ADDRADD",
        "enp0s25",
        "IPv4",
        "10.130.38.147/24",
        "a0:1d:48:b7:e2:4e"
        ));

}
*/
