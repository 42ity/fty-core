#include "cidr.h"
#include <cstdio>
#include <stdlib.h>

namespace utils {

CIDRAddress::CIDRAddress() {
  cidr = NULL;
}

CIDRAddress::CIDRAddress(const std::string address) {
  cidr = NULL;
  set(address);
}

CIDRAddress::CIDRAddress(const CIDRAddress& address) {
  cidr = NULL;
  set(address);
}

CIDRAddress& CIDRAddress::operator++() {
  struct in_addr inaddr;
  unsigned char *bytes; 
  int i;
  
  if( ! valid() ) return *this;
  // do nothing for networks?
  if( cidr_get_proto(cidr) == CIDR_IPV4 ) {
    cidr_to_inaddr(cidr, &inaddr);
    bytes = (unsigned char *)&inaddr.s_addr;
    for(i=3; i>=0; i--) {
      bytes[i]++;
      if( bytes[i] != 0 ) { break; }
    }
    setCidrPtr(cidr_from_inaddr(&inaddr));
  }
  return *this;
}

CIDRAddress& CIDRAddress::operator--() {
  struct in_addr inaddr;
  unsigned char *bytes; 
  int i;
  
  if( ! valid() ) return *this;
  // do nothing for networks?
  if( cidr_get_proto(cidr) == CIDR_IPV4 ) {
    cidr_to_inaddr(cidr, &inaddr);
    bytes = (unsigned char *)&inaddr.s_addr;
    for(i=3; i>=0; i--) {
      bytes[i]--;
      if( bytes[i] != 255 ) { break; }
    }
    setCidrPtr(cidr_from_inaddr(&inaddr));
  }
  return *this;
}

CIDRAddress CIDRAddress::operator++(int) {
  CIDRAddress result;

  result.setCidrPtr(cidr ? cidr_dup(cidr) : NULL);
  ++(*this);
  return result;
}

CIDRAddress CIDRAddress::operator--(int) {
  CIDRAddress result;

  result.setCidrPtr(cidr ? cidr_dup(cidr) : NULL);
  --(*this);
  return result;
}

void CIDRAddress::setCidrPtr(CIDR *newcidr) {
  if(cidr) cidr_free(cidr);
  cidr = newcidr;
}

int CIDRAddress::prefix() {
  return cidr_get_pflen(cidr);
}

void CIDRAddress::invalidate() {
  setCidrPtr(NULL);
}

bool CIDRAddress::valid() const {
  in_addr in_addr4;
  in6_addr in_addr6;
  
  if( cidr == NULL ) return false;
  if( cidr_get_proto(cidr) == CIDR_IPV4 ) {
    if( cidr_to_inaddr(cidr,&in_addr4) ) {
      return (in_addr4.s_addr != 0); // 0.0.0.0 address
    } else {
      return false;
    }
  }
  if( cidr_get_proto(cidr) == CIDR_IPV6 ) {
    if( cidr_to_in6addr(cidr,&in_addr6) ) {
      for( int i=0; i<16; i++) {
        if( in_addr6.s6_addr[i] != 0 ) { return true; }
      }
      return false; // :: address
    } else {
      return false;
    }
  }
  return false;
}

bool CIDRAddress::contains(const CIDRAddress& address) {
  return (cidr_contains(cidr,address.cidr) == 0);
}

bool CIDRAddress::in(const CIDRAddress& address) {
  return (cidr_contains(address.cidr,cidr) == 0);  
}

bool CIDRAddress::equals(const CIDRAddress& address) {
  return (cidr_equals(address.cidr,cidr) == 0);  
}

CIDRAddress CIDRAddress::hostMin(){
  CIDRAddress result;
  CIDR *host = cidr_addr_hostmin(cidr);
  result.setCidrPtr(host);
  return result;
}

CIDRAddress CIDRAddress::hostMax(){
  CIDRAddress result;
  CIDR *host = cidr_addr_hostmax(cidr);
  result.setCidrPtr(host);
  return result;
}

CIDRAddress CIDRAddress::host() {
  CIDRAddress result;
  in_addr in_addr4;
  in6_addr in_addr6;
  
  if( ! valid() ) return result;
  if( cidr_get_proto(cidr) == CIDR_IPV4 ) {
    cidr_to_inaddr(cidr,&in_addr4);
    result.setCidrPtr( cidr_from_inaddr(&in_addr4) );
  }
  if( cidr_get_proto(cidr) == CIDR_IPV6 ) {
    cidr_to_in6addr(cidr,&in_addr6);
    result.setCidrPtr( cidr_from_in6addr(&in_addr6) );
  }
  return result;
}

CIDRAddress CIDRAddress::broadcast(){
  CIDRAddress result;
  CIDR *host = cidr_addr_broadcast(cidr);
  result.setCidrPtr(host);
  return result;
}

bool CIDRAddress::set(const std::string text) {
  CIDR *newcidr = cidr_from_str(text.c_str());
  setCidrPtr(newcidr);
  return (cidr != NULL);
}

bool CIDRAddress::set(const CIDRAddress& from) {
  setCidrPtr(from.cidr ? cidr_dup(from.cidr) : NULL);
  return ( cidr != NULL );
}

std::string CIDRAddress::toString() {
  std::string addr = "";
  
  if(cidr) {
    if( cidr_get_proto(cidr) == CIDR_IPV4 ) {
      if( prefix() == 32 ) {
        return addressToString();
      }
      return networkToString();
    }
    if( cidr_get_proto(cidr) == CIDR_IPV6 ) {
      if( prefix() == 128 ) {
        return addressToString();
      }
      return networkToString();
    }
  }
  return addr;
}

std::string CIDRAddress::addressToString() {
  std::string addr = "";
  char *cstr;
  
  if(cidr) {
    cstr = cidr_to_str(cidr,CIDR_ONLYADDR);
    if(cstr) {
      addr = cstr;
      free(cstr);
    }
  }
  return addr;
}

std::string CIDRAddress::networkToString() {
  std::string addr = "";
  char *cstr;
  
  if(cidr) {
    cstr = cidr_to_str(cidr,CIDR_NOFLAGS);
    if(cstr) {
      addr = cstr;
      free(cstr);
    }
  }
  return addr;
}

int CIDRAddress::compare(const CIDRAddress &a2) {
  struct in_addr inaddr1, inaddr2;
  unsigned char *bytes1, *bytes2;
  int i;
  
  if( (cidr_get_proto(cidr) == CIDR_IPV4) && (cidr_get_proto(a2.cidr) == CIDR_IPV4) ) {
    cidr_to_inaddr(cidr, &inaddr1);
    cidr_to_inaddr(a2.cidr, &inaddr2);
    bytes1 = (unsigned char *)&inaddr1.s_addr;
    bytes2 = (unsigned char *)&inaddr2.s_addr;
    for(i=0; i<=3; i++) {
      if( bytes1[i] < bytes2[i] ) return -1; //im smaller
      if( bytes1[i] > bytes2[i] ) return +1; //im bigger
    }
    return 0; //we are equal
  }
  /* should not get here */
  return -2;
}

CIDRAddress& CIDRAddress::operator=(const std::string address) {
  set(address);
  return *this;
}

CIDRAddress& CIDRAddress::operator=(const CIDRAddress& address){
  if( this != &address ){
    setCidrPtr( address.cidr ? cidr_dup(address.cidr) : NULL );
  }
  return *this;
}

CIDRAddress::~CIDRAddress() {
  setCidrPtr( NULL );
}

bool CIDRAddress::operator>(const CIDRAddress &a2) {
  return (compare(a2) == 1);
}

bool CIDRAddress::operator<(const CIDRAddress &a2) {
  return (compare(a2) == -1);
}

bool CIDRAddress::operator==(const CIDRAddress &a2) {
  return (compare(a2) == 0);
}

bool CIDRAddress::operator==(const std::string &a2) {
  CIDRAddress ca2(a2);
  return (compare(ca2) == 0);
}

std::ostream& operator<<(std::ostream& os, CIDRAddress& address)
{
  return os << address.toString();
}

CIDRList::CIDRList(){

}

bool CIDRList::nextSimple(CIDRAddress& address) {
  if( ! address.valid() ) {
    address.set(firstAddress());
    last = lastAddress();
    return address.valid();
  }
  ++address;
  if( address > last ) {
    address.invalidate();
  }
  return address.valid();
}

bool CIDRList::next(CIDRAddress& address) {
  int filtered,iprefix,eprefix;
  do {
    filtered = 0;
    nextSimple(address);
    if( ! address.valid() ) { break; }
    iprefix = bestNetworkPrefixFor(address);
    if( iprefix == -1) {
      // out of scope
      skipToNextPool(address);
      filtered = 1;
    } else {
      eprefix = bestExcludePrefixFor(address);
      if(eprefix > iprefix) {
        //excluded
        skipToExcludeEnd(address);
        filtered = 1;
      } else {
        //not excluded, skip net and broadcast
        CIDRAddress net = bestNetworkFor(address);
        // FIXME IPv6
        if( net.prefix() < 31 /* and IPv4? */) {
          if( ( address == net.host() ) || ( address == net.broadcast() ) ){
            filtered = 1;
          }
        }
      }
    }
  } while ( filtered );
  return address.valid();
}

bool CIDRList::add(const std::string net) {
  CIDRAddress cnet(net);
  return add(cnet);
}

bool CIDRList::add(const CIDRAddress& net) {
  if( ! net.valid() ) { return false; }
  for(unsigned int i=0; i<networks.size(); i++ ) {
    if( networks[i]->equals(net) ) {
      return false;
    }
  }
  CIDRAddress *copy = new CIDRAddress(net);
  networks.push_back(copy);
  return true;
}

bool CIDRList::exclude(const std::string net) {
  CIDRAddress cnet(net);
  return exclude(cnet);
}

bool CIDRList::exclude(const CIDRAddress& net) {
  if( ! net.valid() ) return false;
  for(unsigned int i=0; i<excludedNetworks.size(); i++ ) {
    if( excludedNetworks[i]->equals(net) ) return false;
  }
  CIDRAddress *copy = new CIDRAddress(net);
  excludedNetworks.push_back(copy);
  return true;
}

CIDRAddress CIDRList::firstAddress() {
  CIDRAddress addr, result;
  if( ! networks.size() ) { return result; }
  result = networks[0]->host();
  for(unsigned int i=1; i<networks.size(); i++ ){
    addr = networks[i]->host();
    if( addr < result ) result = addr;
  }
  return result;
}

CIDRAddress CIDRList::lastAddress() {
  CIDRAddress addr, result;
  if( ! networks.size() ) { return result; }
  result = networks[0]->hostMax();
  for(unsigned int i=1; i<networks.size(); i++ ){
    addr = networks[i]->hostMax();
    if( addr >  result ) result = addr;
  }
  return result;
}

CIDRAddress CIDRList::bestNetworkFor(CIDRAddress& address) {
  int prefix, bestprefix = -1;
  CIDRAddress net;

  for(unsigned int i=0; i<networks.size(); i++ ) {
    if( networks[i]->contains(address) ) {
      prefix = networks[i]->prefix();
      if(prefix > bestprefix) {
        bestprefix = prefix;
        net = *networks[i];
      }
    }
  }
  return net;
}

void CIDRList::skipToNextPool(CIDRAddress& address) {
  CIDRAddress selected;
  for(unsigned int i=0; i<networks.size(); i++ ) {
    if( *networks[i] > address ) {
      if( selected.valid() ) {
        if( *networks[i] < selected ) {
          selected = networks[i]->host();
        }
      } else {
        selected = networks[i]->host();
      }
    }
  }
  --selected;
  address.set(selected);
}

void CIDRList::skipToExcludeEnd(CIDRAddress& address) {
  CIDRAddress currentExclude = bestExcludeFor(address);
  CIDRAddress excludeEnd = currentExclude.broadcast();
  CIDRAddress selected = excludeEnd;
  
  for(unsigned int i=0; i<networks.size(); i++ ) {
    if( (*networks[i] > address) && (*networks[i] < excludeEnd) ) {
      if( selected.valid() ) {
        if( *networks[i] < selected ) {
          selected = networks[i]->host();
          --selected;
        }
      } else {
        selected = networks[i]->host();
        --selected;
      }
    }
  }
  address.set(selected);
}

int CIDRList::bestNetworkPrefixFor(CIDRAddress& address) {
  CIDRAddress bestnet = bestNetworkFor(address);
  if(bestnet.valid()) return bestnet.prefix();
  return -1;
}

CIDRAddress CIDRList::bestExcludeFor(CIDRAddress& address) {
  int prefix, bestprefix = -1;
  CIDRAddress net;
  for(unsigned int i=0; i<excludedNetworks.size(); i++ ) {
    if( excludedNetworks[i]->contains(address) ) {
      prefix = excludedNetworks[i]->prefix();
      if(prefix > bestprefix) {
        bestprefix = prefix;
        net = *excludedNetworks[i];
      }
    }
  }
  return net;
}

int CIDRList::bestExcludePrefixFor(CIDRAddress& address) {
  CIDRAddress bestnet = bestExcludeFor(address);
  if(bestnet.valid()) return bestnet.prefix();
  return -1;
}

bool CIDRList::includes(const CIDRAddress& address) {
  for(unsigned int i=0; i<networks.size(); i++ ){
    if( networks[i]->contains(address)  ) return true;
  }
  return false;
}

bool CIDRList::excludes(const CIDRAddress& address) {
  for(unsigned int i=0; i<excludedNetworks.size(); i++ ){
    if( excludedNetworks[i]->contains(address) ) {
      return true;
    }
  }
  return false;
}

CIDRList::~CIDRList(){
  unsigned int i;
  for(i=0; i < networks.size(); i++ ) {
    if(networks[i] != NULL)  { delete networks[i]; }
  }
  for(i=0; i < excludedNetworks.size(); i++ ) {
    if(excludedNetworks[i] != NULL) { delete excludedNetworks[i]; }
  }
}

} // namespace utils
