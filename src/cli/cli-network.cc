/* cli-network.c: command line interface - network command
 
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
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>,
           Karol Hrdina <karolhrdina@eaton.com>
 
Description: network CLI command
References: BIOS-245, BIOS-126
*/

#include <getopt.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <jsoncpp/json/json.h>
#include <zmq.h>

#include "log.h"
#include "utils.h"
#include "cli.h"

#define ZMQ_RECV_BUFFER_SIZE  1024
#define ZMQ_BUS_DEFAULT   "tcp://localhost:5559"

/* IMPLEMENTATION SPECIFICS

`network list` expected json:
{
  "RD-ID"     : rc-identifier,
  "networks"  :
    [{
      "type"      :   type-value,
      "name"      :   name-value,
      "ipver"     :   ipver-value,
      "ipaddr"    :   ipaddr-value,
      "prefixlen" :   prefixlen,
      "mac"       :   mac-value
     },...]
}

`network add` json sent:
{
  "module"  : module-value,
  "command" : [ "network", "add" ],
  "data"    : {
    "ipver"     :   ipver-value,
    "ipaddr"    :   ipaddr-value,
    "prefixlen" :   prefixlen
  }
}

`network remove` json sent:
{
  "module"  : module-value,
  "command" : [ "network", "del" ],
  "data"    : {
    "ipver"     :   ipver-value,
    "ipaddr"    :   ipaddr-value,
    "prefixlen" :   prefixlen
  }
}
*/

struct network_opts {
    bool show_details;
};

static struct network_opts opts = {
    .show_details = false
};

static struct option long_options[] = {
    /* These options set a flag. */
    {"details", no_argument,       0, 'd'},
    {0, 0, 0, 0}
};

static void do_network_help() {
    fputs("cli network {list|add|remove} command-arguments\n", stderr);
    fputs("\n", stderr);
    fputs("            list - list of networks to monitor\n", stderr);
    fputs("            add ip-address/prefixlen - add new network range to monitor\n", stderr);
    fputs("            remove ip-address/prefixlen - remove network range from monitoring\n", stderr);
}

static const char* str_enum_ValueType(Json::ValueType vt) {
    switch (vt) {
        case Json::ValueType::nullValue:
            return "'null' value"; break;
        case Json::ValueType::intValue:
            return "signed integer value"; break;
        case Json::ValueType::uintValue:
            return "unsigned integer value"; break;
        case Json::ValueType::realValue:
            return "double value"; break;
        case Json::ValueType::stringValue:
            return "UTF-8 string value"; break;
        case Json::ValueType::booleanValue:
            return "bool value"; break;
        case Json::ValueType::arrayValue:
            return "array value (ordered list)"; break;
        case Json::ValueType::objectValue:
            return "object value (collection of name/value pairs)"; break;
    }
}

static char char_type(Json::Value &v) {

    if (v == "AUTOMATIC") {
        return 'A';
    }
    else if (v == "DELETED") {
        return 'D';
    }
    else if (v == "MANUAL") {
        return 'M';
    }
    return '?';
}

static bool json_pack(const char **gargv, int argc,  std::string& result) {

  char *const *argv = (char *const*) gargv;
  Json::FastWriter wr;
  Json::Value command(Json::arrayValue);
  Json::Value json(Json::objectValue);

  // TODO KHR (?) : All of these magical constants
  json["module"] = "cli";
  command.append("network");

  ////  LIST  ////
  if (streq(argv[optind], "list")) {
    command.append("list");
    json["data"] = Json::Value(Json::objectValue);
  ////  ADD, REMOVE   ////
  } else if(streq(argv[optind], "add") ||
            streq(argv[optind], "remove")) {
    Json::Value data(Json::objectValue);
    if (optind + 1 == argc) {
      fprintf(stderr, "Too few arguments.\n");
      return false;
    }
    std::string argument(argv[optind+1]);
    std::size_t index = argument.find('/');
    if (index == std::string::npos) {
      fprintf(stderr, "unknown command line argument '%s'. CIDR format expected.\n",
              argv[optind+1]);
      return false;
    }
    data["ipver"] = "IPV4";
    data["ipaddr"] = argument.substr(0, index);
    data["prefixlen"] = argument.substr(index + 1, argument.length());
    json["data"] = data;
    if (streq(argv[optind], "add")) {  
      command.append("add");
    } else {
      command.append("del");
    }
  ////  ERROR - UNKNOWN SUB-COMMAND  ////
  } else {
    fprintf(stderr, "unknown sub-command '%s'\n",
            argv[optind]);
    return false;
  }
  json["command"] = command;
  result.assign(wr.write(json));
  return true;
}

