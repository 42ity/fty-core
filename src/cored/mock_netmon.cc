/*
WARNING:
This code uses the following "shortcut" to format std::strings:

using snprintf to format the buffer of std::string directly

This works only in c++11. Rewrite this if using anything older!
C++11 guarantess that the internal buffer of std::basic_string<char>
lies in a continuous memory segment.
*/
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <string>
#include <thread>
#include <map>
#include <utility>
#include <jsoncpp/json/json.h>

#include "utilities_zeromq.h"
#include "mock_netmon.h"

// mock_netmon specific defines
#define MACADDR_LENGTH  18
#define IPADDR_LENGTH   15
#define MOCK_NETMON_ADD 1
#define MOCK_NETMON_DEL 0
#define ZMQ_IDENTITY_LENGTH 10
#define ZMQ_IO_THREADS_NUM  1

//! Package network data into a json encoded string
static bool json_pack(
    const char *event, const char *interface, const char *ipversion,
    const char *ipaddress, uint8_t prefixlen, const char *macaddress,
    std::string& string) {
  Json::FastWriter wr{};
  Json::Value arr(Json::arrayValue);
  Json::Value entry(Json::objectValue);

  entry["event"] = event;
  entry["name"]  = interface;
  entry["ipver"] = ipversion;
  entry["ipaddr"] = ipaddress;
  entry["prefixlen"] = prefixlen;
  entry["mac"] = macaddress;

  arr.append(entry);

  string.assign(wr.write(arr));
  return true;
}

//! generate a pseudo-random MAC address string
static void gen_macaddr(std::string& mac) {
  mac.resize(MACADDR_LENGTH);
  int bytes = snprintf(&mac[0],
                       MACADDR_LENGTH,
                       "%02x:%02x:%02x:%02x:%02x:%02x",
                       rand() % 256, rand() % 256, rand() % 256,
                       rand() % 256, rand() % 256, rand() % 256);
}

//! generate a pseudo-random IP address string
static void gen_ipaddr(std::string& ip) {
  ip.resize(IPADDR_LENGTH);
  int bytes = snprintf(&ip[0],
                       IPADDR_LENGTH,
                       "%d.%d.%d.%d",
                       rand() % 256, rand() % 256,
                       rand() % 256, rand() % 256);
}

// CS-LANG:
//  Implementacia tohoto je strasna prasacina, pobavte sa, nenadavajte :)
void mocks::netmon(short min, short max, const char *connection) {
  // input arguments check
  if (min >= max || connection == NULL || strlen(connection) == 0) {
    fprintf(stderr, "ERROR: bad function input arguments.\n");
    return;
  }
  srand((unsigned) time(NULL));

  std::map<std::tuple<std::string, std::string, std::string>, bool> stored;
  unsigned short sleep_duration = 0;
  
  // zeromq initialization
  zmq::context_t context(ZMQ_IO_THREADS_NUM);
  zmq::socket_t sock_dealer(context, ZMQ_DEALER);
  char identity[ZMQ_IDENTITY_LENGTH];
  if (snprintf(identity, ZMQ_IDENTITY_LENGTH, "%d%d",
               getpid(), rand() % 99) < 0) {
    fprintf(stderr, "ERROR: snprintf(ZMQ_IDENTITY, ...) failed.\n");
    return;
  }
  sock_dealer.setsockopt(ZMQ_IDENTITY, identity, strlen(identity));
  sock_dealer.connect(connection);
  
  std::string macaddr, ipaddr, str_tmp; 
  while (true) {
    int action = rand() % 2;

    if (action) { // ADD

      gen_macaddr(macaddr);
      gen_ipaddr(ipaddr);
      int ifnumber = rand() % (IFCOUNT);
      std::tuple<std::string, std::string, std::string>
        insertion(std::string(IFNAMES[ifnumber]), ipaddr, macaddr);

      auto ret = stored.insert(std::make_pair(insertion, true));
      if (ret.second == true) {
        str_tmp.clear();
        json_pack(EVENT_NETWORK_ADD_STR, IFNAMES[ifnumber], IPVERSION_IPV4_STR,
                  ipaddr.c_str(), 24, macaddr.c_str(), str_tmp);
        utils::zeromq::str_send(sock_dealer, str_tmp);
      } // else do nothing 
    } else { // REMOVE

      std::size_t size = stored.size();
      if (size == 0) {
        continue;
      }
      ssize_t chosen = rand() % size;
      auto it = stored.cbegin();
      for (int i = 0; i < chosen; i++) {
        it++;
      }
      json_pack(EVENT_NETWORK_DEL_STR,
                std::get<0>(it->first).c_str(),
                IPVERSION_IPV4_STR,
                std::get<1>(it->first).c_str(),
                24, std::get<2>(it->first).c_str(),
                str_tmp);
      utils::zeromq::str_send(sock_dealer, str_tmp);        
      stored.erase(it);
    }
    sleep_duration = rand() % (max+1);
    if (sleep_duration < min) {
      sleep_duration = min;
    }
    sleep(sleep_duration);
  }
}

