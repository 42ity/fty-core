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
 
Description: persistence layer (db) mock implementation
References: BIOS-248, BIOS-326
*/
#include "mock_db.h"


bool mock_db::network_insert(const char* interface, const char* ipversion,
                             const char* ipaddress, uint8_t prefixlen,
                             const char* macaddress) {
  if (interface == nullptr ||
      ipversion == nullptr ||
      ipaddress == nullptr ||
      macaddress == nullptr) {
    //log    
    return false;
  }
  std::lock_guard<std::mutex> lock_guard(networks_mutex_);
 
 for (auto it = networks_.cbegin(); it != networks_.cend(); it++) {    
    if (it->interface.compare(interface) == 0 &&
        it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen &&
        it->macaddress.compare(macaddress) == 0 &&
        it->type.compare("automatic") == 0) {
      return false;
    }    
  }  
  networks_.push_back(network_dt(interface, ipversion, ipaddress, prefixlen,
                                 macaddress, "automatic"));
  return true;
}


bool
mock_db::network_add(
const char *ipversion, const char *ipaddress, uint8_t prefixlen) {
  if (ipversion == nullptr ||
      ipaddress == nullptr) {
    // log
    return false;
  }
  // std::list<network_dt>::const_iterator it; // gcc v.4.9
  std::list<network_dt>::iterator it;
  // if aplicable mask exist, remove it
  // for (it = networks_mask_.cbegin(); it != networks_mask_.cend(); it++) { // gcc v.4.9
  for (it = networks_mask_.begin(); it != networks_mask_.end(); it++) {
    if (it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen) {
      networks_mask_.erase(it);
      break;
    }
  }
  
  // or add a manual network
  // for (it = networks_.cbegin(); it != networks_.cend(); it++) { // gcc v.4.9
  for (it = networks_.begin(); it != networks_.end(); it++) {
    if (it->interface.empty() &&
        it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen &&
        it->macaddress.empty() &&
        it->type.compare("manual") == 0) {
     return false;
    }    
  }   
  networks_.push_back(network_dt("", ipversion, ipaddress, prefixlen,
                                 "", "manual"));
  return true;
}

// networks_mask
bool
mock_db::network_remove(const char* ipversion, const char* ipaddress,
                        uint8_t prefixlen) {
  std::lock_guard<std::mutex> lock_guard(networks_mutex_);

  for (auto it = networks_.begin(); it != networks_.end(); it++) {
      if (it->type == "manual" &&
          it->ipaddress == ipaddress &&
          it->prefixlen == prefixlen) {

          networks_.erase(it);
          break;
      }
  }

  for (auto it = networks_mask_.cbegin(); it != networks_mask_.cend(); it++) {
    if (it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen) {
      return false;
    }        
  }
  networks_mask_.push_back(
    network_dt("", ipversion, ipaddress, prefixlen, "", ""));
  return true;
}

bool
mock_db::network_remove(const char *interface, const char *ipversion,
               const char *ipaddress, uint8_t prefixlen,
               const char *macaddress) {  
  if (interface == nullptr ||
      ipversion == nullptr ||
      ipaddress == nullptr ||
      macaddress == nullptr) {
    //log    
    return false;
  }
  std::lock_guard<std::mutex> lock_guard(networks_mutex_);
  // std::list<network_dt>::const_iterator it; // gcc v.4.9
  std::list<network_dt>::iterator it;
  // for (it = networks_.cbegin(); it != networks_.cend(); it++) { // gcc v.4.9
  for (it = networks_.begin(); it != networks_.end(); it++) {    
    if (it->interface.compare(interface) == 0 &&
        it->ipversion.compare(ipversion) == 0 &&
        it->ipaddress.compare(ipaddress) == 0 &&
        it->prefixlen == prefixlen &&
        it->macaddress.compare(macaddress) == 0 &&
        it->type.compare("automatic") == 0) {
      break;
    }    
  }
  // if (it != networks_.cend()) { // gcc v.4.9
  if (it != networks_.end()) {  
    networks_.erase(it);
    return true;
  }
  return false;
}

void mock_db::network_list(std::list<network_dt>& result) const {
  std::lock_guard<std::mutex> lock_guard(networks_mutex_);
  result.clear();
  for (auto it : networks_) {
    result.push_back(it);
  }

  for (auto it : networks_mask_) {
      it.type = "deleted";
    result.push_back(it);
  }
  return; 
}

