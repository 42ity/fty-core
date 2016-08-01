/*
Copyright (C) 2015-2016 Eaton

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

/*!
 * \file iface.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Function for accessing network interfaces on Linux
 */
#include <stdexcept>
#include <stdio.h>
#include <stdbool.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <map>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include "iface.h"

std::set<std::string> get_ifaces() {
   std::set<std::string> ret;
   struct ifaddrs* start = NULL;
   if(getifaddrs(&start) != 0)
      return ret;
   struct ifaddrs* it = start;
   while(it != NULL) {
      ret.insert(it->ifa_name);
      it = it->ifa_next;
   }
   return ret;
}

iface get_iface(std::string iface) {
   struct iface ret;
   struct ifaddrs* start = NULL;
   if(getifaddrs(&start) != 0)
      return ret;
   struct ifaddrs* it = start;
   while(it != NULL) {
      if(it->ifa_name != iface) {
         it = it->ifa_next;
         continue;
      }
      ret.state = (it->ifa_flags & IFF_UP) ? "up" : "down";
      struct ifreq ifr;
      struct ethtool_value edata;

      memset(&ifr, 0, sizeof(ifr));
      strncpy(ifr.ifr_name, it->ifa_name, sizeof(ifr.ifr_name)-1);

      edata.cmd = ETHTOOL_GLINK;

      int fd = socket(PF_INET, SOCK_DGRAM, 0);
      if(ret.mac.empty() && ioctl(fd, SIOCGIFHWADDR, &ifr) != -1) {
         char buff[3];
         memset(buff, 0, sizeof buff);
         for(int i = 0; i < 6; ++i) {
            if(!ret.mac.empty()) {
               ret.mac += ":";
            }
            sprintf(buff, "%02x", (unsigned char) ifr.ifr_addr.sa_data[i]);
            ret.mac += buff;
         }
      }

      ifr.ifr_flags = it->ifa_flags | IFF_UP;
      if((it->ifa_flags & IFF_UP) || (ioctl(fd, SIOCSIFFLAGS, &ifr) != -1)) {
         // To detect link interface has to be up for some time
         if((it->ifa_flags & IFF_UP) == 0)
            sleep(5);
         ifr.ifr_data = (caddr_t) &edata;
         if(ioctl(fd, SIOCETHTOOL, &ifr) != -1) {
            ret.cable = edata.data ? "yes" : "no";
         }
         if((it->ifa_flags & IFF_UP) == 0) {
            ifr.ifr_data = NULL;
            ifr.ifr_flags = it->ifa_flags;
            ioctl(fd, SIOCSIFFLAGS, &ifr);
         }
      } else {
         ret.cable = "unknown";
      }
      close(fd);

      if(it->ifa_addr->sa_family == AF_INET) {
         ret.ip.push_back(inet_ntoa(((struct sockaddr_in*)(it->ifa_addr))->sin_addr));
         ret.netmask.push_back(inet_ntoa(((struct sockaddr_in*)(it->ifa_netmask))->sin_addr));
      }
      it = it->ifa_next;
   }
   freeifaddrs(start);
   return ret;
}
