
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/net/stateful_tcp_actor.h"
#include "idgs/pb/cluster_config.pb.h"


namespace idgs {
namespace net {
class   NetworkModelAsio;


class AsyncTcpServer {
public:
  AsyncTcpServer(NetworkModelAsio* net, asio::io_service& _io_service);
  ~AsyncTcpServer();

  void init(idgs::pb::ClusterConfig* cfg);

  int32_t start();
  void stop();

  NetworkModelAsio* getNetwork() const {
    return network;
  }

private:
  void run();
  void handle_accept(asio::ip::tcp::socket *sock, const asio::error_code& error);

private:
  friend class StatefulTcpActor;
  NetworkModelAsio* network;
  asio::io_service& io_service;
  asio::ip::tcp::acceptor* acceptor;
  idgs::pb::ClusterConfig* cfg;

private:
  AsyncTcpServer(const AsyncTcpServer&) = delete;
  AsyncTcpServer(AsyncTcpServer&&) = delete;
  AsyncTcpServer& operator =(const AsyncTcpServer&) = delete;
  AsyncTcpServer& operator =(const AsyncTcpServer&&) = delete;
};
} // namespace net
} // namespace idgs
