
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <thread>
// #include "idgs/net/network_model.h"
#include "idgs/net/member_endpoint.h"
#include "idgs/httpserver/http_server.h"
#include "idgs/pb/cluster_config.pb.h"

namespace idgs {
namespace net {
class InnerTcpServer;
class AsyncTcpServer;
class AsyncUdpServer;

class RpcMemberListener;
class NetworkModelAsio;
class NetworkStatistics;
class ResendScheduler;

typedef NetworkModelAsio NetworkModel;

//typedef std::unique_ptr<RpcMessage> rpc_message_ptr;
class NetworkModelAsio {
public:
  NetworkModelAsio();
  ~NetworkModelAsio();

  int32_t init(idgs::pb::ClusterConfig* cfg);
  int32_t start();
  int32_t shutdown();

  int32_t send(idgs::actor::ActorMessagePtr msg);

  asio::io_service& getIoService() {
    return ioService;
  }

  NetworkStatistics* getNetworkStatistics() {
    return networkStatistics;
  }

  void putEndPoint(int32_t memberId, const ::idgs::pb::EndPoint& endPoint);

  MemberEndPoint* getEndPoint(int32_t memberId) {
    return &endPointCache[memberId];
  }

  ResendScheduler* getResendScheduler() {
    return resendScheduler;
  }

  /// set socket options, e.g. receive/send buffer size
  static void setUdpSocketOption(asio::ip::udp::socket& s, bool server = true);
  static void setTcpSocketOption(asio::ip::tcp::socket& s);

  InnerTcpServer* getInnerTcpServer() {
    return innerTcpServer;
  }

  AsyncTcpServer* getOuterTcpServer() {
    return outerTcpServer;
  }

  idgs::http::server::HttpServer* getHttpServer() {
    return httpServer;
  }

  idgs::pb::ClusterConfig* getClusterConfig() const {
    return cfg;
  }

private:
  idgs::pb::ClusterConfig* cfg;
  asio::io_service ioService;
  InnerTcpServer* innerTcpServer;
  AsyncTcpServer* outerTcpServer;
  AsyncUdpServer* innerUdpServer;

  RpcMemberListener* rpcMemberListener;
  NetworkStatistics* networkStatistics;
  ResendScheduler* resendScheduler;
  idgs::http::server::HttpServer* httpServer;

  std::map<int32_t, MemberEndPoint> endPointCache;
  std::vector<std::shared_ptr<std::thread>> ioThreads;
  static int SOCKET_BUFFER_SIZE;
};
} // namesapce net
} // namespace idgs
