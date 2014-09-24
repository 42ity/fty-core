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
#include <Variant/Variant.h>
#include <Variant/Schema.h>

#include "json_schemas.h"

#include "jsonf.h"

const char *json_pack(const char *event, const char *name, const char *ipver,
                      const char *ipaddr, uint8_t prefixlen, const char *mac) {
  //Json::FastWriter wr;
  //Json::Value command(Json::arrayValue);
  libvariant::Variant command(libvariant::VariantDefines::ListType);
  //Json::Value json(Json::objectValue);
  libvariant::Variant json(libvariant::VariantDefines::MapType);

  json["module"] = JP_MODULE;

  //command.append("network");
  command.Append("network");
  if (strcmp(event, JP_EVENT_ADD) == 0) {
    command.Append(JP_EVENT_ADD);
  } else {
    command.Append(JP_EVENT_DEL);
  }
  json["command"] = command;

  //Json::Value data(Json::objectValue);
  libvariant::Variant data(libvariant::VariantDefines::MapType);

  data["name"]  = name;
  data["ipver"] = ipver;
  data["ipaddr"] = ipaddr;
  data["prefixlen"] = prefixlen;
  data["mac"] = mac;

  json["data"] = data;

  // Don't forget to self-describe the message
  json["schema"] = utils::json::enumtable(
    utils::json::MessageTypesEnum::NetmonNetworkAddDel);


  //return strdup(wr.write(json).c_str());
  return strdup(libvariant::SerializeJSON(json).c_str());
}
