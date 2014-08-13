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
*/

/*
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>
           Karol Hrdina <karolhrdina@eaton.com>
 
Description: packages netmon's messages to JSON format for controller
References: BIOS-247, BIOS-244
*/

#include <string.h>
#include "jsoncpp/json/json.h"

#include "jsonf.h"

const char *json_pack(const char *event, const char *name, const char *ipver,
                      const char *ipaddr, uint8_t prefixlen, const char *mac) {
  Json::FastWriter wr;
  Json::Value command(Json::arrayValue);
  Json::Value json(Json::objectValue);

  json["module"] = "netmon";

  command.append("network");
  if (strcmp(event, "add") == 0) {
    command.append("add");
  } else {
    command.append("del");
  }
  json["command"] = command;

  Json::Value data(Json::objectValue);

  data["name"]  = name;
  data["ipver"] = ipver;
  data["ipaddr"] = ipaddr;
  data["prefixlen"] = prefixlen;
  data["mac"] = mac;

  json["data"] = data;


  return wr.write(json).c_str();
}
