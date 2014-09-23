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
 
Description: main cored
References: BIOS-248
*/

#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <list>

//#include <jsoncpp/json/json.h>

#include <Variant/Variant.h>
#include <Variant/Schema.h>

#include "main.h"
#include "mock_db.h"
#include "utilities.h"
#include "utilities_zeromq.h"
#include "../utils/messages/json_schemas.h"

void worker(int socket_type, const char *connection, mock_db *db, int id) {
  zmq::context_t context(1);
  zmq::socket_t socket(context, socket_type);
  socket.connect(connection);

  while (true) {
    std::string identity;
    std::string message;

    utils::zeromq::str_recv(socket, identity);
    utils::zeromq::str_recv(socket, message);

#ifndef NDEBUG
    fprintf(stderr, "####\tDEBUG - worker\t####\n");
    fprintf(stderr, "# Received json:\n# %s", message.c_str());
    fprintf(stderr, "\n");
#endif
    
    // decode message
    //Json::Reader reader;
    //Json::Value root(Json::objectValue);
    libvariant::Variant root = libvariant::DeserializeJSON(message);

    // sanity check - we require object with at least "schema" key
    utils::json::MessageTypesEnum message_type;
    if (root.GetType() == libvariant::VariantDefines::MapType &&
        root.Contains("schema") && root["schema"].IsNumber()) {       
      message_type = utils::json::codetable(root["schema"].AsInt());
    } else {
      // TODO log that we received bad message
      printf("ERROR: Message doesn't have 'schema':\n%s\n",
             message.c_str());
      continue;
    }

    std::string strerr;
    utils::json::ValidateResultEnum validation_result;
    switch(message_type) {
      ///////////////////////////////
      ///     CliNetworkAddDel    ///
      ///////////////////////////////
      case utils::json::MessageTypesEnum::CliNetworkAddDel:
      {
        validation_result = validate_parse(message.c_str(), message_type, root, strerr);
        if (validation_result != utils::json::ValidateResultEnum::Valid) {
          // TODO - log error strerr
          printf("ERROR: Validation failed:\nReason:'%s'\nMessage:\n%s\n",
                 strerr.c_str(), message.c_str());
          continue;
        }
        if (root["command"][1].AsString().compare("add") == 0) {
          db->network_add(
            root["data"]["ipver"].AsString(),
            root["data"]["ipaddr"].AsString(),
            root["data"]["prefixlen"].AsInt());
          // TODO generate a back-message later
          /* TBD protocol for cli to expect a response
             Probably something
                {
                 "cmd-result" : 0/1,
                 "message" : string
                }   
          */
        } else if (root["command"][1].AsString().compare("del") == 0) {
          db->network_remove(
            root["data"]["ipver"].AsString(),
            root["data"]["ipaddr"].AsString(),
            root["data"]["prefixlen"].AsInt());
        } else { // should never reach
          // TODO log
          printf("ERROR: Should never reach here MARK_ONE\n"); 
        }    
        break;
      }
      ///////////////////////////////
      ///     CliNetworkList      ///
      ///////////////////////////////
      case utils::json::MessageTypesEnum::CliNetworkList:
      {
        validation_result = validate_parse(message.c_str(), message_type, root, strerr);
        if (validation_result != utils::json::ValidateResultEnum::Valid) {
          // TODO - log error strerr
          printf("ERROR: Validation failed:\nReason:'%s'\nMessage:\n%s\n",
                 strerr.c_str(), message.c_str());
          continue;
        }
        std::list<network_dt> networks;
        db->network_list(networks);
        
        libvariant::Variant json(libvariant::VariantDefines::MapType);
        libvariant::Variant array(libvariant::VariantDefines::ListType);
        json["rc-id"] = 42;
        
        for(auto it = networks.cbegin(); it != networks.cend(); it++) {
          libvariant::Variant item(libvariant::VariantDefines::MapType);
          item["type"] = it->type;
          if (!it->interface.empty()) {
            item["name"] = it->interface;
          } 
          item["ipver"] = it->ipversion;
          item["ipaddr"] = it->ipaddress;
          item["prefixlen"] = it->prefixlen;
          if (!it->macaddress.empty()) {
            item["mac"] = it->macaddress;
          }
          array.Append(item);
        }
        json["networks"] = array;
        json["schema"] = utils::json::enumtable(utils::json::MessageTypesEnum::NetworkList);
        // send it
        utils::zeromq::str_sendmore(socket, identity);
        //utils::zeromq::str_send(socket, wr.write(json));
        utils::zeromq::str_send(socket, libvariant::SerializeJSON(json));

        break;
      }
      //////////////////////////////////
      ///     NetmonNetworkAddDel    ///
      //////////////////////////////////
      case utils::json::MessageTypesEnum::NetmonNetworkAddDel:
      {
        // Validate
        validation_result = validate_parse(message.c_str(), message_type, root, strerr);
        if (validation_result != utils::json::ValidateResultEnum::Valid) {
          // TODO - log error strerr
          printf("ERROR: Validation failed:\nReason:'%s'\nMessage:\n%s\n",
                 strerr.c_str(), message.c_str());
          continue;
        }
        if (root["command"][1].AsString().compare("add") == 0) {
          // add root["data"]
          db->network_insert(
            root["data"]["name"].AsString(),
            root["data"]["ipver"].AsString(), 
            root["data"]["ipaddr"].AsString(), 
            root["data"]["prefixlen"].AsInt(), 
            root["data"]["mac"].AsString());
        } else if (root["command"][1].AsString().compare("del") == 0) {
          // remove root["data"]
          db->network_remove(
            root["data"]["name"].AsString(),
            root["data"]["ipver"].AsString(), 
            root["data"]["ipaddr"].AsString(), 
            root["data"]["prefixlen"].AsInt(), 
            root["data"]["mac"].AsString());
        } else { // should not reach here
          // TODO log
          printf("ERROR: Should never reach here MARK_TWO\n"); 
          continue;
        }    
        break;
      }
      //////////////////////////
      ///     NetworkList    ///
      //////////////////////////
      case utils::json::MessageTypesEnum::NetworkList:
      {
        // TODO log
        printf("Why would anyone send us network list?\n");
        break;
      }
      //////////////////////
      ///     DEFAULT    ///
      //////////////////////
      default:
        printf("ERROR: Unknown message_type '%d'\nMessage:\n%s\n",
               static_cast<int>(message_type),
               message.c_str());
        break;
    }
    continue;    
  }
}

