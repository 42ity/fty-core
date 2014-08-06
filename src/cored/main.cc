#include <stdlib.h>
//#include <limits.h>
#include <thread>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iostream>

#include "main.h"
#include "mock_netmon.h"
#include "mock_db.h"
#include "utilities.h"
#include "utilities_zeromq.h"

void worker(int socket_type, const char *connection,
            uint8_t id) {
  zmq::context_t context(1);
  zmq::socket_t socket(context, socket_type);
  socket.connect(connection);
  
  while (true) {    
    zmq::message_t identity;
    zmq::message_t message;
    socket.recv(&identity);
    socket.recv(&message);
    
    // TODO: 
    // decode
    // store/remove/list
    // socket.send(identity, ZMQ_SNDMORE);
    // socket.send(message);
    
    char fname[20];
    snprintf(fname, 20, "./worker%d", id);
    std::ofstream f(fname, std::ios_base::app);
    snprintf(fname, 20, "%d", id);
    f << "Worker " << fname << "::"
    << std::string(static_cast<char*>(identity.data()), identity.size()) << "::"
    << std::string(static_cast<char*>(message.data()), message.size()) << std::endl;
    f.close();

  }
}

int main(int argc, char **argv) {
  // -h --help help usage
  if (argc >= 2 && (strcmp(argv[1], "-h") == 0 ||
                    strcmp(argv[1], "--help") == 0 ||
                    strcmp(argv[1], "help") == 0 ||
                    strcmp(argv[1], "usage") == 0)) {
    utils::print_usage();
    return EXIT_SUCCESS;
  }             

  // deal with cmdline arguments
  int netmon_min_sleep = 0, netmon_max_sleep = 0;
  if (argc >= 3) {
    netmon_min_sleep = atoi(argv[1]);
    netmon_max_sleep = atoi(argv[2]);
  }

  if (argc < 3 || netmon_min_sleep >= netmon_max_sleep) {    
    netmon_min_sleep = MOD_NETMON_DEFAULT_MIN;
    netmon_max_sleep = MOD_NETMON_DEFAULT_MAX;
    printf("%s\n", MSG_INF_DEFAULT_SLEEP_NETMON);
  }

  // zeromq initialization
  zmq::context_t context(1);
  zmq::socket_t front(context, ZMQ_ROUTER);  
  zmq::socket_t back(context, ZMQ_DEALER);

  // use 'bind' as this is a permanent connection point
  front.bind("tcp://*:5559");
  back.bind("tcp://*:5560");
  
  zmq_pollitem_t items[] = {
    { front, 0, ZMQ_POLLIN, 0 },
    { back, 0, ZMQ_POLLIN, 0}
  };

  std::thread worker1(worker, ZMQ_DEALER, "tcp://localhost:5560", 1);
  std::thread worker2(worker, ZMQ_DEALER, "tcp://localhost:5560", 2);
  
  //////////////////////////////
  //    Start NETMON module   //
  //////////////////////////////
  std::thread netmon(mocks::netmon, netmon_min_sleep, netmon_max_sleep,
                     "tcp://localhost:5559");  
  printf("netmon started\n");



  // TODO KHR(5.8): return values are not checked, do it
  while (true) {
    zmq::message_t zmsg;
    int64_t more; // multipart detection

    zmq::poll(&items[0], 2, -1);

    if (items[0].revents & ZMQ_POLLIN) { // ZMQ_POLLIN event set in revents on front
      while (1) { // Process all parts of the message
        front.recv(&zmsg);
        // debug
        printf("%s\n", std::string(static_cast<char*>(zmsg.data()), zmsg.size()).c_str());
        size_t more_size = sizeof(more);
        front.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        back.send(zmsg, more ? ZMQ_SNDMORE : 0);        
        if (!more) {
          break; // last message part
        }
      }
    }    
    if (items[1].revents & ZMQ_POLLIN) { // ZMQ_POLLIN event set in revents on back
      while (1) { // Process all parts of the message
        back.recv(&zmsg);
        size_t more_size = sizeof (more);        
        back.getsockopt(ZMQ_RCVMORE, &more, &more_size);
        front.send(zmsg, more? ZMQ_SNDMORE: 0);
        if (!more) {
          break; // last message part
        }
      }      
    }
  }  
  
  // TODO Commands need to be accepted from
  //      -  main cmdline - probably dispatch a thread 
  //      -  CLI - MVY 
  //      -  possibly REST API - N/A atm

  netmon.join();

  return EXIT_SUCCESS;
}
