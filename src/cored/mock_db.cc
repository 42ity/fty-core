#include "mock_db.h"

// TODO KHR(5.8) Implement multithreading
// NOTE: Add operations - not checking duplicity

bool mock_db::add_network(const std::string& interface,
                          const std::string& ipversion,
                          const std::string& ipaddress,
                          uint8_t prefixlen,
                          const std::string& macaddress) {
  return add_network(interface.c_str(), ipversion.c_str(), ipaddress.c_str(),
                     prefixlen, macaddress.c_str());
}

bool mock_db::add_network(const char* interface, const char* ipversion,
                          const char* ipaddress, uint8_t prefixlen,
                          const char* macaddress) {
  networks_.emplace_back(interface, ipversion, ipaddress, prefixlen,
                         macaddress);
  return true;
}

bool mock_db::remove_network(const char* interface, const char* ipversion,
                             const char* ipaddress, uint8_t prefixlen,
                             const char* macaddress) {
  for (auto it = networks_.begin(); it != networks_.end(); it++) {
    if (it->interface.compare(interface) == 0 &&
        it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen &&
        it->macaddress.compare(macaddress) == 0) {
      networks_.erase(it);
      return true;
    }        
  } 
  return false;
}

bool mock_db::remove_network(const std::string& interface, const std::string& ipversion,
                             const std::string& ipaddress, uint8_t prefixlen,
                             const std::string& macaddress) {
  return remove_network(interface.c_str(), ipversion.c_str(),
                        ipaddress.c_str(), prefixlen,
                        macaddress.c_str());
}

void mock_db::list_network(std::list<network_dt>& networks) {
  networks.clear();
  networks.assign(networks_.cbegin(), networks_.cend());
  return; 
}

