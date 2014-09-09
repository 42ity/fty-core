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
#ifndef MOCK_DB_H_
#define MOCK_DB_H_

/*
 DISCLAIMER:
 No time to do this nice; please don't judge me :(

 Currently, db mock is implemented in the _simplest_ way that is possible:
 We synchronize each read/write access with a mutex. It's thread-safe, but
 not multithreaded.
*/

/*
 IMPLEMENTATION NOTES:
 - There should probably be a wrapper class for individual types
 that we want to persist... etc..

 - Although currently the primary key is just (ipaddres,ifname)(*)
 We should use the the whole quadruple and expose whole set of operations
 on data sets in the interface.

 (*) Reason beaing that in the netmon, just ADD and DEL operations are 
 supported at the moment.

 - Currently 'add' operations do not check for duplicity

 - As it may be hard and/or dangerous to implement hash... we 
 fell back to std::list. The downside is O(n) added operations when
 deleting a member of a list due to the neccessity of finding it first,
 the upside is that we don't need to bother with  template<>struct hash<>
 for each supported data type.
 TODO See how boost deals with unordered_map and custom type hashes,
  specifically hash_combine
*/

#include <string>
#include <list>
#include <mutex>
#include <fstream>
#include <iostream>

// TODO think about assign, copy const, explicit conv...
struct network_dt
{
  network_dt() : prefixlen(0) {}

  network_dt(const std::string& interface_name, const std::string& ip_version,
             const std::string& ip_address, uint8_t prefixlength,
             const std::string& mac_address)
  : interface(interface_name), ipversion(ip_version), ipaddress(ip_address),
    prefixlen(prefixlength), macaddress(mac_address), type("automatic") {}

  network_dt(const char *interface_name, const char *ip_version,
             const char *ip_address, uint8_t prefixlength,
             const char *mac_address)
  : interface(interface_name), ipversion(ip_version), ipaddress(ip_address),
    prefixlen(prefixlength), macaddress(mac_address), type("automatic") {}  

  network_dt(const std::string& interface_name, const std::string& ip_version,
             const std::string& ip_address, uint8_t prefixlength,
             const std::string& mac_address, const std::string& origin)
  : interface(interface_name), ipversion(ip_version), ipaddress(ip_address),
    prefixlen(prefixlength), macaddress(mac_address), type(origin) {}
  
  std::string interface;
  std::string ipversion;
  std::string ipaddress;
  uint8_t prefixlen;
  std::string macaddress;
  std::string type;
};

class mock_db
{
 public:
  mock_db() {}

  bool network_insert(const char *interface, const char *ipversion,
                      const char *ipaddress, uint8_t prefixlen,
                      const char *macaddress);
  bool network_insert(const std::string& interface, const std::string& ipversion,
                      const std::string& ipaddress, uint8_t prefixlen,
                      const std::string& macaddress) {
    return
    network_insert(interface.c_str(), ipversion.c_str(), ipaddress.c_str(),
                   prefixlen, macaddress.c_str());
  }

  bool network_add(const char *ipversion, const char *ipaddress, uint8_t prefixlen);
  bool network_add(const std::string& ipversion, const std::string& ipaddress,
                   uint8_t prefixlen) {
    return
    network_add(ipversion.c_str(), ipaddress.c_str(), prefixlen);
  }

  bool network_remove(const char* ipversion, const char* ipaddress, uint8_t prefixlen);
  bool network_remove(const std::string& ipversion, const std::string& ipaddress,
                      uint8_t prefixlen) {
    return
    network_remove(ipversion.c_str(), ipaddress.c_str(), prefixlen);
  }
  bool network_remove(const char *interface, const char *ipversion,
                      const char *ipaddress, uint8_t prefixlen,
                      const char *macaddress);
  bool network_remove(const std::string& interface, const std::string& ipversion,
                      const std::string& ipaddress, uint8_t prefixlen,
                      const std::string& macaddress) {
    return
    network_remove(interface.c_str(), ipversion.c_str(), ipaddress.c_str(),
                   prefixlen, macaddress.c_str());
  }
  /*!
  \brief

  \param[in, out] networks TODO
  
  \note Although we try to follow a no in-out parameter policy, an alternative
        to this would be std::list<...>& list_networks(), call new operator
        inside and propagate memory management onto the caller. 
  */
  void network_list(std::list<network_dt>& networks) const; 

 private:
  std::list<network_dt> networks_;
  std::list<network_dt> networks_mask_;
  mutable std::mutex networks_mutex_;
  
  
  //mock_db(const mock_db&);        // disallow copy-constructor
  void operator=(const mock_db&); // disallow assign operator

};

/* NOTE: Keeping this commented code in case we'll go for std::unordered_set  

//std::unordered_set<network_dt, network_dt_hash, network_dt_equal> networks_;

// Alternative way:
//template<> struct hash<network_dt>
//{
//  size_t operator()(const network_dt& n) const {}
//}
struct network_dt_hash
{
  std::size_t operator()(const network_dt& n) const
  {
    return std::hash<std::string>()(n.interface) ^
           std::hash<std::string>()(n.ipversion) ^
           std::hash<std::string>()(n.ipaddress) ^
           std::hash<uint8_t>()(n.prefixlen)  ^
           std::hash<std::string>()(n.macaddress); 
  }
};

struct network_dt_equal
{
  bool operator()(const network_dt& a, const network_dt& b) const
  {
    return a.interface.compare(b.interface) == 0 &&
           a.ipversion.compare(b.ipversion) == 0 &&
           a.ipaddress.compare(b.ipaddress) == 0 &&
           a.prefixlen == b.prefixlen &&
           a.macaddress.compare(b.macaddress) == 0;
  }
};
*/

#endif // MOCK_DB_H_