sig_atomic_t quit = false;

int do_proxy(mock_db *db, std::vector<std::thread *> *thread_pool) {
  if (db == NULL) {
    return EXIT_FAILURE;
  }

  zmq::context_t context(1);
  zmq::socket_t front(context, ZMQ_ROUTER);
  zmq::socket_t back(context, ZMQ_DEALER);

  front.bind("tcp://*:5559");
  back.bind("tcp://*:5560");

  for (int i = 0; i < SRV_THREAD_POOL_MAX; i++) {
    thread_pool->push_back(
    new std::thread(worker, ZMQ_DEALER, "tcp://localhost:5560", db, i+1));
    thread_pool->at(i)->detach();
  }

  try {
    zmq::proxy(front, back, nullptr);
  } catch(...) {
    // TODO
    throw;
  }
  return EXIT_SUCCESS;
}

void sig_recv(int signum) {
#ifndef NDEBUG
  fprintf(stderr, "\n####\tDEBUG - sig_recv()\t####\n");
  fprintf(stderr, "# RECEIVED SIGNAL: %d\n", signum);
  fprintf(stderr, "\n");
#endif
  quit = true;
}

int main(int argc, char **argv) {

  // register for SIGINT
  struct sigaction sig_action;
  memset(&sig_action, 0, sizeof(sig_action));
  sig_action.sa_handler = sig_recv;
  sigfillset(&sig_action.sa_mask);
  sigaction(SIGINT, &sig_action, nullptr);
  
  
  // -h --help help usage
  if (argc >= 2 && (strcmp(argv[1], "-h") == 0 ||
                    strcmp(argv[1], "--help") == 0 ||
                    strcmp(argv[1], "help") == 0 ||
                    strcmp(argv[1], "usage") == 0)) {
    utils::print_usage();
    return EXIT_SUCCESS;
  }             

  pid_t pid = 0;
  switch(pid = fork()) {
    case -1:
      fprintf(stderr, "ERROR: Forking failed\n");
      return EXIT_FAILURE;      
      break;
    case 0: // child      
      {
        setenv("ZMQ_BUS", "tcp://localhost:5559", 1);
#ifdef DEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Starting ./netmon module\n");
        fprintf(stderr, "\n");
#endif
#ifdef DEVEL
        std::string path = getenv("PATH");
        setenv("PATH", ".", 1);
#endif
        int ret = std::system("netmon");
#ifdef DEVEL
        setenv("PATH", path.c_str(), 1);
#endif
#ifdef DEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Module ./netmon finished with return value '%d'\n", ret);
        fprintf(stderr, "\n");
#endif
      }
      break; // break child
    default: // parent
      {
        mock_db db;
#ifdef DEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Starting server thread\n");
        fprintf(stderr, "\n");
#endif
        std::vector<std::thread *> thread_pool;
        std::thread server(do_proxy, &db, &thread_pool);
#ifdef DEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Server thread started\n");
        fprintf(stderr, "\n");
#endif
        // TODO
        // process (not thread, and if, then only one specific) receives
        // the signal. This is here for the cleanup. Needs to be reworked.
        while (1) {
          sleep(1);
          if (quit) {
            // Dump the DB
#ifdef DEBUG
            fprintf(stderr, "####\tDEBUG - main()\t####\n");
            fprintf(stderr, "# Dumping db -> ./db_dump\n");
            fprintf(stderr, "\n");
#endif
            std::ofstream f("./db_dump", std::ios_base::app);
            std::list<network_dt> networks;
            db.network_list(networks);
            f << "size: " << networks.size() << std::endl;
            std::string sprefixlen;            
            for (auto it = networks.begin(); it != networks.end(); it++) {              
              f << it->type << " " << it->interface << " " << it->ipversion
               << " " << it->ipaddress << " " << (int) it->prefixlen << " " <<
               it->macaddress << std::endl;
            }
            f.close();
            // Cleanup
#ifdef DEBUG
            fprintf(stderr, "###\tDEBUG - main()\t####\n");
            fprintf(stderr, "# Freeing thread number:\n");
#endif
            for (int i = 0; i < SRV_THREAD_POOL_MAX; i++) {
              if (thread_pool.at(i) != nullptr) {
#ifdef DEBUG
                fprintf(stderr, "#\t%d\n", i);
#endif
                delete thread_pool.at(i);
              }
              thread_pool.at(i) = nullptr;
            }
#ifdef DEBUG
            fprintf(stderr, "\n");
#endif
            break;
          }
        }
      }
      break; // break parent
  }

  // TODO Commands need to be accepted from
  //      -  main cmdline - probably dispatch a thread 
  //      -  CLI - MVY 
  //      -  possibly REST API - N/A atm
  return EXIT_SUCCESS;
}
