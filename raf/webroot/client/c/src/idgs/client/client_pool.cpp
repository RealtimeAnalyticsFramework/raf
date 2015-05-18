/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/client/client_pool.h"
#include "idgs/client/asio_tcp_client.h"
#include "idgs/util/singleton.h"
#include "idgs/util/utillity.h"



namespace idgs {
namespace client {
class PooledTcpClient : public TcpClient {
public:
  friend class TcpClientPool;

  virtual idgs::ResultCode initialize() override {
    if (client.get() == NULL) {
      return RC_ERROR;
    }
    return client->initialize();
  }
  virtual const idgs::client::pb::Endpoint& getServerEndpoint() const override {
    return client->getServerEndpoint();
  }
  virtual void setId(int id_) override {
    client->setId(id_);
  }
  virtual int getId() const override {
    return client->getId();
  }
  virtual bool isOpen() const override {
    return client->isOpen();
  }

  virtual idgs::ResultCode send(ClientActorMessagePtr& actorMsg, int timeout_sec = 10) override {
    return client->send(actorMsg, timeout_sec);
  }
  virtual idgs::ResultCode receive(ClientActorMessagePtr& msg, int timeout_sec = 10) override {
    return client->receive(msg, timeout_sec);
  }
  virtual idgs::ResultCode sendRecv(ClientActorMessagePtr& actorMsg, ClientActorMessagePtr& response, int timeout_sec = 10) override {
    return client->sendRecv(actorMsg, response, timeout_sec);
  }

  virtual idgs::ResultCode close() override {
    if (client) {
      DVLOG(2) << "client " << client.get() << " is recycled by pool";
      pool.putBack(client);
      client.reset();
    }
    return RC_OK;
  }

