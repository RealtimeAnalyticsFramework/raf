/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include <atomic>
#include <tbb/concurrent_queue.h>

#include "idgs/client/client_const.h"
#include "idgs/client/client_setting.h"
#include "idgs/client/tcp_client.h"

#include "idgs/store/data_store.h"

namespace idgs {
namespace client {
typedef std::shared_ptr<TcpClient> TcpClientPtr;

class PooledTcpClient;

class TcpClientPool {
  friend class PooledTcpClient;
public:

  TcpClientPool();
  ~TcpClientPool();

  idgs::ResultCode loadConfig(const ClientSetting& setting);

  std::shared_ptr<TcpClient> getTcpClient(idgs::ResultCode &code);

  void close();

  std::string toString();

  void getAvailableServer(idgs::client::pb::Endpoint& server, std::vector<TcpClientPtr>& clients) const;

  const std::shared_ptr<idgs::client::pb::ClientConfig>& getClientConfig() const {
    return clientConfig;
  }

  size_t size() const {
    return pool.size();
  }

  idgs::store::DataStore* getDataStore();
private:
  enum State {
    NOT_INITIALIZED, INITIALIZED
  };

  bool putBack(TcpClientPtr client);

private:
  std::shared_ptr<idgs::client::pb::ClientConfig> clientConfig;
  tbb::concurrent_bounded_queue<TcpClientPtr> pool;
  std::vector<idgs::client::pb::Endpoint> availableServers;
  std::atomic<State> isLoaded;

  idgs::store::DataStore datastore;
};

TcpClientPool& getTcpClientPool();

} // namespace client
} // namespace idgs