static int
do_network_list(
FILE *stream, const char* message, const struct global_opts *gopts) {
	assert(stream);
  if (stream == NULL) {
    log_error("stream empty\n");
    return EXIT_FAILURE;
  }

  Json::Reader rd;
  Json::Value root{Json::ValueType::arrayValue};
  bool res = rd.parse(message, root, false);
  if (res == false) {
      log_error("Error parsing json message:\n%s\n",
              rd.getFormattedErrorMessages().c_str());
      return EXIT_FAILURE;
  }

  assert(root.isObject());

  for (auto rcid : root.getMemberNames()) {    
    fputs(rcid.c_str(), stdout);
    fputs("\n", stdout);

    for (auto entry : root[rcid]) {
      assert(entry.isObject());
      fprintf(stdout,
              "\t%c %s/%d %s %s\n",
              char_type(entry["type"]),
              entry["ipaddr"].asCString(),
              entry["prefixlen"].asInt(),
              entry["name"].asCString(),
              entry["mac"].asCString());
      }
  }
  return EXIT_SUCCESS;
}

int
do_network(
const int argc, const char **gargv, const struct global_opts *gopts) {

  char *const *argv = (char *const*) gargv;
  int option_index = 0;
  int c = 0;
  // command line argument parsing
  while (c != -1) {
    c = getopt_long(argc, argv, "d", long_options, &option_index);
    switch (c) {
      case -1:
        continue;
        break;
      case 'd':
        opts.show_details = true;
        break;
      case '?':
        /* getopt_long already printed an error message. */
        break;
      default:
        return -1;
    }
  }

#ifndef NDEBUG
  log_debug("####\tdo_network()\t####\n");
  log_debug("# argc: '%d'\n# optind: '%d'\n", argc, optind);
  log_debug("# **argv:\n");  
  for (int i = 0; i < argc; i++) {
    log_debug("#\t[ %d ]\t%s\n", i, argv[i]);
  }
  log_debug("# non-option ARGV-elements:\n");
  {
    int i = optind;
    while (i < argc) {
      log_debug("#\t%s\n", argv[i++]);
    }
  }
  log_debug("\n");
#endif

  if (optind >= argc) {
    do_network_help();
    return EXIT_FAILURE;
  }

  // TODO ? (?) : think of a more robust way of doing this
  // For now I am just passing the smelly turd along ;)
  std::string ZMQ_BUS;
  const char *zmq_bus_env = getenv("ZMQ_BUS");
  if (zmq_bus_env == NULL || strlen(zmq_bus_env) == 0) {    
    ZMQ_BUS.assign(ZMQ_BUS_DEFAULT);
    log_info("environmental variable ZMQ_BUS not set. Defaulting to '%s'\n",
            ZMQ_BUS.c_str());    
  } else {
    ZMQ_BUS.assign(zmq_bus_env);
  }

  void *context = NULL;
  context = zmq_ctx_new();
  if (context == NULL) {
    log_error("zmq_ctx_new() failed: %m.\n");
    return EXIT_FAILURE; // TODO KHR (?) : maybe create defines for various errors?
  }
  void *socket = NULL;
  socket = zmq_socket(context, ZMQ_DEALER);
  if (socket == NULL) {
    log_error("zmq_socket() failed: %m.\n");
    return EXIT_FAILURE;
  }
  int ret = zmq_setsockopt(socket, ZMQ_IDENTITY, "cli", strlen("cli")); // hmmptf  
  if (ret == -1) {
    log_error("zmq_setsockopt() failed, errno ='%d': %m\n", zmq_errno());
    return EXIT_FAILURE;
  }  
  ret =  zmq_connect(socket, ZMQ_BUS.c_str());
  if (ret == -1) {
    log_error("zmq_connect() failed: %m.\n");
    return EXIT_FAILURE;
  }

  std::string message;
  if (!json_pack(gargv, argc, message)) {
    return EXIT_FAILURE;    
  }
#ifndef NDEBUG
  log_debug("\n####do_network()\t####\n");
  log_debug("# Sending json:\n# %s\n", message.c_str());
#endif
  ret = zmq_send(socket, message.c_str(), message.size(), 0); //(int) ZMQ_DONTWAIT);  
/* Unfortunatelly, i wasn't able to finish this successfully
The idea is to have cli know if cored is not boud; there is a possibility
to stage the message for sending without blocking and read number of bytes and
return define/enum... however in the time given, i wasn't able to make it work... TBD
  fprintf(stderr, "ret = %d\n", ret);
  if (ret == -1) {
    if (zmq_errno() == EAGAIN) {
      printf("cored is not running on the specified connection.\n");
      return EXIT_SUCCESS;
    }
    fprintf(stderr, "zmq_send() failed.\n");
    return EXIT_FAILURE;
  }
*/
  if (streq(argv[optind], "list")) {
    char buffer[ZMQ_RECV_BUFFER_SIZE+1];
    ret = zmq_recv(socket, buffer, ZMQ_RECV_BUFFER_SIZE, 0);
    buffer[ZMQ_RECV_BUFFER_SIZE] = '\0';
    if (ret > ZMQ_RECV_BUFFER_SIZE) {
      log_warning("buffer too small and the message was truncated.\n");
    } else if (ret == -1) {    
      log_error("zmq_recv failed: %m.\n");
      return EXIT_FAILURE;
    }
    return do_network_list(stdout, buffer, gopts);
  }
  zmq_close(socket);
  zmq_ctx_destroy(context);
  return EXIT_SUCCESS;
}