  ~PooledTcpClient() {
    function_footprint();
    close();
  }

private:
  PooledTcpClient(TcpClientPool& pool_, TcpClientPtr _client) :
      pool(pool_), client(_client) {
  }

private:
  TcpClientPool& pool;
  TcpClientPtr client;
};



TcpClientPool::TcpClientPool() : availableServers(0) {
  clientConfig = std::make_shared<idgs::client::pb::ClientConfig>();
  idgs::expr::ExpressionFactory::init();
  isLoaded.store(NOT_INITIALIZED);
}

TcpClientPool::~TcpClientPool() {
  function_footprint();
  close();
}

#define MAX_TRY_TIME_GET_CLIENT_FROM_POOL 10

idgs::ResultCode TcpClientPool::loadConfig(const ClientSetting& setting) {
  idgs::ResultCode code = RC_CLIENT_POOL_IS_AREADY_INIT;
  State expectedState = NOT_INITIALIZED;
  if (isLoaded.compare_exchange_strong(expectedState, INITIALIZED)) {
    clientConfig = std::make_shared<idgs::client::pb::ClientConfig>();
    LOG(INFO) << "Loading client config: " << setting.clientConfig;
    code = (ResultCode)idgs::parseIdgsConfig(clientConfig.get(), setting.clientConfig);

    if (code != RC_SUCCESS) {
      DLOG(ERROR)<< "Failed to parse configuration " << setting.clientConfig << " error: " << getErrorDescription(code);
      return code;
    }

    if (setting.storeConfig != "") {
      code = datastore.loadCfgFile(setting.storeConfig);
      if (code != RC_SUCCESS) {
        DLOG(ERROR)<< "Pars data store configuration " << setting.storeConfig << " error: " << getErrorDescription(code);;
        return code;
      }
    }

    bool isSynchClient = !clientConfig->async_client();
    VLOG_IF(2, isSynchClient) << "Synchronous client is not supported";
    availableServers.reserve(clientConfig->server_addresses_size());

    DVLOG(2) << "The client configured server number is " << clientConfig->server_addresses_size();

    for (int i = 0; i < clientConfig->server_addresses_size(); ++i) {
      DVLOG(2) << "try to connect to server: " << clientConfig->server_addresses(i).host() << ":"
                  << clientConfig->server_addresses(i).port();
      TcpClientPtr client;
      client = std::make_shared<AsioTcpClient>(clientConfig->server_addresses(i));
      client->setId(i);
      code = client->initialize();
      if (code != RC_SUCCESS) {
        DLOG(WARNING)<< " The client " << client.get() << " to server  ["
        << clientConfig->server_addresses(i).host() << ":"
        << clientConfig->server_addresses(i).port() << "]" << " is not available";
      } else {
        DVLOG(2) << "The client " << client.get() << " to server  [" << client->getServerEndpoint().host()
        << ":" << client->getServerEndpoint().port() << "]" << " is ready";
        availableServers.push_back(clientConfig->server_addresses(i));
      }
      client->close();
    }

    if (availableServers.empty()) {
      DLOG(ERROR)<< "All the servers is not available";
      return RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE;
    }

    DVLOG(2) << " The available server size is " << availableServers.size();

    pool.set_capacity(clientConfig->pool_size());

    size_t serverSize = availableServers.size();
    for (int step = 0; step < pool.capacity(); ++step) {
      int serverIndex = step % serverSize;
      idgs::client::pb::Endpoint& server = availableServers.at(serverIndex);

      TcpClientPtr client;
      client = std::make_shared<AsioTcpClient>(server);

      client->setId(step);
      code = client->initialize();

      if (code != RC_SUCCESS) {
        DLOG(WARNING)<< " The client " << client.get() << " to server  ["
        << client->getServerEndpoint().host() << ":"
        << client->getServerEndpoint().port() << "]" << " is not available";
      } else {
        DVLOG(2) << "Put client " << client.get()
        << " at step " << step << " with serverIndex " << serverIndex << " with server " <<
        client->getServerEndpoint().host() << ":" << client->getServerEndpoint().port();
        pool.push(client);
      }
    }
    VLOG(6) << toString();

    // load module descriptor
    for (int i = 0; i < clientConfig->modules_size(); ++ i) {
      auto m = clientConfig->modules(i);
      auto name = m.name();
      auto config = m.config_file();
      if (name == "store") {
        code = datastore.loadCfgFile(config);
        if (code != RC_SUCCESS) {
          LOG(ERROR)<< "Failed to parse store config file,  " << getErrorDescription(code) << ", file: " << config;
          continue;
        }
      }
    }
    code = RC_SUCCESS;
  }
  return code;
}

std::shared_ptr<TcpClient> TcpClientPool::getTcpClient(idgs::ResultCode &code) {
  TcpClientPtr client(NULL);
  std::shared_ptr<TcpClient> clientInterface(NULL);
  size_t try_count = 0;
  size_t count = 0;
  do {
    ++try_count;
    if (pool.try_pop(client)) {
      ++count;
      if (!client->isOpen()) {
        LOG(ERROR)<< "socket has been closed";
        toString();
        continue;
      }
      DVLOG(2) << "get client " << client.get();
      /// @todo let std::make_shared friend of PooledTcpClient
      clientInterface.reset(new PooledTcpClient(*this, client));
      code = RC_SUCCESS;
      break;
    } else {
      LOG(ERROR)<< "pop a client error, current available client = " << pool.size() << ", please check the configured pool size";
      code = RC_ERROR;
    }
  } while(count < pool.size() && try_count < MAX_TRY_TIME_GET_CLIENT_FROM_POOL);
  return clientInterface;
}

void TcpClientPool::close() {
  DVLOG(2) << "call TcpClientPool close";
  function_footprint();
  State expectedState = INITIALIZED;
  if (isLoaded.compare_exchange_strong(expectedState, NOT_INITIALIZED)) {
    DVLOG(2) << " clear pool with size: " << pool.size();
    pool.clear();
    clientConfig.reset();
    availableServers.clear();
  } else {
    DVLOG(2) << "Client Pool Is not initialized or is already closed";
  }
}

bool TcpClientPool::putBack(TcpClientPtr client) {
  if (isLoaded.load() == NOT_INITIALIZED) {
    DLOG(WARNING)<< "Client Pool Is not initialized";
    return false;
  }

  if(!pool.try_push(client)) {
    DLOG(WARNING) << "Try to recycle extra client";
    return false;
  }

  return true;
}

idgs::store::DataStore* TcpClientPool::getDataStore() {
  return &datastore;
}

std::string TcpClientPool::toString() {
  std::stringstream ss;
  ss << "Client Pool(size:  " << size() << ")" << std::endl;
  for (auto e = pool.unsafe_begin(); e != pool.unsafe_end(); ++e) {
    std::string flag = (*e)->isOpen() ? " connected" : " closed";
    ss << "client " << (*e).get()->getId() << flag << " to server: " << (*e)->getServerEndpoint().host() << ":"
        << (*e)->getServerEndpoint().port() << std::endl;
  }
  return ss.str();
}

TcpClientPool& getTcpClientPool() {
  return idgs::util::singleton<TcpClientPool>::getInstance();
}

} // namespace client
} // namespace idgs

