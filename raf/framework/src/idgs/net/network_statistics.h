
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <atomic>
#include <string>

namespace idgs {
namespace net {

class NetworkStatistics {
public:
  NetworkStatistics();
  virtual ~NetworkStatistics();

public:
  std::atomic_ulong udpPacketSent;
  std::atomic_ulong udpBytesSent;
  std::atomic_ulong udpPacketRecv;
  std::atomic_ulong udpBytesRecv;
  std::atomic_ulong udpPacketResent;
  std::atomic_ulong udpBytesResent;

  std::atomic_ulong innerTcpPacketSent;
  std::atomic_ulong innerTcpBytesSent;
  std::atomic_ulong innerTcpPacketRecv;
  std::atomic_ulong innerTcpBytesRecv;

  std::atomic_ulong outerTcpPacketSent;
  std::atomic_ulong outerTcpBytesSent;
  std::atomic_ulong outerTcpPacketRecv;
  std::atomic_ulong outerTcpBytesRecv;
  std::atomic_ulong outerTcpConnections;

  std::atomic_ulong multicastPacketSent;
  std::atomic_ulong multicastBytesSent;
  std::atomic_ulong multicastPacketRecv;
  std::atomic_ulong multicastBytesRecv;
  void reset();
  std::string toString() const;
  std::string toJsonString() const;
};

} // namespace net 
} // namespace idgs 
