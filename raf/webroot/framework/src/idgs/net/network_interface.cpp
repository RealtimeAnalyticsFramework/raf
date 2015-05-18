/*
 * network_interface.cpp
 *
 *  Created on: Jan 21, 2014
 *      Author: root
 */

#include "network_interface.h"

#include <ifaddrs.h>
#include <arpa/inet.h>

namespace idgs {
namespace net {

idgs::ResultCode network::getInterfaceAddress(std::vector<std::string>& addrs) {
  struct ifaddrs *ifaddrs, *ifa;
  int family;
  char addr[1025];
  if(getifaddrs(&ifaddrs) == -1) {
    LOG(ERROR) << "get interface addresses error!";
    return RC_ERROR;
  }
  for(ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    if(ifa->ifa_addr == NULL) {
      continue;
    }
    family = ifa->ifa_addr->sa_family;
    if(family == AF_INET) { /// ipv4
      if(inet_ntop(ifa->ifa_addr->sa_family, (void *)&(((struct sockaddr_in *) ifa->ifa_addr)->sin_addr), addr, sizeof(addr)) == NULL) {
        LOG(ERROR) << "inet_ntop failed!";
        continue;
      }
      if(std::string(addr).compare("127.0.0.1") == 0) {
        continue;
      }
      DVLOG(3) << "ipv4 address: " << addr;
      addrs.push_back(addr);
    } else if(family == AF_INET6) { /// ipv6
      if(inet_ntop(ifa->ifa_addr->sa_family, (void *)&(((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr), addr, sizeof(addr)) == NULL) {
        LOG(ERROR) << "inet_ntop failed!";
        continue;
      }
      std::string prefix(":");
      if(std::string(addr).substr(0, prefix.length()) == prefix) {
        continue;
      }
      DVLOG(3) << "ipv6 address: " << addr;
      addrs.push_back(addr);
    }
  } // end for loop
  freeifaddrs(ifaddrs); // release
  return RC_OK;
} // end function getInterfaceAddress
} // end namespace util
} // end namespace idgs





