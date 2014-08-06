#ifndef KVDB_H_
#define KVDB_H_
/*
 DISCLAIMER: No time to do this nice; please don't judge me :(

 There should probably be a wrapper class for individual types
 that we want to persist... etc..

 Although atm the primary key is just (ipaddre,ifname), we use
 the the whole quadruple ... 

 As it may be hard and/or dangerous to implement hash... we 
 fell back to std::list. TODO See how boost deals with 
 unordered_map and custom type hashes, specifically hash_combine

 TODO: thread-safety
*/

#include <string>
#include <list>
#include <unordered_set>
#include <utility>

// TODO think about assign, copy const, explicit conv...
struct network_dt
{
  network_dt() : prefixlen(0) {}

  network_dt(const std::string& interface_name, const std::string& ip_version,
             const std::string& ip_address, uint8_t prefixlength,
             const std::string& mac_address)
  : interface(interface_name), ipversion(ip_version), ipaddress(ip_address),
    prefixlen(prefixlength), macaddress(mac_address) {}

  network_dt(const char *interface_name, const char *ip_version,
             const char *ip_address, uint8_t prefixlength,
             const char *mac_address)
  : interface(interface_name), ipversion(ip_version), ipaddress(ip_address),
    prefixlen(prefixlength), macaddress(mac_address) {}  

  std::string interface;
  std::string ipversion;
  std::string ipaddress;
  uint8_t prefixlen;
  std::string macaddress;
};

/* NOTE: Keeping this in case we'll go for std::unordered_set  
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
class mock_db
{
 public:
  mock_db() {}

  //! TODO
  bool add_network(const char* interface, const char* ipversion,
                   const char* ipaddress, uint8_t prefixlen,
                   const char* macaddress);
  //! TODO
  bool add_network(const std::string& interface, const std::string& ipversion,
                   const std::string& ipaddress, uint8_t prefixlen,
                   const std::string& macaddress);
  //! TODO
  bool remove_network(const char* interface, const char* ipversion,
                      const char* ipaddress, uint8_t prefixlen,
                      const char* macaddress);
  //! TODO
  bool remove_network(const std::string& interface, const std::string& ipversion,
                      const std::string& ipaddress, uint8_t prefixlen,
                      const std::string& macaddress);
  /*!
  \brief

  \param[in, out] networks TODO
  
  \note Although we try to follow a no in-out parameter policy, an alternative
        to this would be std::list<...>& list_networks(), call new operator
        inside and propagate memory management onto the caller. 
  */
  void list_network(std::list<network_dt>& networks); 

 private:
  //std::unordered_set<network_dt, network_dt_hash, network_dt_equal> networks_;
  std::list<network_dt> networks_;
  
  mock_db(const mock_db&);        // disallow copy-constructor
  void operator=(const mock_db&); // disallow assign operator

};

#endif // KVDB_H_
