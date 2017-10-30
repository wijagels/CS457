#include "netutils.hpp"

#include <cstring>
#include <string>
#include <stdexcept>

extern "C" {
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
}

std::string get_public_ip() {
  ifaddrs* ifAddrStruct = nullptr;
  ifaddrs* ifa = nullptr;
  in_addr* tmpAddrPtr = nullptr;

  getifaddrs(&ifAddrStruct);
  for (ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
    // We don't want interfaces with no address or loopback
    if (!ifa->ifa_addr || !std::strcmp(ifa->ifa_name, "lo")) continue;
    if (ifa->ifa_addr->sa_family == AF_INET) {
      tmpAddrPtr = &(reinterpret_cast<sockaddr_in*>(ifa->ifa_addr))->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      return {addressBuffer};
    }
  }
  throw std::runtime_error{"Unable to find an address to bind to"};
}

