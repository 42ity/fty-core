/* 
Copyright (C) 2014 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   cli-network.c
    \brief  command line interface - network command
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#include <getopt.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string.h>

#include <zmq.h>

#include "log.h"
#include "cli.h"
#include "../utils/messages/json_schemas.h"
#include "../utils/utils.h"

#define ZMQ_RECV_BUFFER_SIZE  1024
#define ZMQ_BUS_DEFAULT   "tcp://localhost:5559"
#define NETWORK_LIST_CMD_FORMAT \
  " | %c | %-39s | %-10s | %-17s\n"

#define NETWORK_TYPE_AUTOMATIC "automatic"
#define NETWORK_TYPE_DELETED   "deleted"
#define NETWORK_TYPE_MANUAL    "manual"

#define IPADDRESS_VER_IPV4  "ipv4"
#define IPADDRESS_VER_IPV6  "ipv6"


using namespace utils::json;

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
    fputs("cli network {show|add|remove} command-arguments\n", stderr);
    fputs("\n", stderr);
    fputs("            show - list of networks to monitor\n", stderr);
    fputs("            add ip-address/prefixlen - add new network range to monitor\n", stderr);
    fputs("            del ip-address/prefixlen - remove network range from monitoring\n", stderr);
}


static char network_type_to_char(const char *string) {
    if (strcmp(string, NETWORK_TYPE_AUTOMATIC) == 0) {
        return 'A';
    } else if (strcmp(string, NETWORK_TYPE_DELETED) == 0) {
        return 'D';
    } else if (strcmp(string, NETWORK_TYPE_MANUAL) == 0) {
        return 'M';
    }
    return '?';
}

static bool json_pack(const char **gargv, int argc,  std::string& result) {

  char *const *argv = (char *const*) gargv;

  libvariant::Variant command(libvariant::VariantDefines::ListType);
  libvariant::Variant json(libvariant::VariantDefines::MapType);

  json["module"] = "cli";
  command.Append("network");

  ////  LIST  ////
  if (argv[optind] && strcmp(argv[optind], "show") == 0) {
    json["schema"] = utils::json::enumtable(
      utils::json::MessageTypesEnum::CliNetworkList);
    command.Append("list");    
    json["command"] = command;
  ////  ADD, REMOVE   ////
  } else if(argv[optind] &&
            (strcmp(argv[optind], "add") == 0 || strcmp(argv[optind], "del") == 0)) {
    libvariant::Variant data(libvariant::VariantDefines::MapType);
    json["schema"] = utils::json::enumtable(
      utils::json::MessageTypesEnum::CliNetworkAddDel);
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
    data["ipver"] = IPADDRESS_VER_IPV4;
    data["ipaddr"] = argument.substr(0, index);
    data["prefixlen"] = atoi(argument.substr(index + 1, argument.length()).c_str());
    json["data"] = data;
    if (argv[optind] && strcmp(argv[optind], "add") == 0) {  
      command.Append("add");      
    } else {
      command.Append("del");
    }
  ////  ERROR - UNKNOWN SUB-COMMAND  ////
  } else {
    fprintf(stderr, "unknown sub-command '%s'\n",
            argv[optind]);
    return false;
  }
  json["command"] = command;
  result.assign(libvariant::SerializeJSON(json));
  return true;
}

static int
do_network_show(
FILE *stream, const char* message, const struct global_opts *gopts) {
	assert(stream);
  if (stream == NULL) {
    log_error("stream empty");
    return EXIT_FAILURE;
  }

  libvariant::Variant data(libvariant::VariantDefines::ListType);

  // validate
  std::string strerr;
  libvariant::Variant root;
  ValidateResultEnum validation_result =
  validate_parse(message, MessageTypesEnum::NetworkList, root, strerr);

  // TODO log - write something when message invalid
  if (validation_result != ValidateResultEnum::Valid) {
    printf("ERROR:\n%s\n", strerr.c_str());
    printf("Message:\n%s\n", message);
    return EXIT_FAILURE;
  }  

/*
  bool res = rd.parse(message, root, false);
  if (res == false) {
      log_error("Error parsing json message:\n%s",
              rd.getFormattedErrorMessages().c_str());
      return EXIT_FAILURE;
  }
*/
  //assert(root.isObject());
  
  std::string name, mac;
  fprintf(stdout, "rc-id:\t%s\n", std::to_string(root["rc-id"].AsInt()).c_str());
  for (unsigned int i = 0; i < root["networks"].Size(); ++i) {
     
     if (root["networks"][i].Contains("name")) {
      name.assign( root["networks"][i]["name"].AsString());
     } else {
      name.assign("");
     }
     if (root["networks"][i].Contains("mac")) {
      mac.assign( root["networks"][i]["mac"].AsString());
     } else {
      mac.assign("");
     }
     fprintf(stdout,
            NETWORK_LIST_CMD_FORMAT,
            network_type_to_char(root["networks"][i]["type"].AsString().c_str()),
            root["networks"][i]["ipaddr"].AsString().append("/").append(
              std::to_string(root["networks"][i]["prefixlen"].AsInt())
            ).c_str(),            
            name.c_str(),
            mac.c_str());   
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
  log_debug("####\tdo_network()\t####");
  log_debug("# argc: '%d'\n# optind: '%d'", argc, optind);
  log_debug("# **argv:");  
  for (int i = 0; i < argc; i++) {
    log_debug("#\t[ %d ]\t%s", i, argv[i]);
  }
  log_debug("# non-option ARGV-elements:");
  {
    int i = optind;
    while (i < argc) {
      log_debug("#\t%s", argv[i++]);
    }
  }
  log_debug("");
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
    log_info("environmental variable ZMQ_BUS not set. Defaulting to '%s'",
            ZMQ_BUS.c_str());    
  } else {
    ZMQ_BUS.assign(zmq_bus_env);
  }

  void *context = NULL;
  context = zmq_ctx_new();
  if (context == NULL) {
    log_error("zmq_ctx_new() failed: %m.");
    return EXIT_FAILURE; // TODO KHR (?) : maybe create defines for various errors?
  }
  void *socket = NULL;
  socket = zmq_socket(context, ZMQ_DEALER);
  if (socket == NULL) {
    log_error("zmq_socket() failed: %m.");
    return EXIT_FAILURE;
  }
  int ret = zmq_setsockopt(socket, ZMQ_IDENTITY, "cli", strlen("cli")); // hmmptf  
  if (ret == -1) {
    log_error("zmq_setsockopt() failed, errno ='%d': %m", zmq_errno());
    return EXIT_FAILURE;
  }  
  ret =  zmq_connect(socket, ZMQ_BUS.c_str());
  if (ret == -1) {
    log_error("zmq_connect() failed: %m.");
    return EXIT_FAILURE;
  }

  std::string message;
  if (!json_pack(gargv, argc, message)) {
    return EXIT_FAILURE;    
  }
#ifndef NDEBUG
  log_debug("\n####do_network()\t####");
  log_debug("# Sending json:\n# %s", message.c_str());
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
  if (argv[optind] && strcmp(argv[optind], "show") == 0) {
    char buffer[ZMQ_RECV_BUFFER_SIZE+1];
    ret = zmq_recv(socket, buffer, ZMQ_RECV_BUFFER_SIZE, 0);
    buffer[ZMQ_RECV_BUFFER_SIZE] = '\0';
    if (ret > ZMQ_RECV_BUFFER_SIZE) {
      log_warning("buffer too small and the message was truncated.");
    } else if (ret == -1) {    
      log_error("zmq_recv failed: %m.");
      return EXIT_FAILURE;
    }
    return do_network_show(stdout, buffer, gopts);
  }
  zmq_close(socket);
  zmq_ctx_destroy(context);
  return EXIT_SUCCESS;
}
