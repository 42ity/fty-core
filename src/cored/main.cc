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

#include <jsoncpp/json/json.h>

#include "main.h"
#include "mock_db.h"
#include "utilities.h"
#include "utilities_zeromq.h"

void worker(int socket_type, const char *connection, mock_db *db, int id) {
  zmq::context_t context(1);
  zmq::socket_t socket(context, socket_type);
  socket.connect(connection);

  while (true) {
    std::string identity;
    std::string message;

    utils::zeromq::str_recv(socket, identity);
    utils::zeromq::str_recv(socket, message);
    
    // decode
    Json::Reader reader;
    Json::Value root(Json::objectValue);
    //                                              collectComments
    bool parse_result = reader.parse(message, root, false);
    if (parse_result == false ||
        root.isObject() == false ||
        root["module"].empty() == true || 
        root["module"].isString() == false ||
        root["command"].empty() == true ||
        root["command"].isArray() == false || 
//        root["data"].empty() == true || // TODO this had to be take out due to cli
        root["data"].isObject() == false) {
      PRINTF_STDERR("Parsing json failed or unexpected json format: %s\n",
                    reader.getFormattedErrorMessages().c_str())
      PRINTF_STDERR("message:\n%s\n", message.c_str());
      // send empty
      utils::zeromq::str_sendmore(socket, identity);
      utils::zeromq::str_send(socket, "bad json");
      continue;
    }
#ifndef NDEBUG
    fprintf(stderr, "####\tDEBUG - worker\t####\n");
    fprintf(stderr, "# Received json:\n# %s", message.c_str());
    fprintf(stderr, "\n");
#endif
    
    std::string module = root["module"].asString();
    Json::Value command(Json::arrayValue);
    command = root["command"];
    Json::Value data(Json::objectValue);
    data = root["data"];
    std::string cmd_main = command[0].asString();

    // TODO KHR(?): this if () {} else if () ... is fugly; needs to be rewritten
    //   enum, function(str->int), or transport int in json (have global codetable)
    
    std::transform(cmd_main.begin(), cmd_main.end(), cmd_main.begin(), tolower);
    std::transform(module.begin(), module.end(), module.begin(), tolower);    
    if (module.compare("cli") == 0) {
      /////////////////////////////////
      //            CLI              //
      /////////////////////////////////
      if (command.size() >= 2 && cmd_main.compare("network") == 0) {
        // TODO validate fields ipver, ipaddr, prefixlen        
        std::string cmd_what = command[1].asString();
        std::transform(cmd_what.begin(), cmd_what.end(), cmd_what.begin(), tolower);
        if (cmd_what.compare("add") == 0) {
          int prefixlen = atoi(root["data"]["prefixlen"].asString().c_str());
          db->network_add(root["data"]["ipver"].asString(),
                          root["data"]["ipaddr"].asString(), 
                          prefixlen);
          // TODO generate a back-message later
          /* TBD protocol for cli to expect a response
             Probably something
                {
                 "cmd-result" : 0/1,
                 "message" : string
                }   
          */
           
        } else if (cmd_what.compare("del") == 0) {
          int prefixlen = atoi(root["data"]["prefixlen"].asString().c_str());
          db->network_remove(root["data"]["ipver"].asString(), 
                             root["data"]["ipaddr"].asString(), 
                             prefixlen); 
        } else if (cmd_what.compare("list") == 0) {
          std::list<network_dt> networks;
          db->network_list(networks);
          Json::FastWriter wr;
          Json::Value json(Json::objectValue);
          Json::Value array(Json::arrayValue);
          json["RC-ID"] = "42";
          
          for(auto it = networks.cbegin(); it != networks.cend(); it++) {
            Json::Value item(Json::objectValue);
            item["type"] = it->type;
            item["name"] = it->interface;
            item["ipver"] = it->ipversion;
            item["ipaddr"] = it->ipaddress;
            item["prefixlen"] = it->prefixlen;
            item["mac"] = it->macaddress;
            array.append(item);
          }
          json["networks"] = array;
          // send it
          utils::zeromq::str_sendmore(socket, identity);
          utils::zeromq::str_send(socket, wr.write(json));
        }            
      }
    } else if (module.compare("netmon") == 0) {    
      /////////////////////////////////
      //           NETMON            //
      /////////////////////////////////

      if (command.size() >= 2 && cmd_main.compare("network") == 0) {      
        std::string cmd_what = command[1].asString();
        std::transform(cmd_what.begin(), cmd_what.end(), cmd_what.begin(),
                       tolower);
        if (cmd_what.compare("add") == 0) {
          // add root["data"]
          db->network_insert(root["data"]["name"].asString(),
                             root["data"]["ipver"].asString(), 
                             root["data"]["ipaddr"].asString(), 
                             root["data"]["prefixlen"].asInt(), 
                             root["data"]["mac"].asString());
        } else if (cmd_what.compare("del") == 0) {
          // remove root["data"]
          db->network_remove(root["data"]["name"].asString(),
                             root["data"]["ipver"].asString(), 
                             root["data"]["ipaddr"].asString(), 
                             root["data"]["prefixlen"].asInt(), 
                             root["data"]["mac"].asString());
        } else {
          PRINTF_STDERR("Unexpected json format. '%s' is not a recognized command\n",
                        command.toStyledString().c_str())
          utils::zeromq::str_sendmore(socket, identity);
          utils::zeromq::str_send(socket, "bad api - unknown 'command'");
          continue;
        }        
      } // TODO KHR(?): should we PRINTF_STDERR here? 
    } else {
      /////////////////////////////////
      //       ERROR - UNKNOWN       //
      /////////////////////////////////
      PRINTF_STDERR("Unexpected json format. 'module'='%s' is unknown\n",
                    module.c_str());
      utils::zeromq::str_sendmore(socket, identity);
      utils::zeromq::str_send(socket, "bad json - unknown 'module'");
      continue;
    }
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
#ifndef NDEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Starting ./netmon module\n");
        fprintf(stderr, "\n");
#endif
        int ret = std::system("./netmon");
#ifndef NDEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Module ./netmon finished with return value '%d'\n", ret);
        fprintf(stderr, "\n");
#endif
      }
      break; // break child
    default: // parent
      {
        mock_db db;
#ifndef NDEBUG
        fprintf(stderr, "####\tDEBUG - main()\t####\n");
        fprintf(stderr, "# Starting server thread\n");
        fprintf(stderr, "\n");
#endif
        std::vector<std::thread *> thread_pool;
        std::thread server(do_proxy, &db, &thread_pool);
#ifndef NDEBUG
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
#ifndef NDEBUG
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
#ifndef NDEBUG
            fprintf(stderr, "###\tDEBUG - main()\t####\n");
            fprintf(stderr, "# Freeing thread number:\n");
#endif
            for (int i = 0; i < SRV_THREAD_POOL_MAX; i++) {
              if (thread_pool.at(i) != nullptr) {
#ifndef NDEBUG
                fprintf(stderr, "#\t%d\n", i);
#endif
                delete thread_pool.at(i);
              }
              thread_pool.at(i) = nullptr;
            }
#ifndef NDEBUG
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
