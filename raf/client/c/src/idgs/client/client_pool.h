/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "client_const.h"
#include "tbb/concurrent_queue.h"
#include "tcp_synchronous_client.h"
#include "tcp_asynch_client.h"
#include "idgs/util/singleton.h"

namespace idgs {
namespace client {
typedef std::shared_ptr<TcpClient> TcpClientPtr;
typedef std::shared_ptr<TcpSynchronousClient> TcpSynchClientPtr;
typedef std::shared_ptr<TcpAsynchClient> TcpAsynchClientPtr;

class TcpClientInterface;

class TcpClientPool {
  friend class TcpSynchronousClient;
  friend class TcpAsynchClient;
  friend class TcpClientInterface;
public:

  TcpClientPool();
  ~TcpClientPool();

  idgs::ResultCode loadConfig(const ClientSetting& setting);

  std::shared_ptr<TcpClientInterface> getTcpClient(idgs::ResultCode &code);

  void close();

  std::string toString();

  void getAvailableServer(idgs::client::pb::Endpoint& server, std::vector<TcpClientPtr>& clients) const;

  const std::shared_ptr<idgs::client::pb::ClientConfig>& getClientConfig() const {
    return clientConfig;
  }

  size_t size() const {
    return pool.size();
  }

private:
  enum State {
    NOT_INITIALIZED, INITIALIZED
  };

  bool putBack(TcpClientPtr client);

  std::shared_ptr<idgs::client::pb::ClientConfig> clientConfig;
  tbb::concurrent_bounded_queue<TcpClientPtr> pool;
  std::vector<idgs::client::pb::Endpoint> availableServers;
  tbb::atomic<State> isLoaded;
};

class TcpClientInterface {
public:
  friend class TcpClientPool;

  TcpClientInterface(const TcpClientInterface& rhs) {
    this->client = rhs.client;
  }
  TcpClientInterface(TcpClientInterface&& rhs) {
    this->client = std::move(rhs.client);
  }
  ;

  const idgs::client::pb::Endpoint& getServerAddress() {
    return client->getServerAddress();
  }

  void send(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec = 10) {
    if (client.get() == NULL) {
      *code = RC_ERROR;
      return;
    }

    return client->send(actorMsg, code, timeout_sec);
  }

  ClientActorMessagePtr sendRecv(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec = 10) {
    if (client.get() == NULL) {
      *code = RC_ERROR;
      return ClientActorMessagePtr(NULL);
    }

    return client->sendRecv(actorMsg, code, timeout_sec);
  }

  ClientActorMessagePtr receive(ResultCode& code) {
    if (client.get() == NULL) {
      code = RC_ERROR;
      return ClientActorMessagePtr(NULL);
    }

    return client->receive(&code);
  }

  bool close() {
    if (client.get()) {
      DVLOG(2) << "client " << client.get() << " is recycled by pool";
      TcpClientPtr _c = client;
      client.reset();
      return ::idgs::util::singleton<TcpClientPool>::getInstance().putBack(_c);
    }
    return true;
  }

  ~TcpClientInterface() {
    function_footprint();
    close();
  }

private:

  TcpClientInterface(TcpClientPtr _client) :
      client(_client) {

  }

  idgs::ResultCode initialize() {
    if (client.get() == NULL) {
      return RC_ERROR;
    }

    return client->initialize();
  }

  TcpClientPtr client;
};
}
}
