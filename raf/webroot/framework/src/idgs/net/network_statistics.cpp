
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/net/network_statistics.h"
#include <sstream>
#include <iomanip>

namespace idgs {
namespace net {

NetworkStatistics::NetworkStatistics() {
  outerTcpConnections.store(-1);
  reset();
}

NetworkStatistics::~NetworkStatistics() {
}

void NetworkStatistics::reset() {
  udpPacketSent.store(0);
  udpBytesSent.store(0);
  udpPacketRecv.store(0);
  udpBytesRecv.store(0);
  udpPacketResent.store(0);
  udpBytesResent.store(0);

  innerTcpPacketSent.store(0);
  innerTcpBytesSent.store(0);
  innerTcpPacketRecv.store(0);
  innerTcpBytesRecv.store(0);

  outerTcpPacketSent.store(0);
  outerTcpBytesSent.store(0);
  outerTcpPacketRecv.store(0);
  outerTcpBytesRecv.store(0);

  multicastPacketSent.store(0);
  multicastBytesSent.store(0);
  multicastPacketRecv.store(0);
  multicastBytesRecv.store(0);
}

#define DUMP_NET(AD)   if (AD.load())  ss << std::setw(21) << #AD << ": " << AD.load() << std::endl;

#define DUMP_JSON_NET(AD) if (AD.load())  ss << "\"" << #AD << "\": \"" << AD.load() <<"\",";

std::string NetworkStatistics::toString() const {
  std::stringstream ss;

  DUMP_NET(udpPacketSent);
  DUMP_NET(udpPacketRecv);
  DUMP_NET(udpPacketResent);
  DUMP_NET(udpBytesSent);
  DUMP_NET(udpBytesRecv);
  DUMP_NET(udpBytesResent);

  DUMP_NET(innerTcpPacketSent);
  DUMP_NET(innerTcpPacketRecv);
  DUMP_NET(innerTcpBytesSent);
  DUMP_NET(innerTcpBytesRecv);

  DUMP_NET(outerTcpPacketSent);
  DUMP_NET(outerTcpPacketRecv);
  DUMP_NET(outerTcpBytesSent);
  DUMP_NET(outerTcpBytesRecv);
  DUMP_NET(outerTcpConnections);

  DUMP_NET(multicastPacketSent);
  DUMP_NET(multicastPacketRecv);
  DUMP_NET(multicastBytesSent);
  DUMP_NET(multicastBytesRecv);

  return ss.str();
}

std::string NetworkStatistics::toJsonString() const {
  std::stringstream ss;
  ss << "{";
  DUMP_JSON_NET(udpPacketSent);
  DUMP_JSON_NET(udpPacketRecv);
  DUMP_JSON_NET(udpPacketResent);
  DUMP_JSON_NET(udpBytesSent);
  DUMP_JSON_NET(udpBytesRecv);
  DUMP_JSON_NET(udpBytesResent);

  DUMP_JSON_NET(innerTcpPacketSent);
  DUMP_JSON_NET(innerTcpPacketRecv);
  DUMP_JSON_NET(innerTcpBytesSent);
  DUMP_JSON_NET(innerTcpBytesRecv);

  DUMP_JSON_NET(outerTcpPacketSent);
  DUMP_JSON_NET(outerTcpPacketRecv);
  DUMP_JSON_NET(outerTcpBytesSent);
  DUMP_JSON_NET(outerTcpBytesRecv);
  DUMP_JSON_NET(outerTcpConnections);

  DUMP_JSON_NET(multicastPacketSent);
  DUMP_JSON_NET(multicastPacketRecv);
  DUMP_JSON_NET(multicastBytesSent);
  DUMP_JSON_NET(multicastBytesRecv);

  std::string ret = ss.str();
  if (ret.size() > 1) {
    ret.pop_back();
  }

  ret.append("}");

  return ret;
}

} // namespace net 
} // namespace idgs 
