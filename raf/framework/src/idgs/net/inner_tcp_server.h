
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <tbb/spin_rw_mutex.h>
#include "inner_tcp_connection.h"
#include "idgs/pb/cluster_config.pb.h"

namespace idgs {
namespace pb {
class ClusterConfig;

} // namespace pb
} // namespace idgs

namespace idgs {
namespace net {
class   NetworkModelAsio;

/// Inner TCP server: to send and receive message between members.
class InnerTcpServer {
public:
  InnerTcpServer(NetworkModelAsio* net, asio::io_service& io_service);
  virtual ~InnerTcpServer();

public:
  void init(idgs::pb::ClusterConfig* cfg);
  int start();
  int stop();

  NetworkModelAsio* getNetwork() const {
    return network;
  }

public:
  bool setConnection(uint32_t memberId, std::shared_ptr<InnerTcpConnection> conn);
  bool resetConnectionIfEq(uint32_t memberId, std::shared_ptr<InnerTcpConnection> conn);
  std::shared_ptr<InnerTcpConnection>& getConnection(uint32_t memberId);

  std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > getQueue(int memberId);
  int32_t sendMessage(idgs::actor::ActorMessagePtr& msg);
  std::string toString();

  uint32_t getTcpBatchSize() const {
    return cfg == NULL? 0 : cfg->batch_message();
  }

  void connect(int peerId);

private:
  void accept();
  void handle_accept(std::shared_ptr<InnerTcpConnection> conn, const asio::error_code& error);

private:
  friend class InnerTcpConnection;
  NetworkModelAsio* network;
  asio::io_service& io_service;
  asio::ip::tcp::acceptor* acceptor;
  idgs::pb::ClusterConfig* cfg;

  /// index is destination member id.
  std::vector<std::shared_ptr<InnerTcpConnection> > connections;
  std::vector<std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > > queues;
  tbb::spin_rw_mutex mutex;
};

} // namespace net 
} // namespace idgs 
